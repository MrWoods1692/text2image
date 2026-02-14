/*
 * Text2Image Thread Pool Implementation
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the implementation of the ThreadPool class.
 */

#include "text2image_internal.h"

#include <iostream>
#include <thread>
#include <chrono>

namespace text2image {

ThreadPool::ThreadPool(size_t numThreads)
    : m_stop(false)
    , m_activeThreads(0)
    , m_maxThreads(numThreads) {
    // Create worker threads
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back(&ThreadPool::worker, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::enqueue(std::shared_ptr<Task> task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Don't allow enqueueing after shutdown
        if (m_stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        // Add the task to the queue
        m_tasks.push(task);
    }
    
    // Notify one worker thread
    m_condition.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    
    // Notify all worker threads
    m_condition.notify_all();
    
    // Wait for all worker threads to finish
    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // Clear the worker threads
    m_workers.clear();
}

void ThreadPool::setMaxThreads(size_t numThreads) {
    if (numThreads == m_maxThreads) {
        return;
    }
    
    size_t currentSize = m_workers.size();
    
    if (numThreads > currentSize) {
        // Add more threads
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_maxThreads = numThreads;
        }
        
        // Create new worker threads
        for (size_t i = currentSize; i < numThreads; ++i) {
            m_workers.emplace_back(&ThreadPool::worker, this);
        }
    }
    else if (numThreads < currentSize) {
        // Reduce the number of threads
        // Note: This is a bit tricky as we can't directly kill threads
        // We'll set the maxThreads and let threads exit naturally
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_maxThreads = numThreads;
        }
        
        // Notify all threads so they can check the new maxThreads value
        m_condition.notify_all();
    }
}

void ThreadPool::worker() {
    while (true) {
        std::shared_ptr<Task> task;
        
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            // Wait until there's a task or we're shutting down
            m_condition.wait(lock, [this] {
                return m_stop || !m_tasks.empty() || m_activeThreads >= m_maxThreads;
            });
            
            // Check if we're shutting down or if we've reached max threads
            if (m_stop || (m_tasks.empty() && m_activeThreads >= m_maxThreads)) {
                // If we're reducing thread count, exit this thread
                if (m_workers.size() > m_maxThreads) {
                    return;
                }
                
                // Otherwise, just wait again
                if (m_tasks.empty()) {
                    continue;
                }
            }
            
            // Get the next task from the queue
            task = m_tasks.front();
            m_tasks.pop();
            
            // Increment active threads count
            ++m_activeThreads;
        }
        
        try {
            // Execute the task
            task->setStatus(TaskStatus::RUNNING);
            
            // Here we would call the render engine to render the task
            // For now, we'll just simulate some work
            // In the real implementation, this would be:
            // LibraryContext::getInstance().getRenderEngine()->render(task);
            
            // Simulate work with a sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Mark task as completed
            task->setStatus(TaskStatus::COMPLETED);
            
            // Execute the callback if provided
            task->executeCallback(true);
        }
        catch (const std::exception& e) {
            // Handle exception
            task->setStatus(TaskStatus::FAILED);
            task->setErrorMessage("Exception in worker thread: " + std::string(e.what()));
            task->executeCallback(false);
        }
        catch (...) {
            // Handle unknown exception
            task->setStatus(TaskStatus::FAILED);
            task->setErrorMessage("Unknown exception in worker thread");
            task->executeCallback(false);
        }
        
        // Decrement active threads count
        --m_activeThreads;
    }
}

} // namespace text2image