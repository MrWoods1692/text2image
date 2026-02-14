/*
 * Text2Image Task Implementation
 * Copyright (c) 2025 Text2Image contributors
 *
 * This file contains the implementation of the Task class.
 */

#include "text2image_internal.h"

#include <cstdlib>
#include <algorithm>

namespace text2image {

Task::Task(const std::string& html, const std::string& css, const Text2Image_RenderOptions& options)
    : m_html(html)
    , m_css(css)
    , m_options(options)
    , m_status(TaskStatus::PENDING)
    , m_priority(TaskPriority::NORMAL)
    , m_callback(nullptr)
    , m_userData(nullptr) {
    // Generate a unique handle for this task
    m_handle = reinterpret_cast<Text2Image_TaskHandle>(this);
}

Task::~Task() {
}

} // namespace text2image