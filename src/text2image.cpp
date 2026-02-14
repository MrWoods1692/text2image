/*
 * Text2Image Library Implementation
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the implementation of the public API functions.
 */

#include "text2image.h"
#include "text2image_internal.h"

#include <cstring>
#include <algorithm>
#include <thread>

namespace text2image {

// Global instance of the library context
static LibraryContext& g_context = LibraryContext::getInstance();

} // namespace text2image

// Public API implementation

bool Text2Image_Init() {
    return text2image::g_context.initialize();
}

void Text2Image_Shutdown() {
    text2image::g_context.shutdown();
}

Text2Image_TaskHandle Text2Image_CreateTask(const char* html, const char* css, const Text2Image_RenderOptions* options) {
    if (!html) {
        text2image::g_context.setLastError("HTML content cannot be null");
        return nullptr;
    }

    // Use default options if none provided
    Text2Image_RenderOptions defaultOptions = Text2Image_GetDefaultOptions();
    if (!options) {
        options = &defaultOptions;
    }

    auto task = text2image::g_context.createTask(html, css ? css : "", options);
    if (!task) {
        return nullptr;
    }

    return task->getHandle();
}

bool Text2Image_Render(Text2Image_TaskHandle task, const char* outputPath) {
    if (!task) {
        text2image::g_context.setLastError("Invalid task handle");
        return false;
    }

    auto taskPtr = text2image::g_context.getTask(task);
    if (!taskPtr) {
        text2image::g_context.setLastError("Task not found");
        return false;
    }

    return text2image::g_context.renderSync(taskPtr, outputPath);
}

bool Text2Image_RenderAsync(Text2Image_TaskHandle task, const char* outputPath, Text2Image_RenderCallback callback, void* userData) {
    if (!task) {
        text2image::g_context.setLastError("Invalid task handle");
        return false;
    }

    auto taskPtr = text2image::g_context.getTask(task);
    if (!taskPtr) {
        text2image::g_context.setLastError("Task not found");
        return false;
    }

    return text2image::g_context.renderAsync(taskPtr, outputPath, callback, userData);
}

bool Text2Image_GetResult(Text2Image_TaskHandle task, uint8_t** buffer, size_t* size) {
    if (!task || !buffer || !size) {
        text2image::g_context.setLastError("Invalid parameters");
        return false;
    }

    auto taskPtr = text2image::g_context.getTask(task);
    if (!taskPtr) {
        text2image::g_context.setLastError("Task not found");
        return false;
    }

    if (taskPtr->getStatus() != text2image::TaskStatus::COMPLETED) {
        text2image::g_context.setLastError("Task not completed");
        return false;
    }

    const auto& result = taskPtr->getResult();
    if (result.empty()) {
        text2image::g_context.setLastError("No result available");
        return false;
    }

    // Allocate memory for the buffer
    *buffer = new uint8_t[result.size()];
    if (!*buffer) {
        text2image::g_context.setLastError("Failed to allocate memory");
        return false;
    }

    // Copy the result data
    std::memcpy(*buffer, result.data(), result.size());
    *size = result.size();

    return true;
}

void Text2Image_FreeBuffer(uint8_t* buffer) {
    if (buffer) {
        delete[] buffer;
    }
}

void Text2Image_FreeTask(Text2Image_TaskHandle task) {
    if (task) {
        text2image::g_context.freeTask(task);
    }
}

const char* Text2Image_GetLastError() {
    return text2image::g_context.getLastError();
}

void Text2Image_SetMaxThreads(int numThreads) {
    if (numThreads <= 0) {
        // Auto-detect based on CPU cores
        numThreads = static_cast<int>(std::thread::hardware_concurrency());
    }
    
    text2image::g_context.getThreadPool().setMaxThreads(static_cast<size_t>(numThreads));
}

Text2Image_RenderOptions Text2Image_GetDefaultOptions() {
    Text2Image_RenderOptions options;
    
    // Default resolution: AUTO
    options.resolution = TEXT2IMAGE_RESOLUTION_AUTO;
    
    // Default format: PNG
    options.format = TEXT2IMAGE_FORMAT_PNG;
    
    // Default quality: 90%
    options.quality = 90;
    
    // Default custom dimensions: 0 (auto-detect)
    options.customWidth = 0;
    options.customHeight = 0;
    
    // Default background: white solid color
    options.backgroundType = TEXT2IMAGE_BACKGROUND_SOLID;
    options.backgroundColor = 0xFFFFFFFF;  // White (ARGB)
    options.backgroundImage = nullptr;
    options.backgroundBlur = 0.0f;
    
    // Default border radius: 0 (no rounded corners)
    options.borderRadius = 0;
    
    // Default JavaScript: disabled
    options.enableJavaScript = false;
    
    // Default timeout: 30 seconds
    options.timeout = 30000;
    
    return options;
}