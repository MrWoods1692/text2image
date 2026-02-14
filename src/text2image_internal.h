/*
 * Text2Image Library Internal Header
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains internal definitions and structures used by the library.
 */

#ifndef TEXT2IMAGE_INTERNAL_H
#define TEXT2IMAGE_INTERNAL_H

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>

#include "text2image.h"

namespace text2image {

// Forward declarations
class RenderEngine;
class Task;
class ThreadPool;

// Task priority levels
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2
};

// Task status
enum class TaskStatus {
    PENDING = 0,
    RUNNING = 1,
    COMPLETED = 2,
    FAILED = 3,
    CANCELLED = 4
};

// Task structure
class Task {
public:
    Task(const std::string& html, const std::string& css, const Text2Image_RenderOptions& options);
    ~Task();

    // Getters
    Text2Image_TaskHandle getHandle() const { return m_handle; }
    const std::string& getHtml() const { return m_html; }
    const std::string& getCss() const { return m_css; }
    const Text2Image_RenderOptions& getOptions() const { return m_options; }
    TaskStatus getStatus() const { return m_status.load(); }
    const std::string& getErrorMessage() const { return m_errorMessage; }
    const std::vector<uint8_t>& getResult() const { return m_result; }
    TaskPriority getPriority() const { return m_priority; }

    // Setters
    void setStatus(TaskStatus status) { m_status.store(status); }
    void setErrorMessage(const std::string& message) { m_errorMessage = message; }
    void setResult(const std::vector<uint8_t>& result) { m_result = result; }
    void setPriority(TaskPriority priority) { m_priority = priority; }

    // Render callback
    void setCallback(Text2Image_RenderCallback callback, void* userData) {
        m_callback = callback;
        m_userData = userData;
    }

    void executeCallback(bool success) {
        if (m_callback) {
            m_callback(m_handle, success, m_userData);
        }
    }

private:
    Text2Image_TaskHandle m_handle;
    std::string m_html;
    std::string m_css;
    Text2Image_RenderOptions m_options;
    std::atomic<TaskStatus> m_status;
    std::string m_errorMessage;
    std::vector<uint8_t> m_result;
    TaskPriority m_priority;
    Text2Image_RenderCallback m_callback;
    void* m_userData;
};

// Thread pool for concurrent task execution
class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Add task to the queue
    void enqueue(std::shared_ptr<Task> task);

    // Stop all threads
    void shutdown();

    // Set maximum number of threads
    void setMaxThreads(size_t numThreads);

private:
    // Worker thread function
    void worker();

    std::vector<std::thread> m_workers;
    std::queue<std::shared_ptr<Task>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop;
    std::atomic<size_t> m_activeThreads;
    size_t m_maxThreads;
};

// Render engine interface
class RenderEngine {
public:
    virtual ~RenderEngine() = default;

    // Initialize the render engine
    virtual bool initialize() = 0;

    // Shutdown the render engine
    virtual void shutdown() = 0;

    // Render a task
    virtual bool render(std::shared_ptr<Task> task) = 0;

    // Get the engine name
    virtual std::string getName() const = 0;
};

// Skia-based render engine implementation
class SkiaRenderEngine : public RenderEngine {
public:
    SkiaRenderEngine();
    ~SkiaRenderEngine() override;

    bool initialize() override;
    void shutdown() override;
    bool render(std::shared_ptr<Task> task) override;
    std::string getName() const override { return "Skia"; }

private:
    // Internal implementation details
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// Library context
class LibraryContext {
public:
    static LibraryContext& getInstance();

    bool initialize();
    void shutdown();

    // Task management
    std::shared_ptr<Task> createTask(const char* html, const char* css, const Text2Image_RenderOptions* options);
    void freeTask(Text2Image_TaskHandle handle);
    std::shared_ptr<Task> getTask(Text2Image_TaskHandle handle);

    // Rendering
    bool renderSync(std::shared_ptr<Task> task, const char* outputPath);
    bool renderAsync(std::shared_ptr<Task> task, const char* outputPath, Text2Image_RenderCallback callback, void* userData);

    // Error handling
    void setLastError(const std::string& error);
    const char* getLastError() const;

    // Thread pool
    ThreadPool& getThreadPool() { return m_threadPool; }

private:
    LibraryContext();
    ~LibraryContext();

    std::unique_ptr<RenderEngine> m_renderEngine;
    ThreadPool m_threadPool;
    std::unordered_map<Text2Image_TaskHandle, std::shared_ptr<Task>> m_tasks;
    std::mutex m_tasksMutex;
    std::string m_lastError;
    std::mutex m_errorMutex;
    std::atomic<bool> m_initialized;
};

} // namespace text2image

#endif // TEXT2IMAGE_INTERNAL_H