/* Copyright (c) 2020, Jalen Adams
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

enum log_level {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

#define log_trace(...) log_message(TRACE, stdout, __VA_ARGS__)
#define log_debug(...) log_message(DEBUG, stdout, __VA_ARGS__)
#define log_info(...)  log_message(INFO, stdout, __VA_ARGS__)
#define log_warn(...)  log_message(WARN, stderr, __VA_ARGS__)
#define log_error(...) log_message(ERROR, stderr, __VA_ARGS__)
#define log_fatal(...) log_message(FATAL, stderr, __VA_ARGS__)

void log_message(enum log_level level, FILE *fp, const char *fmt, ...);

#endif /* LOGGER_H */

