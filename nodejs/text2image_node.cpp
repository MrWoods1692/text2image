/*
 * Text2Image Node.js Binding
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the Node.js binding implementation for the Text2Image library.
 */

#include <napi.h>
#include <string>
#include <memory>
#include <vector>

#include "../include/text2image.h"

// Forward declarations
Napi::Value Initialize(const Napi::CallbackInfo& info);
Napi::Value Shutdown(const Napi::CallbackInfo& info);
Napi::Value CreateTask(const Napi::CallbackInfo& info);
Napi::Value Render(const Napi::CallbackInfo& info);
Napi::Value RenderAsync(const Napi::CallbackInfo& info);
Napi::Value GetResult(const Napi::CallbackInfo& info);
Napi::Value FreeTask(const Napi::CallbackInfo& info);
Napi::Value GetLastError(const Napi::CallbackInfo& info);
Napi::Value SetMaxThreads(const Napi::CallbackInfo& info);
Napi::Value GetDefaultOptions(const Napi::CallbackInfo& info);

// Task reference management
std::unordered_map<Text2Image_TaskHandle, Napi::ObjectReference> g_taskRefs;
std::mutex g_taskRefsMutex;

// Async worker for render operations
class RenderAsyncWorker : public Napi::AsyncWorker {
public:
    RenderAsyncWorker(Napi::Function& callback, Text2Image_TaskHandle task, const std::string& outputPath)
        : Napi::AsyncWorker(callback), task_(task), outputPath_(outputPath), success_(false) {}

    ~RenderAsyncWorker() {}

    // Executed in worker thread
    void Execute() override {
        success_ = Text2Image_Render(task_, outputPath_.empty() ? nullptr : outputPath_.c_str());
        if (!success_) {
            errorMessage_ = Text2Image_GetLastError();
        }
    }

    // Executed in main thread after Execute completes
    void OnOK() override {
        Callback().Call({Env().Null(), Napi::Boolean::New(Env(), success_)});
    }

    void OnError(const Napi::Error& e) override {
        Callback().Call({e.Value(), Napi::Boolean::New(Env(), false)});
    }

private:
    Text2Image_TaskHandle task_;
    std::string outputPath_;
    bool success_;
    std::string errorMessage_;
};

// Initialize the module
Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    // Initialize the Text2Image library
    if (!Text2Image_Init()) {
        Napi::Error::New(env, "Failed to initialize Text2Image library").ThrowAsJavaScriptException();
        return exports;
    }

    // Export functions
    exports.Set("initialize", Napi::Function::New<Initialize>(env));
    exports.Set("shutdown", Napi::Function::New<Shutdown>(env));
    exports.Set("createTask", Napi::Function::New<CreateTask>(env));
    exports.Set("render", Napi::Function::New<Render>(env));
    exports.Set("renderAsync", Napi::Function::New<RenderAsync>(env));
    exports.Set("getResult", Napi::Function::New<GetResult>(env));
    exports.Set("freeTask", Napi::Function::New<FreeTask>(env));
    exports.Set("getLastError", Napi::Function::New<GetLastError>(env));
    exports.Set("setMaxThreads", Napi::Function::New<SetMaxThreads>(env));
    exports.Set("getDefaultOptions", Napi::Function::New<GetDefaultOptions>(env));

    // Export constants
    Napi::Object resolution = Napi::Object::New(env);
    resolution.Set("AUTO", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_AUTO));
    resolution.Set("720P", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_720P));
    resolution.Set("1080P", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_1080P));
    resolution.Set("2K", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_2K));
    resolution.Set("4K", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_4K));
    resolution.Set("8K", Napi::Number::New(env, TEXT2IMAGE_RESOLUTION_8K));
    exports.Set("Resolution", resolution);

    Napi::Object format = Napi::Object::New(env);
    format.Set("PNG", Napi::Number::New(env, TEXT2IMAGE_FORMAT_PNG));
    format.Set("JPG", Napi::Number::New(env, TEXT2IMAGE_FORMAT_JPG));
    format.Set("JPEG", Napi::Number::New(env, TEXT2IMAGE_FORMAT_JPEG));
    format.Set("WEBP", Napi::Number::New(env, TEXT2IMAGE_FORMAT_WEBP));
    format.Set("BMP", Napi::Number::New(env, TEXT2IMAGE_FORMAT_BMP));
    format.Set("TIF", Napi::Number::New(env, TEXT2IMAGE_FORMAT_TIF));
    format.Set("TIFF", Napi::Number::New(env, TEXT2IMAGE_FORMAT_TIFF));
    format.Set("HEIC", Napi::Number::New(env, TEXT2IMAGE_FORMAT_HEIC));
    format.Set("HEIF", Napi::Number::New(env, TEXT2IMAGE_FORMAT_HEIF));
    format.Set("AVIF", Napi::Number::New(env, TEXT2IMAGE_FORMAT_AVIF));
    exports.Set("Format", format);

    Napi::Object backgroundType = Napi::Object::New(env);
    backgroundType.Set("SOLID", Napi::Number::New(env, TEXT2IMAGE_BACKGROUND_SOLID));
    backgroundType.Set("IMAGE", Napi::Number::New(env, TEXT2IMAGE_BACKGROUND_IMAGE));
    exports.Set("BackgroundType", backgroundType);

    return exports;
}

