/*
 * Text2Image Skia Render Engine Implementation
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the implementation of the SkiaRenderEngine class.
 */

#include "text2image_internal.h"

#include <SkCanvas.h>
#include <SkDocument.h>
#include <SkPaint.h>
#include <SkPath.h>
#include <SkStream.h>
#include <SkSurface.h>
#include <SkTypeface.h>
#include <SkFont.h>
#include <SkTextBlob.h>
#include <SkImage.h>
#include <SkCodec.h>
#include <SkData.h>

#include <libxml/HTMLparser.h>
#include <libxml/css.h>

#include <fstream>
#include <sstream>
#include <regex>

namespace text2image {

// SkiaRenderEngine implementation details
class SkiaRenderEngine::Impl {
public:
    Impl();
    ~Impl();

    bool initialize();
    void shutdown();
    bool render(std::shared_ptr<Task> task);

private:
    // HTML parsing and rendering
    bool parseHtml(const std::string& html, const std::string& css);
    bool renderHtmlToCanvas(SkCanvas* canvas, int width, int height, const Text2Image_RenderOptions& options);
    
    // Background handling
    bool drawBackground(SkCanvas* canvas, int width, int height, const Text2Image_RenderOptions& options);
    
    // Image format conversion
    bool encodeImage(SkCanvas* canvas, SkEncodedImageFormat format, int quality, std::vector<uint8_t>& output);
    
    // CSS parsing
    bool parseCss(const std::string& css);
    
    // Font loading
    sk_sp<SkTypeface> loadFont(const std::string& fontFamily, int weight, bool italic);
    
    // HTML element rendering
    void renderElement(SkCanvas* canvas, xmlNode* node, int x, int y, int width);
    
    // Code highlighting
    std::string highlightCode(const std::string& code, const std::string& language);
    
    // Image loading
    sk_sp<SkImage> loadImage(const std::string& path);
    
