/*
 * Text2Image Library
 * Copyright (c) 2025 Text2Image contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TEXT2IMAGE_H
#define TEXT2IMAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resolution types supported by the library
 */
typedef enum {
    TEXT2IMAGE_RESOLUTION_AUTO = 0,  ///< Automatically determine resolution based on content
    TEXT2IMAGE_RESOLUTION_720P = 1,  ///< 1280x720
    TEXT2IMAGE_RESOLUTION_1080P = 2, ///< 1920x1080
    TEXT2IMAGE_RESOLUTION_2K = 3,    ///< 2560x1440
    TEXT2IMAGE_RESOLUTION_4K = 4,    ///< 3840x2160
    TEXT2IMAGE_RESOLUTION_8K = 5     ///< 7680x4320
} Text2Image_Resolution;

/**
 * @brief Output image formats supported by the library
 */
typedef enum {
    TEXT2IMAGE_FORMAT_PNG = 0,
    TEXT2IMAGE_FORMAT_JPG = 1,
    TEXT2IMAGE_FORMAT_JPEG = 1,  ///< Same as JPG
    TEXT2IMAGE_FORMAT_WEBP = 2,
    TEXT2IMAGE_FORMAT_BMP = 3,
    TEXT2IMAGE_FORMAT_TIF = 4,
    TEXT2IMAGE_FORMAT_TIFF = 4,  ///< Same as TIF
    TEXT2IMAGE_FORMAT_HEIC = 5,
    TEXT2IMAGE_FORMAT_HEIF = 5,  ///< Same as HEIC
    TEXT2IMAGE_FORMAT_AVIF = 6
} Text2Image_Format;

/**
 * @brief Background types for the rendered image
 */
typedef enum {
    TEXT2IMAGE_BACKGROUND_SOLID = 0,  ///< Solid color background
    TEXT2IMAGE_BACKGROUND_IMAGE = 1   ///< Image background
} Text2Image_BackgroundType;

/**
 * @brief Task handle type
 */
typedef void* Text2Image_TaskHandle;

/**
 * @brief Render callback function type
 */
typedef void (*Text2Image_RenderCallback)(Text2Image_TaskHandle task, bool success, void* userData);

/**
 * @brief Render options structure
 */
typedef struct {
    // Resolution settings
    Text2Image_Resolution resolution;  ///< Image resolution
    
    // Output format
    Text2Image_Format format;          ///< Output image format
    
    // Quality settings (0-100)
    int quality;                       ///< Image quality
    
    // Custom dimensions (used when resolution is AUTO)
    int customWidth;                   ///< Custom width in pixels
    int customHeight;                  ///< Custom height in pixels
    
    // Background settings
    Text2Image_BackgroundType backgroundType;  ///< Background type
    uint32_t backgroundColor;                  ///< Background color (ARGB format)
    const char* backgroundImage;               ///< Background image path
    float backgroundBlur;                      ///< Background blur amount (0-100)
    
    // Output image border radius
    int borderRadius;                   ///< Border radius in pixels
    
    // Additional rendering options
    bool enableJavaScript;              ///< Enable JavaScript execution
    int timeout;                        ///< Render timeout in milliseconds
} Text2Image_RenderOptions;

/**
 * @brief Initialize the Text2Image library
 * 
 * @return true if initialization was successful, false otherwise
 */
bool Text2Image_Init();

/**
 * @brief Shutdown the Text2Image library and release resources
 */
void Text2Image_Shutdown();

/**
 * @brief Create a new render task
 * 
 * @param html HTML content to render
 * @param css CSS styles to apply
 * @param options Render options
 * @return Task handle or NULL on error
 */
Text2Image_TaskHandle Text2Image_CreateTask(const char* html, const char* css, const Text2Image_RenderOptions* options);

/**
 * @brief Render a task synchronously
 * 
 * @param task Task handle
 * @param outputPath Output file path
 * @return true if rendering was successful, false otherwise
 */
bool Text2Image_Render(Text2Image_TaskHandle task, const char* outputPath);

/**
 * @brief Render a task asynchronously
 * 
 * @param task Task handle
 * @param outputPath Output file path
 * @param callback Callback function to call when rendering is complete
 * @param userData User data to pass to the callback
 * @return true if rendering was started successfully, false otherwise
 */
bool Text2Image_RenderAsync(Text2Image_TaskHandle task, const char* outputPath, Text2Image_RenderCallback callback, void* userData);

/**
 * @brief Get the rendered image data in memory
 * 
 * @param task Task handle
 * @param buffer Pointer to receive the image data buffer
 * @param size Pointer to receive the image data size
 * @return true if successful, false otherwise
 * @note The caller is responsible for freeing the buffer with Text2Image_FreeBuffer
 */
bool Text2Image_GetResult(Text2Image_TaskHandle task, uint8_t** buffer, size_t* size);

/**
 * @brief Free a buffer returned by Text2Image_GetResult
 * 
 * @param buffer Buffer to free
 */
void Text2Image_FreeBuffer(uint8_t* buffer);

/**
 * @brief Free a render task
 * 
 * @param task Task handle
 */
void Text2Image_FreeTask(Text2Image_TaskHandle task);

/**
 * @brief Get the last error message
 * 
 * @return Error message string or NULL if no error
 */
const char* Text2Image_GetLastError();

/**
 * @brief Set the maximum number of threads to use for rendering
 * 
 * @param numThreads Maximum number of threads (0 = auto-detect)
 */
void Text2Image_SetMaxThreads(int numThreads);

/**
 * @brief Get the default render options
 * 
 * @return Default render options
 */
Text2Image_RenderOptions Text2Image_GetDefaultOptions();

#ifdef __cplusplus
}
#endif

#endif // TEXT2IMAGE_H