// Initialize function
Napi::Value Initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool result = Text2Image_Init();
    return Napi::Boolean::New(env, result);
}

// Shutdown function
Napi::Value Shutdown(const Napi::CallbackInfo& info) {
    Text2Image_Shutdown();
    return info.Env().Undefined();
}

// Convert JavaScript options object to Text2Image_RenderOptions
Text2Image_RenderOptions ConvertOptions(const Napi::Object& jsOptions) {
    Text2Image_RenderOptions options = Text2Image_GetDefaultOptions();

    // Resolution
    if (jsOptions.Has("resolution")) {
        options.resolution = static_cast<Text2Image_Resolution>(jsOptions.Get("resolution").ToNumber().Int32Value());
    }

    // Format
    if (jsOptions.Has("format")) {
        options.format = static_cast<Text2Image_Format>(jsOptions.Get("format").ToNumber().Int32Value());
    }

    // Quality
    if (jsOptions.Has("quality")) {
        options.quality = jsOptions.Get("quality").ToNumber().Int32Value();
    }

    // Custom dimensions
    if (jsOptions.Has("customWidth")) {
        options.customWidth = jsOptions.Get("customWidth").ToNumber().Int32Value();
    }
    if (jsOptions.Has("customHeight")) {
        options.customHeight = jsOptions.Get("customHeight").ToNumber().Int32Value();
    }

    // Background settings
    if (jsOptions.Has("backgroundType")) {
        options.backgroundType = static_cast<Text2Image_BackgroundType>(jsOptions.Get("backgroundType").ToNumber().Int32Value());
    }
    if (jsOptions.Has("backgroundColor")) {
        options.backgroundColor = jsOptions.Get("backgroundColor").ToNumber().Uint32Value();
    }
    if (jsOptions.Has("backgroundImage")) {
        options.backgroundImage = jsOptions.Get("backgroundImage").ToString().Utf8Value().c_str();
    }
    if (jsOptions.Has("backgroundBlur")) {
        options.backgroundBlur = jsOptions.Get("backgroundBlur").ToNumber().FloatValue();
    }

    // Border radius
    if (jsOptions.Has("borderRadius")) {
        options.borderRadius = jsOptions.Get("borderRadius").ToNumber().Int32Value();
    }

    // Additional options
    if (jsOptions.Has("enableJavaScript")) {
        options.enableJavaScript = jsOptions.Get("enableJavaScript").ToBoolean().Value();
    }
    if (jsOptions.Has("timeout")) {
        options.timeout = jsOptions.Get("timeout").ToNumber().Int32Value();
    }

    return options;
}

// CreateTask function
Napi::Value CreateTask(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 1) {
        Napi::Error::New(env, "Expected at least 1 argument (html)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get HTML content
    std::string html = info[0].ToString().Utf8Value();

    // Get CSS content (optional)
    std::string css = "";
    if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
        css = info[1].ToString().Utf8Value();
    }

    // Get options (optional)
    Text2Image_RenderOptions options = Text2Image_GetDefaultOptions();
    if (info.Length() > 2 && info[2].IsObject()) {
        options = ConvertOptions(info[2].ToObject());
    }

    // Create the task
    Text2Image_TaskHandle task = Text2Image_CreateTask(html.c_str(), css.c_str(), &options);
    if (!task) {
        Napi::Error::New(env, Text2Image_GetLastError()).ThrowAsJavaScriptException();
        return env.Null();
    }

    // Create a JavaScript object to hold the task handle
    Napi::Object taskObj = Napi::Object::New(env);
    taskObj.Set("handle", Napi::External<Text2Image_TaskHandle>::New(env, &task));

    // Store a reference to the task object
    {
        std::lock_guard<std::mutex> lock(g_taskRefsMutex);
        g_taskRefs[task] = Napi::ObjectReference::New(taskObj);
    }

    return taskObj;
}