    // Members
    xmlDocPtr m_htmlDoc;
    std::unordered_map<std::string, std::string> m_cssRules;
    std::vector<sk_sp<SkTypeface>> m_loadedFonts;
};

SkiaRenderEngine::SkiaRenderEngine()
    : m_impl(new Impl()) {
}

SkiaRenderEngine::~SkiaRenderEngine() {
    delete m_impl;
}

bool SkiaRenderEngine::initialize() {
    return m_impl->initialize();
}

void SkiaRenderEngine::shutdown() {
    m_impl->shutdown();
}

bool SkiaRenderEngine::render(std::shared_ptr<Task> task) {
    return m_impl->render(task);
}

// Impl class implementation

SkiaRenderEngine::Impl::Impl()
    : m_htmlDoc(nullptr) {
}

SkiaRenderEngine::Impl::~Impl() {
    shutdown();
}

bool SkiaRenderEngine::Impl::initialize() {
    try {
        // Initialize libxml2
        xmlInitParser();
        
        // Load default fonts
        // In a real implementation, we would load system fonts here
        sk_sp<SkTypeface> defaultFont = SkTypeface::MakeDefault();
        if (defaultFont) {
            m_loadedFonts.push_back(defaultFont);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        // Handle exception
        return false;
    }
    catch (...) {
        // Handle unknown exception
        return false;
    }
}

void SkiaRenderEngine::Impl::shutdown() {
    // Clean up HTML document if it exists
    if (m_htmlDoc) {
        xmlFreeDoc(m_htmlDoc);
        m_htmlDoc = nullptr;
    }
    
    // Clear CSS rules
    m_cssRules.clear();
    
    // Clear loaded fonts
    m_loadedFonts.clear();
    
    // Shutdown libxml2
    xmlCleanupParser();
}

bool SkiaRenderEngine::Impl::render(std::shared_ptr<Task> task) {
    try {
        const std::string& html = task->getHtml();
        const std::string& css = task->getCss();
        const Text2Image_RenderOptions& options = task->getOptions();
        
        // Parse HTML and CSS
        if (!parseHtml(html, css)) {
            task->setErrorMessage("Failed to parse HTML/CSS");
            return false;
        }
        
        // Determine canvas size
        int width, height;
        if (options.resolution == TEXT2IMAGE_RESOLUTION_AUTO) {
            // Auto-detect size based on content
            // For now, use custom dimensions or default to 800x600
            width = options.customWidth > 0 ? options.customWidth : 800;
            height = options.customHeight > 0 ? options.customHeight : 600;
        }
        else {
            // Use predefined resolution
            switch (options.resolution) {
                case TEXT2IMAGE_RESOLUTION_720P:
                    width = 1280;
                    height = 720;
                    break;
                case TEXT2IMAGE_RESOLUTION_1080P:
                    width = 1920;
                    height = 1080;
                    break;
                case TEXT2IMAGE_RESOLUTION_2K:
                    width = 2560;
                    height = 1440;
                    break;
                case TEXT2IMAGE_RESOLUTION_4K:
                    width = 3840;
                    height = 2160;
                    break;
                case TEXT2IMAGE_RESOLUTION_8K:
                    width = 7680;
                    height = 4320;
                    break;
                default:
                    width = 800;
                    height = 600;
                    break;
            }
        }
        
        // Create Skia surface
        SkImageInfo info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
        if (!surface) {
            task->setErrorMessage("Failed to create Skia surface");
            return false;
        }
        
        SkCanvas* canvas = surface->getCanvas();
        
        // Draw background
        if (!drawBackground(canvas, width, height, options)) {
            task->setErrorMessage("Failed to draw background");
            return false;
        }
        
        // Render HTML to canvas
        if (!renderHtmlToCanvas(canvas, width, height, options)) {
            task->setErrorMessage("Failed to render HTML");
            return false;
        }
        
        // Apply border radius if needed
        if (options.borderRadius > 0) {
            // Create a new surface with rounded corners
            SkPath path;
            path.addRoundRect(SkRect::MakeWH(width, height), options.borderRadius, options.borderRadius);
            
            sk_sp<SkSurface> roundedSurface = SkSurface::MakeRaster(info);
            SkCanvas* roundedCanvas = roundedSurface->getCanvas();
            
            // Clear the canvas
            roundedCanvas->clear(SK_ColorTRANSPARENT);
            
            // Clip to rounded rectangle
            roundedCanvas->clipPath(path, true);
            
            // Draw the original surface
            sk_sp<SkImage> image = surface->makeImageSnapshot();
            roundedCanvas->drawImage(image, 0, 0);
            
            // Replace the original surface with the rounded one
            surface = roundedSurface;
        }
        
        // Encode the image
        SkEncodedImageFormat format;
        switch (options.format) {
            case TEXT2IMAGE_FORMAT_PNG:
                format = SkEncodedImageFormat::kPNG;
                break;
            case TEXT2IMAGE_FORMAT_JPG:
            case TEXT2IMAGE_FORMAT_JPEG:
                format = SkEncodedImageFormat::kJPEG;
                break;
            case TEXT2IMAGE_FORMAT_WEBP:
                format = SkEncodedImageFormat::kWEBP;
                break;
            case TEXT2IMAGE_FORMAT_BMP:
                format = SkEncodedImageFormat::kBMP;
                break;
            case TEXT2IMAGE_FORMAT_TIF:
            case TEXT2IMAGE_FORMAT_TIFF:
                format = SkEncodedImageFormat::kTIFF;
                break;
            default:
                format = SkEncodedImageFormat::kPNG;
                break;
        }
        
        std::vector<uint8_t> output;
        if (!encodeImage(surface->getCanvas(), format, options.quality, output)) {
            task->setErrorMessage("Failed to encode image");
            return false;
        }
        
        // Store the result in the task
        task->setResult(output);
        
        return true;
    }
    catch (const std::exception& e) {
        task->setErrorMessage("Exception during rendering: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        task->setErrorMessage("Unknown exception during rendering");
        return false;
    }
}

bool SkiaRenderEngine::Impl::parseHtml(const std::string& html, const std::string& css) {
    // Clean up previous document if it exists
    if (m_htmlDoc) {
        xmlFreeDoc(m_htmlDoc);
        m_htmlDoc = nullptr;
    }
    
    // Parse CSS first
    if (!parseCss(css)) {
        return false;
    }
    
    // Parse HTML
    htmlDocPtr doc = htmlReadMemory(html.c_str(), static_cast<int>(html.length()), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        return false;
    }
    
    m_htmlDoc = doc;
    return true;
}

bool SkiaRenderEngine::Impl::renderHtmlToCanvas(SkCanvas* canvas, int width, int height, const Text2Image_RenderOptions& options) {
    if (!m_htmlDoc) {
        return false;
    }
    
    // Get the root element
    xmlNode* root = xmlDocGetRootElement(m_htmlDoc);
    if (!root) {
        return false;
    }
    
    // Render the root element
    renderElement(canvas, root, 0, 0, width);
    
    return true;
}

bool SkiaRenderEngine::Impl::drawBackground(SkCanvas* canvas, int width, int height, const Text2Image_RenderOptions& options) {
    try {
        if (options.backgroundType == TEXT2IMAGE_BACKGROUND_SOLID) {
            // Draw solid background color
            SkPaint paint;
            paint.setColor(options.backgroundColor);
            canvas->drawRect(SkRect::MakeWH(width, height), paint);
        }
        else if (options.backgroundType == TEXT2IMAGE_BACKGROUND_IMAGE && options.backgroundImage) {
            // Load and draw background image
            sk_sp<SkImage> image = loadImage(options.backgroundImage);
            if (!image) {
                return false;
            }
            
            // Draw the image scaled to fit the canvas
            SkPaint paint;
            
            // Apply blur if needed
            if (options.backgroundBlur > 0) {
                // In a real implementation, we would apply a blur filter here
                // For now, we'll just draw the image as is
            }
            
            // Calculate the scaling factor to fit the image to the canvas
            SkScalar scaleX = static_cast<SkScalar>(width) / image->width();
            SkScalar scaleY = static_cast<SkScalar>(height) / image->height();
            SkScalar scale = std::max(scaleX, scaleY);
            
            // Calculate the position to center the image
            SkScalar x = (width - image->width() * scale) / 2;
            SkScalar y = (height - image->height() * scale) / 2;
            
            // Draw the image
            canvas->save();
            canvas->translate(x, y);
            canvas->scale(scale, scale);
            canvas->drawImage(image, 0, 0, &paint);
            canvas->restore();
        }
        
        return true;
    }
    catch (const std::exception& e) {
        // Handle exception
        return false;
    }
    catch (...) {
        // Handle unknown exception
        return false;
    }
}

bool SkiaRenderEngine::Impl::encodeImage(SkCanvas* canvas, SkEncodedImageFormat format, int quality, std::vector<uint8_t>& output) {
    try {
        // Create an image snapshot of the canvas
        sk_sp<SkImage> image = canvas->makeImageSnapshot();
        if (!image) {
            return false;
        }
        
        // Encode the image
        SkDynamicMemoryWStream stream;
        if (!image->encodeToStream(&stream, format, quality)) {
            return false;
        }
        
        // Get the encoded data
        sk_sp<SkData> data = stream.detachAsData();
        if (!data) {
            return false;
        }
        
        // Copy the data to the output vector
        output.resize(data->size());
        std::memcpy(output.data(), data->data(), data->size());
        
        return true;
    }
    catch (const std::exception& e) {
        // Handle exception
        return false;
    }
    catch (...) {
        // Handle unknown exception
        return false;
    }
}

bool SkiaRenderEngine::Impl::parseCss(const std::string& css) {
    // Clear previous CSS rules
    m_cssRules.clear();
    
    // Simple CSS parser (this is a very basic implementation)
    // In a real implementation, we would use a proper CSS parser
    std::regex ruleRegex("([^{]+)\\s*\\{\\s*([^}]+)\\s*\\}");
    std::smatch match;
    
    std::string::const_iterator searchStart(css.cbegin());
    while (std::regex_search(searchStart, css.cend(), match, ruleRegex)) {
        std::string selector = match[1].str();
        std::string properties = match[2].str();
        
        // Store the rule
        m_cssRules[selector] = properties;
        
        // Move to the next match
        searchStart = match.suffix().first;
    }
    
    return true;
}

sk_sp<SkTypeface> SkiaRenderEngine::Impl::loadFont(const std::string& fontFamily, int weight, bool italic) {
    // In a real implementation, we would load the specified font
    // For now, just return the default font
    return SkTypeface::MakeDefault();
}

void SkiaRenderEngine::Impl::renderElement(SkCanvas* canvas, xmlNode* node, int x, int y, int width) {
    if (!node || node->type != XML_ELEMENT_NODE) {
        return;
    }
    
    std::string nodeName(reinterpret_cast<const char*>(node->name));
    
    // Handle text nodes
    if (nodeName == "text" || nodeName == "#text") {
        xmlChar* content = xmlNodeGetContent(node);
        if (content) {
            std::string text(reinterpret_cast<const char*>(content));
            
            // Create paint for text
            SkPaint paint;
            paint.setColor(SK_ColorBLACK);
            
            // Create font
            SkFont font;
            font.setSize(16);
            
            // Draw text
            canvas->drawSimpleText(text.c_str(), text.length(), SkTextEncoding::kUTF8, x, y + 16, font, paint);
            
            xmlFree(content);
        }
    }
    
    // Handle other elements
    // This is a very simplified implementation
    // In a real implementation, we would handle different HTML elements properly
    
    // Render child nodes
    for (xmlNode* child = node->children; child; child = child->next) {
        renderElement(canvas, child, x, y + 20, width);
    }
}

std::string SkiaRenderEngine::Impl::highlightCode(const std::string& code, const std::string& language) {
    // In a real implementation, we would implement code highlighting
    // For now, just return the code as is
    return code;
}

sk_sp<SkImage> SkiaRenderEngine::Impl::loadImage(const std::string& path) {
    try {
        // Read the image file
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return nullptr;
        }
        
        // Get the file size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read the file into a buffer
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return nullptr;
        }
        
        // Create a SkData from the buffer
        sk_sp<SkData> data = SkData::MakeWithCopy(buffer.data(), buffer.size());
        
        // Decode the image
        sk_sp<SkImage> image = SkImage::MakeFromEncoded(data);
        
        return image;
    }
    catch (const std::exception& e) {
        // Handle exception
        return nullptr;
    }
    catch (...) {
        // Handle unknown exception
        return nullptr;
    }
}

} // namespace text2image