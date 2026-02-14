/*
 * Text2Image C++ Example
 * Copyright (c) 2025 Text2Image contributors
 *
 * This is a simple example demonstrating how to use the Text2Image library in C++.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../include/text2image.h"

// Helper function to read a file into a string
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

// Callback function for async rendering
void renderCallback(Text2Image_TaskHandle task, bool success, void* userData) {
    if (success) {
        std::cout << "Async rendering completed successfully!" << std::endl;
        
        // Get the result in memory
        uint8_t* buffer = nullptr;
        size_t size = 0;
        if (Text2Image_GetResult(task, &buffer, &size)) {
            std::cout << "Got result buffer with size: " << size << " bytes" << std::endl;
            
            // Save buffer to file
            std::ofstream outFile("output_async_buffer.png", std::ios::binary);
            if (outFile) {
                outFile.write(reinterpret_cast<const char*>(buffer), size);
                std::cout << "Saved buffer to output_async_buffer.png" << std::endl;
            }
            
            // Free the buffer
            Text2Image_FreeBuffer(buffer);
        } else {
            std::cerr << "Failed to get result: " << Text2Image_GetLastError() << std::endl;
        }
        
        // Free the task
        Text2Image_FreeTask(task);
        
        // Set the flag to indicate completion
        bool* completed = static_cast<bool*>(userData);
        *completed = true;
    } else {
        std::cerr << "Async rendering failed: " << Text2Image_GetLastError() << std::endl;
        
        // Free the task
        Text2Image_FreeTask(task);
        
        // Set the flag to indicate completion
        bool* completed = static_cast<bool*>(userData);
        *completed = true;
    }
}

int main() {
    std::cout << "Text2Image C++ Example" << std::endl;
    std::cout << "=====================" << std::endl;
    
    // Initialize the library
    std::cout << "Initializing Text2Image library..." << std::endl;
    if (!Text2Image_Init()) {
        std::cerr << "Failed to initialize Text2Image library: " << Text2Image_GetLastError() << std::endl;
        return 1;
    }
    
    // Set maximum number of threads (0 = auto-detect)
    Text2Image_SetMaxThreads(0);
    
    // HTML content to render
    const std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Text2Image Example</title>
</head>
<body>
    <div class="container">
        <h1>Text2Image C++ Example</h1>
        <p>This is a demonstration of the Text2Image library capabilities.</p>
        
        <h2>Features</h2>
        <ul>
            <li>High-performance rendering</li>
            <li>Multiple output formats</li>
            <li>Custom CSS styling</li>
            <li>Concurrent processing</li>
            <li>Cross-platform compatibility</li>
        </ul>
        
        <h2>Sample Table</h2>
        <table>
            <tr>
                <th>Format</th>
                <th>Quality</th>
                <th>File Size</th>
            </tr>
            <tr>
                <td>PNG</td>
                <td>Lossless</td>
                <td>Large</td>
            </tr>
            <tr>
                <td>JPEG</td>
                <td>Configurable</td>
                <td>Medium</td>
            </tr>
            <tr>
                <td>WebP</td>
                <td>Configurable</td>
                <td>Small</td>
            </tr>
        </table>
        
        <h2>Code Example</h2>
        <pre><code class="cpp">
#include &lt;text2image.h&gt;

int main() {
    // Initialize the library
    Text2Image_Init();
    
    // Create a render task
    Text2Image_TaskHandle task = Text2Image_CreateTask(
        "&lt;p&gt;Hello World&lt;/p&gt;",
        "p { color: blue; font-size: 24px; }",
        nullptr
    );
    
    // Render to file
    Text2Image_Render(task, "output.png");
    
    // Clean up
    Text2Image_FreeTask(task);
    Text2Image_Shutdown();
    
    return 0;
}
        </code></pre>
    </div>
</body>
</html>
)";
    
    // CSS styles
    const std::string css = R"(
body {
    font-family: 'Arial', sans-serif;
    line-height: 1.6;
    color: #333;
    background-color: #f4f4f4;
    margin: 0;
    padding: 0;
}

.container {
    max-width: 800px;
    margin: 0 auto;
    padding: 30px;
    background-color: white;
    border-radius: 10px;
    box-shadow: 0 2px 15px rgba(0, 0, 0, 0.1);
}

h1 {
    color: #2c3e50;
    border-bottom: 3px solid #3498db;
    padding-bottom: 10px;
}

h2 {
    color: #2980b9;
    margin-top: 30px;
}

ul {
    background-color: #ecf0f1;
    padding: 20px 20px 20px 40px;
    border-radius: 5px;
}

li {
    margin-bottom: 8px;
}

table {
    width: 100%;
    border-collapse: collapse;
    margin: 20px 0;
}

th, td {
    padding: 12px 15px;
    text-align: left;
    border-bottom: 1px solid #ddd;
}

th {
    background-color: #3498db;
    color: white;
    font-weight: bold;
}

tr:hover {
    background-color: #f5f5f5;
}

pre {
    background-color: #2c3e50;
    color: #ecf0f1;
    padding: 20px;
    border-radius: 5px;
    overflow-x: auto;
    font-family: 'Courier New', monospace;
}

code {
    font-family: 'Courier New', monospace;
}
)";
    
    // Create render options
    Text2Image_RenderOptions options = Text2Image_GetDefaultOptions();
    options.resolution = TEXT2IMAGE_RESOLUTION_1080P;
    options.format = TEXT2IMAGE_FORMAT_PNG;
    options.quality = 95;
    options.backgroundColor = 0xFFFFFFFF;
    options.borderRadius = 10;
    
    // Create a render task
    std::cout << "Creating render task..." << std::endl;
    Text2Image_TaskHandle task = Text2Image_CreateTask(html.c_str(), css.c_str(), &options);
    if (!task) {
        std::cerr << "Failed to create task: " << Text2Image_GetLastError() << std::endl;
        Text2Image_Shutdown();
        return 1;
    }
    
    // Test synchronous rendering
    std::cout << "Rendering synchronously to 'output_sync.png'..." << std::endl;
    if (Text2Image_Render(task, "output_sync.png")) {
        std::cout << "Synchronous rendering completed successfully!" << std::endl;
    } else {
        std::cerr << "Synchronous rendering failed: " << Text2Image_GetLastError() << std::endl;
        Text2Image_FreeTask(task);
        Text2Image_Shutdown();
        return 1;
    }
    
    // Test asynchronous rendering
    std::cout << "Rendering asynchronously to 'output_async.png'..." << std::endl;
    
    // Create a new task for async rendering
    Text2Image_TaskHandle asyncTask = Text2Image_CreateTask(html.c_str(), css.c_str(), &options);
    if (!asyncTask) {
        std::cerr << "Failed to create async task: " << Text2Image_GetLastError() << std::endl;
        Text2Image_FreeTask(task);
        Text2Image_Shutdown();
        return 1;
    }
    
    // Flag to indicate completion
    bool completed = false;
    
    // Start async rendering
    if (Text2Image_RenderAsync(asyncTask, "output_async.png", renderCallback, &completed)) {
        std::cout << "Async rendering started. Waiting for completion..." << std::endl;
        
        // Wait for async rendering to complete
        while (!completed) {
            // Sleep for a short time to avoid CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else {
        std::cerr << "Failed to start async rendering: " << Text2Image_GetLastError() << std::endl;
        Text2Image_FreeTask(task);
        Text2Image_FreeTask(asyncTask);
        Text2Image_Shutdown();
        return 1;
    }
    
    // Free the sync task
    Text2Image_FreeTask(task);
    
    // Shutdown the library
    std::cout << "Shutting down Text2Image library..." << std::endl;
    Text2Image_Shutdown();
    
    std::cout << "Example completed successfully!" << std::endl;
    return 0;
}