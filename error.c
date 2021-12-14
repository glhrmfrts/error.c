/**
 * error.c - Error handling.
 * Written by Guilherme Nemeth. Public Domain.
 */

#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

typedef struct error {
    const char* filename;
    int line;
    const char* funcname;
    const char* msg;
    char* format_msg;
    size_t msglen;
    int code;
    void* user_data;
    struct error* from;
} error_t;

#define error_new(code, fmt, ...) (error_new_(__FILE__, __LINE__, __func__, (code), (fmt), __VA_ARGS__))

#define error_from(err) (error_from_(__FILE__, __LINE__, __func__, (err)))

#define error_try(act) { error_t* err = (act); if (err) { return error_from(err); }}

#define noerror (NULL)

#ifdef error_unittest
static int error_mem_usage;
#endif

static inline error_t* error_new_(const char* filename, int line, const char* funcname, int code, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int sz = vsnprintf(NULL, 0, fmt, args);
    
    error_t* result = calloc(1, sizeof(error_t) + sz + 1);
    
#ifdef error_unittest
    error_mem_usage += sizeof(error_t) + sz + 1;
#endif

    char* buf = (char*)(result + 1);

    vsnprintf(buf, sz + 1, fmt, args);
    va_end(args);

    result->filename = filename;
    result->line = line;
    result->funcname = funcname;
    result->msg = buf;
    result->msglen = sz;
    result->code = code;
    result->user_data = NULL;
    result->from = NULL;
    
    return result;
}

static inline error_t* error_from_(const char* filename, int line, const char* funcname, error_t* err) {
    error_t* result = calloc(1, sizeof(error_t));

#ifdef error_unittest
    error_mem_usage += sizeof(error_t);
#endif

    result->from = err;
    result->filename = filename;
    result->line = line;
    result->funcname = funcname;
    result->msg = err->msg;
    result->msglen = err->msglen;
    result->code = err->code;
    return result;
}

static inline int error_formatbuf(const error_t* err, char* buf, size_t bufsize) {
    int offs = 0;
    int inc = (bufsize == 0) ? 0 : 1;
    const error_t* cerr = err;
    while (cerr) {
        if (cerr == err) {
            offs += snprintf(buf + (offs*inc), bufsize - (offs*inc), "%s; at %s (%s:%d);", cerr->msg, cerr->funcname, cerr->filename, cerr->line);
        } else {
            offs += snprintf(buf + (offs*inc), bufsize - (offs*inc), " %s (%s:%d);", cerr->funcname, cerr->filename, cerr->line);
        }
        cerr = cerr->from;
    }
    return offs;
}

static inline char* error_format(error_t* err) {
    if (!err) { return NULL; }
    int sz = error_formatbuf(err, NULL, 0);
    char* buf = malloc(sz + 1);

#ifdef error_unittest
    error_mem_usage += sz + 1;
#endif

    error_formatbuf(err, buf, sz + 1);
    err->format_msg = buf;
    return buf;
}

static inline void error_free(error_t* err) {
    error_t* cerr = err;
    while (cerr) {
        error_t* next = cerr->from;

#ifdef error_unittest
        error_mem_usage -= sizeof(error_t);
        if (cerr == err) {
            error_mem_usage -= cerr->msglen + 1;
        }
        if (cerr->format_msg) {
            error_mem_usage -= strlen(cerr->format_msg) + 1;
        }
#endif

        if (cerr->format_msg) {
            free(cerr->format_msg);
        }
        free(cerr);
        cerr = next;
    }
}

#ifdef error_unittest

#include <assert.h>

error_t* func1_noerror() {
    return noerror;
}

error_t* func2_noerror() {
    error_try(func1_noerror());
    return noerror;
}

error_t* func1() {
    return error_new(0, "something bad happened", "");
}

error_t* func2() {
    error_try(func1());
    return noerror;
}

error_t* func3() {
    error_try(func2());
    return noerror;
}

void testerror() {
    error_t* err = func3();
    if (err) {
        printf("error_mem_usage=%zu\n", error_mem_usage);
        printf("%s\n", error_format(err));
        printf("error_mem_usage=%zu\n", error_mem_usage);
        error_free(err);
        printf("error_mem_usage=%zu\n", error_mem_usage);
    }
    assert(error_mem_usage == 0);

    err = func2_noerror();
    assert(err == noerror);
}

#endif