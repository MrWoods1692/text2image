/*
 * Text2Image Library Context Implementation
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the implementation of the LibraryContext class.
 */

#include "text2image_internal.h"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace text2image {

LibraryContext& LibraryContext::getInstance() {
    static LibraryContext instance;
    return instance;
}

LibraryContext::LibraryContext()
    : m_threadPool(std::thread::hardware_concurrency())
    , m_initialized(false) {
}

LibraryContext::~LibraryContext() {
    shutdown();
}

bool LibraryContext::initialize() {
    if (m_initialized.load()) {
        return true;
    }

    try {
        // Create the render engine
        m_renderEngine = std::make_unique<SkiaRenderEngine>();
        
        // Initialize the render engine
        if (!m_renderEngine->initialize()) {
            setLastError("Failed to initialize render engine: " + m_renderEngine->getName());
            m_renderEngine.reset();
            return false;
        }

        // Seed the random number generator for task IDs
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        m_initialized.store(true);
        return true;
    }
    catch (const std::exception& e) {
        setLastError("Exception during initialization: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        setLastError("Unknown exception during initialization");
        return false;
    }
}

void LibraryContext::shutdown() {
    if (!m_initialized.load()) {
        return;
    }

    // Shutdown the thread pool
    m_threadPool.shutdown();

    // Shutdown the render engine
    if (m_renderEngine) {
        m_renderEngine->shutdown();
        m_renderEngine.reset();
    }

    // Clear all tasks
    {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        m_tasks.clear();
    }

    m_initialized.store(false);
}

std::shared_ptr<Task> LibraryContext::createTask(const char* html, const char* css, const Text2Image_RenderOptions* options) {
    if (!m_initialized.load()) {
        setLastError("Library not initialized");
        return nullptr;
    }

    try {
        // Create a new task
        auto task = std::make_shared<Task>(html, css ? css : "", *options);

        // Add the task to the task map
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_tasks[task->getHandle()] = task;
        }

        return task;
    }
    catch (const std::exception& e) {
        setLastError("Exception creating task: " + std::string(e.what()));
        return nullptr;
    }
    catch (...) {
        setLastError("Unknown exception creating task");
        return nullptr;
    }
}

void LibraryContext::freeTask(Text2Image_TaskHandle handle) {
    if (!handle) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_tasksMutex);
    auto it = m_tasks.find(handle);
    if (it != m_tasks.end()) {
        m_tasks.erase(it);
    }
}

std::shared_ptr<Task> LibraryContext::getTask(Text2Image_TaskHandle handle) {
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    auto it = m_tasks.find(handle);
    if (it != m_tasks.end()) {
        return it->second;
    }
    return nullptr;
}

bool LibraryContext::renderSync(std::shared_ptr<Task> task, const char* outputPath) {
    if (!m_initialized.load()) {
        setLastError("Library not initialized");
        return false;
    }

    if (!task) {
        setLastError("Invalid task");
        return false;
    }

    try {
        // Set task status to running
        task->setStatus(TaskStatus::RUNNING);

        // Render the task
        bool success = m_renderEngine->render(task);

        if (success) {
            task->setStatus(TaskStatus::COMPLETED);

            // Save to file if output path is provided
            if (outputPath) {
                const auto& result = task->getResult();
                if (!result.empty()) {
                    std::ofstream file(outputPath, std::ios::binary);
                    if (!file) {
                        setLastError("Failed to open output file: " + std::string(outputPath));
                        return false;
                    }
                    file.write(reinterpret_cast<const char*>(result.data()), result.size());
                    if (!file) {
                        setLastError("Failed to write to output file: " + std::string(outputPath));
                        return false;
                    }
                }
            }

            return true;
        }
        else {
            task->setStatus(TaskStatus::FAILED);
            setLastError("Rendering failed: " + task->getErrorMessage());
            return false;
        }
    }
    catch (const std::exception& e) {
        task->setStatus(TaskStatus::FAILED);
        std::string errorMsg = "Exception during rendering: " + std::string(e.what());
        task->setErrorMessage(errorMsg);
        setLastError(errorMsg);
        return false;
    }
    catch (...) {
        task->setStatus(TaskStatus::FAILED);
        std::string errorMsg = "Unknown exception during rendering";
        task->setErrorMessage(errorMsg);
        setLastError(errorMsg);
        return false;
    }
}

bool LibraryContext::renderAsync(std::shared_ptr<Task> task, const char* outputPath, Text2Image_RenderCallback callback, void* userData) {
    if (!m_initialized.load()) {
        setLastError("Library not initialized");
        return false;
    }

    if (!task) {
        setLastError("Invalid task");
        return false;
    }

    try {
        // Set the callback
        task->setCallback(callback, userData);

        // Create a wrapper function that will handle the file writing and callback
        auto renderTask = [this, task, outputPath]() {
            // Render the task
            bool success = m_renderEngine->render(task);

            if (success) {
                task->setStatus(TaskStatus::COMPLETED);

                // Save to file if output path is provided
                if (outputPath) {
                    const auto& result = task->getResult();
                    if (!result.empty()) {
                        std::ofstream file(outputPath, std::ios::binary);
                        if (!file) {
                            task->setStatus(TaskStatus::FAILED);
                            task->setErrorMessage("Failed to open output file: " + std::string(outputPath));
                            success = false;
                        }
                        else {
                            file.write(reinterpret_cast<const char*>(result.data()), result.size());
                            if (!file) {
                                task->setStatus(TaskStatus::FAILED);
                                task->setErrorMessage("Failed to write to output file: " + std::string(outputPath));
                                success = false;
                            }
                        }
                    }
                }
            }
            else {
                task->setStatus(TaskStatus::FAILED);
            }

            // Execute the callback
            task->executeCallback(success);
        };

        // Enqueue the task in the thread pool
        m_threadPool.enqueue(task);

        return true;
    }
    catch (const std::exception& e) {
        setLastError("Exception during async rendering setup: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        setLastError("Unknown exception during async rendering setup");
        return false;
    }
}

void LibraryContext::setLastError(const std::string& error) {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    m_lastError = error;
}

const char* LibraryContext::getLastError() const {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    return m_lastError.empty() ? nullptr : m_lastError.c_str();
}

} // namespace text2image