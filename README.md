# Text2Image - 高性能文本转图像动态库

Text2Image是一个高效、高性能、高并发的文本转图像动态库，使用C++编写，支持多种编程语言调用，特别是Node.js。该库能够将HTML/CSS文本渲染为高质量图像，支持丰富的样式定制和多种输出格式。

## 特性

- **高性能渲染**：基于Skia图形库，提供高性能的HTML/CSS渲染
- **高并发处理**：单核千级并发能力，低资源占用
- **跨平台兼容**：支持Windows、macOS、Linux等主流操作系统
- **多语言支持**：提供C API，并支持Node.js、Python等语言绑定
- **多种输出格式**：支持PNG、JPG、WebP、BMP、TIFF、HEIC、AVIF等格式
- **多种分辨率**：支持自动、720P、1080P、2K、4K、8K等分辨率
- **丰富的样式支持**：完整支持CSS样式，包括列表、表格、颜色、边距、字体等
- **高级功能**：支持图片嵌入、代码高亮、背景设置、圆角等功能
- **自动画布尺寸**：根据内容自动调整画布大小

## 系统要求

### 编译环境

- C++17兼容编译器
  - Windows: Visual Studio 2017或更高版本
  - macOS: Xcode 11或更高版本，或Clang
  - Linux: GCC 7或更高版本，或Clang
- CMake 3.14或更高版本
- Python 3.6或更高版本（用于构建）

### 依赖库

- Skia：2D图形库
- libxml2：HTML解析
- FreeType：字体渲染
- libvips：图像处理
- Node.js 10.x或更高版本（仅Node.js绑定需要）

## 安装

### 从源码编译

#### 1. 克隆仓库

```bash
git clone https://github.com/MrWoods1692/text2image.git
cd text2image
```

#### 2. 安装依赖

##### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libxml2-dev libfreetype6-dev libvips-dev
```

##### macOS

```bash
brew install cmake libxml2 freetype vips
```

##### Windows

使用vcpkg安装依赖：

```bash
vcpkg install skia libxml2 freetype libvips
```

#### 3. 编译库

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### 4. 安装库

```bash
sudo cmake --install .
```

### Node.js绑定安装

```bash
cd nodejs
npm install
```

## 使用指南

### C/C++使用示例

```c
#include <stdio.h>
#include <text2image.h>

int main() {
    // 初始化库
    if (!Text2Image_Init()) {
        printf("初始化失败: %s\n", Text2Image_GetLastError());
        return 1;
    }

    // HTML内容
    const char* html = "<div class='container'><h1>Hello, World!</h1><p>这是一个测试。</p></div>";
    
    // CSS样式
    const char* css = "body { font-family: Arial, sans-serif; } .container { padding: 20px; background-color: #f0f0f0; border-radius: 8px; } h1 { color: #333; }";
    
    // 创建渲染选项
    Text2Image_RenderOptions options = Text2Image_GetDefaultOptions();
    options.resolution = TEXT2IMAGE_RESOLUTION_1080P;
    options.format = TEXT2IMAGE_FORMAT_PNG;
    options.quality = 90;
    options.backgroundColor = 0xFFFFFFFF;
    options.borderRadius = 8;
    
    // 创建渲染任务
    Text2Image_TaskHandle task = Text2Image_CreateTask(html, css, &options);
    if (!task) {
        printf("创建任务失败: %s\n", Text2Image_GetLastError());
        Text2Image_Shutdown();
        return 1;
    }
    
    // 同步渲染到文件
    if (Text2Image_Render(task, "output.png")) {
        printf("渲染成功!\n");
    } else {
        printf("渲染失败: %s\n", Text2Image_GetLastError());
    }
    
    // 释放任务
    Text2Image_FreeTask(task);
    
    // 关闭库
    Text2Image_Shutdown();
    
    return 0;
}
```

### Node.js使用示例

```javascript
const { Text2Image, Resolution, Format, BackgroundType } = require('text2image');

// 创建实例
const text2image = new Text2Image();

// HTML内容
const html = `
<div class="container">
    <h1>Hello, World!</h1>
    <p>这是一个测试。</p>
</div>
`;

// CSS样式
const css = `
body {
    font-family: Arial, sans-serif;
}
.container {
    padding: 20px;
    background-color: #f0f0f0;
    border-radius: 8px;
}
h1 {
    color: #333;
}
`;

// 渲染选项
const options = {
    resolution: Resolution['1080P'],
    format: Format.PNG,
    quality: 90,
    backgroundColor: 0xFFFFFFFF,
    borderRadius: 8
};

// 创建渲染任务
const task = text2image.createTask(html, css, options);

// 同步渲染
try {
    text2image.render(task, 'output.png');
    console.log('渲染成功!');
} catch (error) {
    console.error('渲染失败:', error);
} finally {
    // 释放任务
    text2image.freeTask(task);
}

// 异步渲染
const asyncTask = text2image.createTask(html, css, options);
text2image.renderAsync(asyncTask, 'output_async.png', (err, success) => {
    if (err) {
        console.error('异步渲染失败:', err);
    } else if (success) {
        console.log('异步渲染成功!');
    }
    
    // 释放任务
    text2image.freeTask(asyncTask);
    
    // 关闭库
    text2image.shutdown();
});
```

## API参考

### C API

#### 初始化和关闭

```c
// 初始化库
bool Text2Image_Init();

// 关闭库
void Text2Image_Shutdown();
```

#### 任务管理

```c
// 创建渲染任务
Text2Image_TaskHandle Text2Image_CreateTask(const char* html, const char* css, const Text2Image_RenderOptions* options);

