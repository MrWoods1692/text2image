/**
 * Text2Image Node.js Test
 * Copyright (c) 2025 Text2Image contributors
 *
 * This is a simple test script for the Text2Image Node.js module.
 */

const { Text2Image, Resolution, Format, BackgroundType } = require('./index');

// Create a new instance
const text2image = new Text2Image();

// HTML content to render
const html = `
<!DOCTYPE html>
<html>
<head>
    <title>Test Document</title>
</head>
<body>
    <div class="container">
        <h1>Hello, World!</h1>
        <p>This is a test document for the Text2Image library.</p>
        <ul>
            <li>Item 1</li>
            <li>Item 2</li>
            <li>Item 3</li>
        </ul>
        <table border="1">
            <tr>
                <th>Name</th>
                <th>Value</th>
            </tr>
            <tr>
                <td>Item 1</td>
                <td>100</td>
            </tr>
            <tr>
                <td>Item 2</td>
                <td>200</td>
            </tr>
        </table>
        <div class="code">
            <pre><code class="language-javascript">
function hello() {
    console.log("Hello, World!");
}
            </code></pre>
        </div>
    </div>
</body>
</html>
`;

// CSS styles
const css = `
body {
    font-family: Arial, sans-serif;
    line-height: 1.6;
    color: #333;
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
}

h1 {
    color: #2c3e50;
    border-bottom: 2px solid #3498db;
    padding-bottom: 10px;
}

ul {
    background-color: #f8f9fa;
    padding: 20px;
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
}

tr:hover {
    background-color: #f5f5f5;
}

.code {
    background-color: #2c3e50;
    color: #ecf0f1;
    padding: 15px;
    border-radius: 5px;
    overflow-x: auto;
    margin: 20px 0;
}

.container {
    background-color: white;
    border-radius: 10px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    padding: 30px;
}
`;

// Render options
const options = {
    resolution: Resolution['1080P'],
    format: Format.PNG,
    quality: 90,
    backgroundColor: 0xFFFFFFFF,
    borderRadius: 10,
    backgroundType: BackgroundType.SOLID
};

console.log('Creating render task...');
const task = text2image.createTask(html, css, options);

// Test synchronous rendering
console.log('Rendering synchronously...');
const success = text2image.render(task, 'output_sync.png');
if (success) {
    console.log('Synchronous rendering completed successfully!');
} else {
    console.error('Synchronous rendering failed:', text2image.getLastError());
}

// Test asynchronous rendering
console.log('Rendering asynchronously...');
text2image.renderAsync(task, 'output_async.png', (err, success) => {
    if (err) {
        console.error('Asynchronous rendering failed:', err);
    } else if (success) {
        console.log('Asynchronous rendering completed successfully!');
        
        // Test getting result in memory
        console.log('Getting result in memory...');
        try {
            const buffer = text2image.getResult(task);
            console.log(`Got result buffer with size: ${buffer.length} bytes`);
            
            // Save buffer to file
            const fs = require('fs');
            fs.writeFileSync('output_buffer.png', buffer);
            console.log('Saved buffer to output_buffer.png');
        } catch (error) {
            console.error('Failed to get result:', error);
        } finally {
            // Free the task
            console.log('Freeing task...');
            text2image.freeTask(task);
            
            // Shutdown the library
            console.log('Shutting down library...');
            text2image.shutdown();
            
            console.log('Test completed!');
        }
    } else {
        console.error('Asynchronous rendering failed without error');
        
        // Free the task
        text2image.freeTask(task);
        
        // Shutdown the library
        text2image.shutdown();
    }
});