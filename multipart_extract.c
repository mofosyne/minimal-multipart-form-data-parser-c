//
// multipart_extract.c
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//

// Minimal streaming utility: reads HTTP multipart/form-data from stdin,
// outputs the first file's content to stdout. Uses global static state
// to keep stack and code size small for embedded targets.

#include "minimal_multipart_parser.h"
#include <stdbool.h>
#include <stdio.h>

static MinimalMultipartParserContext state = {0};

int main(void)
{
    int c;
    while ((c = getc(stdin)) != EOF)
    {
        // Processor handles incoming stream character by character
        const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)c);

        // Special Events That Needs Handling
        if (event == MultipartParserEvent_DataBufferAvailable)
        {
            // Data Avaliable To Receive
            for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++)
            {
                const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
                putc(rx, stdout);
            }
        }
        else if (event == MultipartParserEvent_DataStreamCompleted)
        {
            // Datastream Finished
            break;
        }
    }

    // Stream ended without file?
    return minimal_multipart_parser_is_file_received(&state) ? 0 : 1;
}
