/**
 * Text2Image Node.js Module
 * Copyright (c) 2025 Text2Image contributors
 *
 * This is the main entry point for the Text2Image Node.js module.
 */

// Load the native module
const native = require('./build/Release/text2image');

/**
 * Text2Image class
 */
class Text2Image {
  constructor() {
    // Initialize the library
    this.initialized = native.initialize();
    if (!this.initialized) {
      throw new Error('Failed to initialize Text2Image library');
    }
  }

  /**
   * Create a new render task
   * @param {string} html - HTML content to render
   * @param {string} [css=''] - CSS styles to apply
   * @param {Object} [options={}] - Render options
   * @returns {Object} Task object
   */
  createTask(html, css = '', options = {}) {
    return native.createTask(html, css, options);
  }

  /**
   * Render a task synchronously
   * @param {Object} task - Task object
   * @param {string} [outputPath=null] - Output file path
   * @returns {boolean} True if rendering was successful
   */
  render(task, outputPath = null) {
    return native.render(task, outputPath);
  }

  /**
   * Render a task asynchronously
   * @param {Object} task - Task object
   * @param {string} [outputPath=null] - Output file path
   * @param {Function} callback - Callback function (err, success)
   */
  renderAsync(task, outputPath = null, callback) {
    if (typeof outputPath === 'function') {
      callback = outputPath;
      outputPath = null;
    }
    native.renderAsync(task, outputPath, callback);
  }

  /**
   * Get the rendered image data
   * @param {Object} task - Task object
   * @returns {Buffer} Image data buffer
   */
  getResult(task) {
    return native.getResult(task);
  }

  /**
   * Free a task
   * @param {Object} task - Task object
   */
  freeTask(task) {
    native.freeTask(task);
  }

  /**
   * Get the last error message
   * @returns {string|null} Error message or null if no error
   */
  getLastError() {
    return native.getLastError();
  }

  /**
   * Set the maximum number of threads to use
   * @param {number} numThreads - Maximum number of threads (0 = auto-detect)
   */
  setMaxThreads(numThreads) {
    native.setMaxThreads(numThreads);
  }

  /**
   * Get default render options
   * @returns {Object} Default render options
   */
  getDefaultOptions() {
    return native.getDefaultOptions();
  }

  /**
   * Shutdown the library and release resources
   */
  shutdown() {
    if (this.initialized) {
      native.shutdown();
      this.initialized = false;
    }
  }
}

// Export constants
const Resolution = native.Resolution;
const Format = native.Format;
const BackgroundType = native.BackgroundType;

// Export the module
module.exports = {
  Text2Image,
  Resolution,
  Format,
  BackgroundType,
  
  // Create a default instance
  instance: new Text2Image(),
  
  // Export methods for convenience (using default instance)
  createTask: (html, css, options) => module.exports.instance.createTask(html, css, options),
  render: (task, outputPath) => module.exports.instance.render(task, outputPath),
  renderAsync: (task, outputPath, callback) => module.exports.instance.renderAsync(task, outputPath, callback),
  getResult: (task) => module.exports.instance.getResult(task),
  freeTask: (task) => module.exports.instance.freeTask(task),
  getLastError: () => module.exports.instance.getLastError(),
  setMaxThreads: (numThreads) => module.exports.instance.setMaxThreads(numThreads),
  getDefaultOptions: () => module.exports.instance.getDefaultOptions(),
  shutdown: () => module.exports.instance.shutdown()
};