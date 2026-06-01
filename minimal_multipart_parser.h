//
// minimal_multipart_parser.h
//
// Copyright (c) 2024 Brian Khuu https://briankhuu.com/
// MIT licensed
//
// https://github.com/mofosyne/minimal-multipart-form-data-parser-c
//

#ifndef MINIMAL_MULTIPART_PARSER_H
#define MINIMAL_MULTIPART_PARSER_H

#include <stdbool.h>

// Size of the full boundary string we are searching for as a multipart file divider
// e.g. `\r\n--BOUNDARY` where BOUNDARY is a user specified 70 bytes long printable ascii string
#define MINIMAL_MULTIPART_PARSER_USER_BOUNDARY_SIZE (70)
#define MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR (4 + MINIMAL_MULTIPART_PARSER_USER_BOUNDARY_SIZE)

#define MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER "\r\n--"
#define MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER_COUNT (sizeof(MINIMAL_MULTIPART_PARSER_BOUNDARY_START_MARKER) - 1)

#define MINIMAL_MULTIPART_PARSER_FILE_START_MARKER "\r\n\r\n"
#define MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT (sizeof(MINIMAL_MULTIPART_PARSER_FILE_START_MARKER) - 1)

#define MINIMAL_MULTIPART_PARSER_MAX_CHAR (MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR)

typedef enum MultipartParserEvent
{
    MultipartParserEvent_None,
    MultipartParserEvent_FileStreamFound,
    MultipartParserEvent_FileStreamStarting,
    MultipartParserEvent_DataBufferAvailable,
    MultipartParserEvent_DataStreamCompleted
} MultipartParserEvent;

typedef enum MultipartParserPhase
{
    MultipartParserPhase_INIT,
    MultipartParserPhase_Preamble_SKIP_LINE,
    MultipartParserPhase_Preamble_CR,
    MultipartParserPhase_Preamble_LF,
    MultipartParserPhase_Preamble_HYPHEN,
    MultipartParserPhase_GetBoundary,
    MultipartParserPhase_GetBoundary_Done,
    MultipartParserPhase_SkipFileHeader,
    MultipartParserPhase_GetFileBytes,
    MultipartParserPhase_EndOfFile
} MultipartParserPhase;

typedef struct MinimalMultipartParserCharBuffer
{
    unsigned char count;
    char buffer[MINIMAL_MULTIPART_PARSER_MAX_CHAR + 1];
} MinimalMultipartParserCharBuffer;

typedef struct MinimalMultipartParserContext
{
    MultipartParserPhase phase;
    bool data_available;
    MinimalMultipartParserCharBuffer data;
    MinimalMultipartParserCharBuffer boundary;
} MinimalMultipartParserContext;

static inline const unsigned int minimal_multipart_parser_get_data_size(const MinimalMultipartParserContext *context) { return context->data.count; }

static inline const char *minimal_multipart_parser_get_data_buffer(const MinimalMultipartParserContext *context) { return context->data.buffer; }

static inline const bool minimal_multipart_parser_is_file_received(const MinimalMultipartParserContext *context) { return context->phase == MultipartParserPhase_EndOfFile; }

MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c);

#endif