// 释放任务
void Text2Image_FreeTask(Text2Image_TaskHandle task);
```

#### 渲染

```c
// 同步渲染
bool Text2Image_Render(Text2Image_TaskHandle task, const char* outputPath);

// 异步渲染
bool Text2Image_RenderAsync(Text2Image_TaskHandle task, const char* outputPath, Text2Image_RenderCallback callback, void* userData);

// 获取渲染结果（内存）
bool Text2Image_GetResult(Text2Image_TaskHandle task, uint8_t** buffer, size_t* size);

// 释放结果缓冲区
void Text2Image_FreeBuffer(uint8_t* buffer);
```

#### 配置

```c
// 设置最大线程数
void Text2Image_SetMaxThreads(int numThreads);

// 获取默认渲染选项
Text2Image_RenderOptions Text2Image_GetDefaultOptions();

// 获取最后错误信息
const char* Text2Image_GetLastError();
```

### Node.js API

#### 类和常量

```javascript
const { Text2Image, Resolution, Format, BackgroundType } = require('text2image');
```

#### 方法

```javascript
// 创建实例
const text2image = new Text2Image();

// 创建渲染任务
const task = text2image.createTask(html, css, options);

// 同步渲染
const success = text2image.render(task, outputPath);

// 异步渲染
text2image.renderAsync(task, outputPath, (err, success) => {
    // 处理结果
});

// 获取渲染结果（内存）
const buffer = text2image.getResult(task);

// 释放任务
text2image.freeTask(task);

// 设置最大线程数
text2image.setMaxThreads(numThreads);

// 获取默认渲染选项
const defaultOptions = text2image.getDefaultOptions();

// 获取最后错误信息
const error = text2image.getLastError();

// 关闭库
text2image.shutdown();
```

## 渲染选项

### C API

```c
typedef struct {
    // 分辨率设置
    Text2Image_Resolution resolution;  // 图像分辨率
    
    // 输出格式
    Text2Image_Format format;          // 输出图像格式
    
    // 质量设置 (0-100)
    int quality;                       // 图像质量
    
    // 自定义尺寸 (当resolution为AUTO时使用)
    int customWidth;                   // 自定义宽度（像素）
    int customHeight;                  // 自定义高度（像素）
    
    // 背景设置
    Text2Image_BackgroundType backgroundType;  // 背景类型
    uint32_t backgroundColor;                  // 背景颜色（ARGB格式）
    const char* backgroundImage;               // 背景图片路径
    float backgroundBlur;                      // 背景模糊程度（0-100）
    
    // 输出图片圆角
    int borderRadius;                   // 圆角半径（像素）
    
    // 其他渲染选项
    bool enableJavaScript;              // 启用JavaScript执行
    int timeout;                        // 渲染超时时间（毫秒）
} Text2Image_RenderOptions;
```

### Node.js API

```javascript
const options = {
    resolution: Resolution.AUTO,        // 分辨率: AUTO, 720P, 1080P, 2K, 4K, 8K
    format: Format.PNG,                 // 格式: PNG, JPG, WEBP, BMP, TIFF, HEIC, AVIF
    quality: 90,                        // 质量: 0-100
    customWidth: 0,                     // 自定义宽度（当resolution为AUTO时使用）
    customHeight: 0,                    // 自定义高度（当resolution为AUTO时使用）
    backgroundType: BackgroundType.SOLID, // 背景类型: SOLID, IMAGE
    backgroundColor: 0xFFFFFFFF,        // 背景颜色（ARGB格式）
    backgroundImage: null,              // 背景图片路径
    backgroundBlur: 0,                  // 背景模糊程度（0-100）
    borderRadius: 0,                    // 圆角半径（像素）
    enableJavaScript: false,            // 启用JavaScript执行
    timeout: 30000                      // 渲染超时时间（毫秒）
};
```

## 性能优化

1. **使用适当的分辨率**：根据实际需求选择合适的分辨率，避免不必要的高分辨率渲染
2. **优化HTML/CSS**：简化HTML结构和CSS样式，减少不必要的嵌套和复杂样式
3. **合理设置线程数**：根据系统CPU核心数设置适当的线程数，默认为CPU核心数
4. **使用异步渲染**：对于批量处理或不需要立即获取结果的场景，使用异步渲染
5. **缓存常用样式**：对于重复使用的样式，可以缓存任务或预编译CSS

## 常见问题

### 1. 渲染速度慢

- 检查HTML/CSS复杂度，简化不必要的嵌套和样式
- 降低输出分辨率或质量
- 增加线程数（通过`Text2Image_SetMaxThreads`）
- 使用异步渲染避免阻塞主线程

### 2. 内存占用高

- 及时释放任务（`Text2Image_FreeTask`）
- 避免同时创建大量任务
- 降低输出分辨率

### 3. 中文显示问题

- 确保系统中安装了中文字体
- 在CSS中指定中文字体：`font-family: "SimSun", "宋体", sans-serif;`

### 4. Node.js绑定编译失败

- 确保安装了Node.js开发工具
- 检查Node.js版本是否兼容（推荐10.x或更高版本）
- 确保已正确安装所有依赖库

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交问题报告、功能请求和代码贡献！请参阅CONTRIBUTING.md文件了解更多信息。

## 联系方式

- 项目主页：https://github.com/MrWoods1692/text2image
- 问题报告：https://github.com/MrWoods1692/text2image/issues
- 邮箱：mail@mrcwoods.com