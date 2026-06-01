//
// minimal_multipart_parser.c
//
// Copyright (c) 2024 Brian Khuu https://briankhuu.com/
// MIT licensed
//
// https://github.com/mofosyne/minimal-multipart-form-data-parser-c
//

#include "minimal_multipart_parser.h"
#include <stdbool.h>

static inline unsigned int buffer_count(MinimalMultipartParserCharBuffer *context) { return context->count; }

static inline void buffer_reset(MinimalMultipartParserCharBuffer *context) { context->count = 0; }

static inline bool buffer_add(MinimalMultipartParserCharBuffer *context, const char c)
{
    if (context->count >= MINIMAL_MULTIPART_PARSER_FULL_BOUNDARY_BUFFER_MAX_CHAR)
    {
        return false;
    }

    context->buffer[context->count++] = c;
    return true;
}

MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c)
{
    MinimalMultipartParserCharBuffer *boundaryBuffer = &(context->boundary);
    MinimalMultipartParserCharBuffer *dataBuffer = &(context->data);

    if (context->data_available)
    {
        buffer_reset(dataBuffer);
        context->data_available = false;
    }

    if (context->phase == MultipartParserPhase_INIT)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_Preamble_CR;
                return MultipartParserEvent_None;
            case '-':
                context->phase = MultipartParserPhase_Preamble_HYPHEN;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_Preamble_SKIP_LINE)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_Preamble_CR;
                return MultipartParserEvent_None;
            case '\n':
                context->phase = MultipartParserPhase_Preamble_LF;
                return MultipartParserEvent_None;
            default:
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_Preamble_CR)
    {
        switch (c)
        {
            case '\n':
                context->phase = MultipartParserPhase_Preamble_LF;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_Preamble_LF)
    {
        switch (c)
        {
            case '-':
                context->phase = MultipartParserPhase_Preamble_HYPHEN;
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_Preamble_HYPHEN)
    {
        // Previously got a dash in '\r\n--', seeking another dash
        switch (c)
        {
            case '-':
                context->phase = MultipartParserPhase_GetBoundary;
                buffer_add(boundaryBuffer, '\r');
                buffer_add(boundaryBuffer, '\n');
                buffer_add(boundaryBuffer, '-');
                buffer_add(boundaryBuffer, '-');
                return MultipartParserEvent_None;
            default:
                context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_GetBoundary)
    {
        switch (c)
        {
            case '\r':
                context->phase = MultipartParserPhase_GetBoundary_Done;
                return MultipartParserEvent_None;
            default:
                if ((c < ' ') || ('~' < c))
                {
                    context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                    buffer_reset(boundaryBuffer);
                    return MultipartParserEvent_None;
                }
                if (!buffer_add(boundaryBuffer, c))
                {
                    context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                    buffer_reset(boundaryBuffer);
                    return MultipartParserEvent_None;
                }
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_GetBoundary_Done)
    {
        switch (c)
        {
            case '\n':
                context->phase = MultipartParserPhase_SkipFileHeader;
                return MultipartParserEvent_FileStreamFound;
            default:
                context->phase = MultipartParserPhase_Preamble_SKIP_LINE;
                buffer_reset(boundaryBuffer);
                return MultipartParserEvent_None;
        }
    }

    if (context->phase == MultipartParserPhase_SkipFileHeader)
    {
        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = (MINIMAL_MULTIPART_PARSER_FILE_START_MARKER)[expected_index];
        if (c != expected_char)
        {
            buffer_reset(dataBuffer);
            return MultipartParserEvent_None;
        }

        buffer_add(dataBuffer, c);
        if (buffer_count(dataBuffer) >= MINIMAL_MULTIPART_PARSER_FILE_START_MARKER_COUNT)
        {
            context->phase = MultipartParserPhase_GetFileBytes;
            buffer_reset(dataBuffer);
            return MultipartParserEvent_FileStreamStarting;
        }

        return MultipartParserEvent_None;
    }

    if (context->phase == MultipartParserPhase_GetFileBytes)
    {
        const unsigned char full_boundary_size = buffer_count(boundaryBuffer);
        const char *full_boundary_string = context->boundary.buffer;

        const unsigned expected_index = buffer_count(dataBuffer);
        const char expected_char = full_boundary_string[expected_index];

        buffer_add(dataBuffer, c);

        if (c != expected_char)
        {
            context->data_available = true;
            return MultipartParserEvent_DataBufferAvailable;
        }

        if (buffer_count(dataBuffer) >= full_boundary_size)
        {
            context->phase = MultipartParserPhase_EndOfFile;
            return MultipartParserEvent_DataStreamCompleted;
        }

        return MultipartParserEvent_None;
    }

    if (context->phase == MultipartParserPhase_EndOfFile)
    {
        // Do nothing... We already got the file now
        return MultipartParserEvent_DataStreamCompleted;
    }

    return MultipartParserEvent_None;
}
