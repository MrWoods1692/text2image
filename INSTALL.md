# Text2Image 安装指南

本指南将帮助您在不同平台上安装和配置Text2Image库。

## 系统要求

### 硬件要求
- CPU: 支持SSE4.2指令集的处理器
- 内存: 至少2GB RAM（推荐4GB或更多）
- 磁盘空间: 至少500MB可用空间

### 软件要求
- C++17兼容的编译器
  - Windows: Visual Studio 2017或更高版本
  - macOS: Xcode 11或更高版本，或Clang
  - Linux: GCC 7或更高版本，或Clang
- CMake 3.14或更高版本
- Python 3.6或更高版本（用于构建）
- Node.js 10.x或更高版本（仅Node.js绑定需要）

## 依赖库

Text2Image依赖以下第三方库：

- **Skia**: 2D图形库
- **libxml2**: HTML解析
- **FreeType**: 字体渲染
- **libvips**: 图像处理

## Windows安装

### 使用预编译的二进制包

1. 从下载最新的Windows二进制包。

2. 解压下载的压缩包到您选择的目录，例如`C:\text2image`。

3. 将`C:\text2image\bin`添加到系统环境变量`PATH`中。

4. 将`C:\text2image\lib`添加到系统环境变量`LIB`中。

5. 将`C:\text2image\include`添加到系统环境变量`INCLUDE`中。

### 从源码编译

1. 安装依赖：
   - 使用[vcpkg](https://github.com/microsoft/vcpkg)安装依赖：
     ```cmd
     vcpkg install skia libxml2 freetype libvips
     ```

2. 克隆仓库：
   ```cmd
   git clone https://github.com/MrWoods1692/text2image.git
   cd text2image
   ```

3. 创建并进入构建目录：
   ```cmd
   mkdir build
   cd build
   ```

4. 配置CMake：
   ```cmd
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

5. 构建项目：
   ```cmd
   cmake --build . --config Release
   ```

6. 安装库：
   ```cmd
   cmake --install . --config Release
   ```

## macOS安装

### 使用Homebrew

```bash
brew tap text2image/text2image
brew install text2image
```

### 从源码编译

1. 安装依赖：
   ```bash
   brew install cmake libxml2 freetype vips
   ```

2. 克隆仓库：
   ```bash
   git clone https://github.com/MrWoods1692/text2image.git
   cd text2image
   ```

3. 运行构建脚本：
   ```bash
   ./build.sh
   ```

4. 或者手动构建：
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   sudo cmake --install .
   ```

## Linux安装

### Ubuntu/Debian

1. 安装依赖：
   ```bash
   sudo apt-get update
   sudo apt-get install -y cmake build-essential libxml2-dev libfreetype6-dev libvips-dev
   ```

2. 克隆仓库：
   ```bash
   git clone https://github.com/MrWoods1692/text2image.git
   cd text2image
   ```

3. 运行构建脚本：
   ```bash
   ./build.sh
   ```

### CentOS/RHEL

1. 安装依赖：
   ```bash
   sudo yum install -y cmake gcc-c++ libxml2-devel freetype-devel vips-devel
   ```

2. 克隆仓库：
   ```bash
   git clone https://github.com/MrWoods1692/text2image.git
   cd text2image
   ```

3. 运行构建脚本：
   ```bash
   ./build.sh
   ```

## Node.js绑定安装

### 从npm安装

```bash
npm install text2image
```

### 从源码编译

1. 确保已安装Text2Image库（见上文）。

2. 克隆仓库：
   ```bash
   git clone https://github.com/MrWoods1692/text2image.git
   cd text2image/nodejs
   ```

3. 安装依赖并构建：
   ```bash
   npm install
   ```

## 验证安装

### C/C++验证

创建一个简单的C++程序`test.cpp`：

```cpp
#include <iostream>
#include <text2image.h>

int main() {
    if (Text2Image_Init()) {
        std::cout << "Text2Image library initialized successfully!" << std::endl;
        
        // 创建一个简单的渲染任务
        const char* html = "<p>Hello, Text2Image!</p>";
        Text2Image_TaskHandle task = Text2Image_CreateTask(html, nullptr, nullptr);
        
        if (task) {
            // 渲染到文件
            if (Text2Image_Render(task, "test_output.png")) {
                std::cout << "Image rendered successfully to test_output.png" << std::endl;
            } else {
                std::cout << "Rendering failed: " << Text2Image_GetLastError() << std::endl;
            }
            
            // 释放任务
            Text2Image_FreeTask(task);
        } else {
            std::cout << "Failed to create task: " << Text2Image_GetLastError() << std::endl;
        }
        
        // 关闭库
        Text2Image_Shutdown();
    } else {
        std::cout << "Failed to initialize Text2Image library!" << std::endl;
    }
    
    return 0;
}
```

编译并运行：

```bash
g++ -o test test.cpp -ltext2image
./test
```

### Node.js验证

创建一个简单的Node.js脚本`test.js`：

```javascript
const { Text2Image } = require('text2image');

try {
    const text2image = new Text2Image();
    console.log('Text2Image library initialized successfully!');
    
    const html = '<p>Hello, Text2Image!</p>';
    const task = text2image.createTask(html);
    
    text2image.render(task, 'test_output.png');
    console.log('Image rendered successfully to test_output.png');
    
    text2image.freeTask(task);
    text2image.shutdown();
} catch (error) {
    console.error('Error:', error);
}
```

运行：

```bash
node test.js
```

## 环境变量

Text2Image支持以下环境变量：

- `TEXT2IMAGE_FONT_PATH`: 自定义字体路径
- `TEXT2IMAGE_CACHE_DIR`: 缓存目录
- `TEXT2IMAGE_MAX_THREADS`: 最大线程数

## 故障排除

### 找不到库文件

- 确保已将库安装到系统路径中
- 在编译时指定库路径：`-L/path/to/lib`
- 在运行时设置`LD_LIBRARY_PATH`（Linux/macOS）或`PATH`（Windows）

### 依赖库问题

- 确保所有依赖库都已正确安装
- 使用`ldd`（Linux）或`otool -L`（macOS）检查库依赖
- 在Windows上使用Dependency Walker检查DLL依赖

### Node.js绑定问题

- 确保Node.js版本兼容（推荐10.x或更高版本）
- 检查Python版本（构建需要Python 3.6+）
- 确保已正确安装Text2Image系统库

## 联系支持

如果您在安装过程中遇到问题，请通过以下方式寻求支持：

- GitHub Issues: https://github.com/MrWoods1692/text2image/issues
- 电子邮件: mail@mrcwoods.com