// Render function
Napi::Value Render(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 1) {
        Napi::Error::New(env, "Expected at least 1 argument (task)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get task handle
    Text2Image_TaskHandle task = nullptr;
    if (info[0].IsObject()) {
        Napi::Object taskObj = info[0].ToObject();
        if (taskObj.Has("handle") && taskObj.Get("handle").IsExternal()) {
            task = *taskObj.Get("handle").As<Napi::External<Text2Image_TaskHandle>>().Data();
        }
    }

    if (!task) {
        Napi::Error::New(env, "Invalid task object").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get output path (optional)
    const char* outputPath = nullptr;
    if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
        outputPath = info[1].ToString().Utf8Value().c_str();
    }

    // Render the task
    bool result = Text2Image_Render(task, outputPath);
    if (!result) {
        Napi::Error::New(env, Text2Image_GetLastError()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    return Napi::Boolean::New(env, true);
}

// RenderAsync function
Napi::Value RenderAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 2) {
        Napi::Error::New(env, "Expected at least 2 arguments (task, callback)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get task handle
    Text2Image_TaskHandle task = nullptr;
    if (info[0].IsObject()) {
        Napi::Object taskObj = info[0].ToObject();
        if (taskObj.Has("handle") && taskObj.Get("handle").IsExternal()) {
            task = *taskObj.Get("handle").As<Napi::External<Text2Image_TaskHandle>>().Data();
        }
    }

    if (!task) {
        Napi::Error::New(env, "Invalid task object").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get callback
    Napi::Function callback = info[info.Length() - 1].As<Napi::Function>();

    // Get output path (optional)
    std::string outputPath = "";
    if (info.Length() > 2 && !info[1].IsUndefined() && !info[1].IsNull() && info[1].IsString()) {
        outputPath = info[1].ToString().Utf8Value();
    }

    // Create and queue the async worker
    RenderAsyncWorker* worker = new RenderAsyncWorker(callback, task, outputPath);
    worker->Queue();

    return env.Undefined();
}

// GetResult function
Napi::Value GetResult(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 1) {
        Napi::Error::New(env, "Expected at least 1 argument (task)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get task handle
    Text2Image_TaskHandle task = nullptr;
    if (info[0].IsObject()) {
        Napi::Object taskObj = info[0].ToObject();
        if (taskObj.Has("handle") && taskObj.Get("handle").IsExternal()) {
            task = *taskObj.Get("handle").As<Napi::External<Text2Image_TaskHandle>>().Data();
        }
    }

    if (!task) {
        Napi::Error::New(env, "Invalid task object").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get the result
    uint8_t* buffer = nullptr;
    size_t size = 0;
    bool result = Text2Image_GetResult(task, &buffer, &size);
    if (!result) {
        Napi::Error::New(env, Text2Image_GetLastError()).ThrowAsJavaScriptException();
        return env.Null();
    }

    // Create a Buffer from the result
    Napi::Buffer<uint8_t> bufferObj = Napi::Buffer<uint8_t>::New(env, buffer, size);

    // Free the buffer (the Buffer object will now own the memory)
    Text2Image_FreeBuffer(buffer);

    return bufferObj;
}

// FreeTask function
Napi::Value FreeTask(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 1) {
        Napi::Error::New(env, "Expected at least 1 argument (task)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get task handle
    Text2Image_TaskHandle task = nullptr;
    if (info[0].IsObject()) {
        Napi::Object taskObj = info[0].ToObject();
        if (taskObj.Has("handle") && taskObj.Get("handle").IsExternal()) {
            task = *taskObj.Get("handle").As<Napi::External<Text2Image_TaskHandle>>().Data();
        }
    }

    if (!task) {
        Napi::Error::New(env, "Invalid task object").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Remove the reference
    {
        std::lock_guard<std::mutex> lock(g_taskRefsMutex);
        auto it = g_taskRefs.find(task);
        if (it != g_taskRefs.end()) {
            it->second.Reset();
            g_taskRefs.erase(it);
        }
    }

    // Free the task
    Text2Image_FreeTask(task);

    return env.Undefined();
}

// GetLastError function
Napi::Value GetLastError(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    const char* error = Text2Image_GetLastError();
    if (!error) {
        return env.Null();
    }
    return Napi::String::New(env, error);
}

// SetMaxThreads function
Napi::Value SetMaxThreads(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check arguments
    if (info.Length() < 1) {
        Napi::Error::New(env, "Expected at least 1 argument (numThreads)").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get number of threads
    int numThreads = info[0].ToNumber().Int32Value();

    // Set max threads
    Text2Image_SetMaxThreads(numThreads);

    return env.Undefined();
}

// GetDefaultOptions function
Napi::Value GetDefaultOptions(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Get default options
    Text2Image_RenderOptions options = Text2Image_GetDefaultOptions();

    // Create JavaScript object
    Napi::Object jsOptions = Napi::Object::New(env);
    jsOptions.Set("resolution", Napi::Number::New(env, options.resolution));
    jsOptions.Set("format", Napi::Number::New(env, options.format));
    jsOptions.Set("quality", Napi::Number::New(env, options.quality));
    jsOptions.Set("customWidth", Napi::Number::New(env, options.customWidth));
    jsOptions.Set("customHeight", Napi::Number::New(env, options.customHeight));
    jsOptions.Set("backgroundType", Napi::Number::New(env, options.backgroundType));
    jsOptions.Set("backgroundColor", Napi::Number::New(env, options.backgroundColor));
    jsOptions.Set("backgroundBlur", Napi::Number::New(env, options.backgroundBlur));
    jsOptions.Set("borderRadius", Napi::Number::New(env, options.borderRadius));
    jsOptions.Set("enableJavaScript", Napi::Boolean::New(env, options.enableJavaScript));
    jsOptions.Set("timeout", Napi::Number::New(env, options.timeout));

    return jsOptions;
}

// Module registration
NODE_API_MODULE(text2image, InitModule)