//
// test.c
//
// Copyright (c) 2024 Brian Khuu
// MIT licensed
//

#include "minimal_multipart_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

char *MultipartParserEvent_To_Str(MultipartParserEvent event)
{
    switch (event)
    {
        case MultipartParserEvent_None:
            return "None";
        case MultipartParserEvent_FileStreamFound:
            return "File Stream Found";
        case MultipartParserEvent_FileStreamStarting:
            return "File Stream Starting";
        case MultipartParserEvent_DataBufferAvailable:
            return "Data Buffer Available";
        case MultipartParserEvent_DataStreamCompleted:
            return "Data Stream Completed";
        default:
            return "?";
    }
}

bool test_case(const char *title, const char *input, const unsigned int input_size, const char *expected, const unsigned int expected_byte_count, MultipartParserPhase expected_end_phase)
{
    bool passed = true;
    char received_file_buffer[1000] = {0};
    unsigned int received_file_byte_count = 0;

    MinimalMultipartParserContext state = {0};
    for (int i = 0; i < input_size; i++)
    {
        const char c = input[i];
#ifdef DEBUG
        printf("[%x : %c]\n", c, isprint(c) ? c : c == '\r' ? 'r' : c == '\n' ? 'n' : '?');
#endif
        const MultipartParserEvent event = minimal_multipart_parser_process(&state, c);
        if (event != MultipartParserEvent_None)
        {
#ifdef DEBUG
            printf("Event: %s\n", MultipartParserEvent_To_Str(event));
#endif
            if (event == MultipartParserEvent_DataBufferAvailable)
            {
                for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++)
                {
                    const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
                    received_file_buffer[received_file_byte_count++] = rx;
                }
            }
            else if (event == MultipartParserEvent_DataStreamCompleted)
            {
                // Finished
                break;
            }
        }
    }

    if (state.phase != expected_end_phase)
    {
        passed = false;
    }

    if (received_file_byte_count != expected_byte_count)
    {
        passed = false;
    }

    if (expected_byte_count == 0 && received_file_byte_count != 0)
    {
        passed = false;
    }

    if (expected_byte_count != 0 && memcmp(expected, received_file_buffer, expected_byte_count) != 0)
    {
        passed = false;
    }

    if (!passed)
    {
        printf("Case '%s' Failed\n", title);
        printf("Expected (%d): '%s'\n", expected_byte_count, expected);
        printf("Got (%d): '%s'\n", received_file_byte_count, received_file_buffer);
        printf("\n");
    }
    else
    {
        printf("Case '%s' Passed\n", title);
    }
    return passed;
}

bool test_case1(void)
{
    const char input[] = "POST / HTTP/1.1\r\n"
                         "Host: localhost:8000\r\n"
                         "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:29.0) Gecko/20100101 Firefox/29.0\r\n"
                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                         "Accept-Language: en-US,en;q=0.5\r\n"
                         "Accept-Encoding: gzip, deflate\r\n"
                         "Cookie: __atuvc=34%7C7; permanent=0; _gitlab_session=226ad8a0be43681acf38c2fab9497240; __profilin=p%3Dt; request_method=GET\r\n"
                         "Connection: keep-alive\r\n"
                         "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266\r\n"
                         "Content-Length: 554\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "text default\r\n"
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = "text default";

    return test_case("full http standard style", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case2(void)
{
    const char input[] = "POST / HTTP/1.1\r\n"
                         "Host: localhost:8000\r\n"
                         "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:29.0) Gecko/20100101 Firefox/29.0\r\n"
                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                         "Accept-Language: en-US,en;q=0.5\r\n"
                         "Accept-Encoding: gzip, deflate\r\n"
                         "Cookie: __atuvc=34%7C7; permanent=0; _gitlab_session=226ad8a0be43681acf38c2fab9497240; __profilin=p%3Dt; request_method=GET\r\n"
                         "Connection: keep-alive\r\n"
                         "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266\r\n"
                         "Content-Length: 554\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "text default\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "Content of a.txt.\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"file2\"; filename=\"a.html\"\r\n"
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         "<!DOCTYPE html><title>Content of a.html.</title>\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = "text default";

    return test_case("multipayload", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case3(void)
{
    const char input[] = "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "text default\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "Content of a.txt.\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"file2\"; filename=\"a.html\"\r\n"
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         "<!DOCTYPE html><title>Content of a.html.</title>\r\n"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = "text default";

    return test_case("cgi style body only", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case4(void)
{
    const char input[] = "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "\x00"
                         "\x01"
                         "\x02"
                         "\x03"
                         "\r\n"
                         "\x00"
                         "\r\n"
                         "-----------------------------9051914041544843365972754266--\r\n";

    const char expected[] = {0x00, 0x01, 0x02, 0x03, '\r', '\n', 0x00};

    return test_case("binary payload", input, sizeof(input) - 1, expected, sizeof(expected), MultipartParserPhase_EndOfFile);
}

bool test_case5(void)
{
    const char input[] = "-----------------------------9051914041544843365972754266\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "\x00"
                         "\x01"
                         "\x02";

    const char expected[] = {0x00, 0x01, 0x02};

    return test_case("Interrupted file transfer", input, sizeof(input) - 1, expected, sizeof(expected), MultipartParserPhase_GetFileBytes);
}

bool test_case6(void)
{
    const char input[] = "POST / HTTP/1.1\r\n"
                         "Host: localhost:8000\r\n"
                         "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:29.0) Gecko/20100101 Firefox/29.0\r\n"
                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                         "Accept-Language: en-US,en;q=0.5\r\n"
                         "Accept-Encoding: gzip, deflate\r\n"
                         "Connection: keep-alive\r\n"
                         "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266\r\n"
                         "Content-Len";

    return test_case("No file stream found", input, strlen(input), NULL, 0, MultipartParserPhase_Preamble_SKIP_LINE);
}

bool test_case7(void)
{
    // Content with a sequence that partially matches the boundary before diverging.
    // Boundary "\r\n--BOUN" shares a 6-byte prefix with "\r\n--BOX" in the content.
    // All 7 buffered bytes ("\r\n--BOX", including the mismatching char) must appear
    // in the output — nothing silently dropped.
    const char input[] = "--BOUN\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "data\r\n--BOXX rest\r\n--BOUN--\r\n";

    const char expected[] = "data\r\n--BOXX rest";

    return test_case("false boundary match in content", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case8(void)
{
    // File part with zero bytes of actual content: the boundary immediately follows
    // the file-start marker. Parser must reach EndOfFile without emitting any data.
    const char input[] = "--BOUNDARY\r\n"
                         "Content-Disposition: form-data; name=\"empty\"\r\n"
                         "\r\n"
                         "\r\n--BOUNDARY--\r\n";

    return test_case("empty file content", input, strlen(input), NULL, 0, MultipartParserPhase_EndOfFile);
}

bool test_case9(void)
{
    // A non-printable byte inside the first boundary candidate forces the parser back
    // to preamble scanning. The valid boundary on the following line must still be found.
    // Also exercises that removing buffer_reset from the GetBoundary error path is safe:
    // the HYPHEN handler's direct writes unconditionally reinitialise the boundary buffer.
    const char input[] = "--BAD\x01BOUNDARY\r\n"
                         "--GOODBOUND\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "hello\r\n--GOODBOUND--\r\n";

    const char expected[] = "hello";

    return test_case("boundary error recovery", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case10(void)
{
    // 70-character boundary fills the user-boundary buffer exactly (the maximum).
    // Verifies the buffer-size constants have no off-by-one.
    const char input[] = "--AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "max boundary test\r\n"
                         "--AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA--\r\n";

    const char expected[] = "max boundary test";

    return test_case("maximum boundary length", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

bool test_case11(void)
{
    // 71-character boundary exceeds the buffer; buffer_add returns false and the parser
    // falls back to SKIP_LINE. A valid boundary on the next line must still be accepted.
    const char input[] = "--BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\r\n"
                         "--GOODBOUND\r\n"
                         "Content-Disposition: form-data; name=\"text\"\r\n"
                         "\r\n"
                         "hello\r\n--GOODBOUND--\r\n";

    const char expected[] = "hello";

    return test_case("boundary overflow recovery", input, strlen(input), expected, strlen(expected), MultipartParserPhase_EndOfFile);
}

int main(int argc, char **argv)
{
    printf("Testing Minimal Multipart Form Data Parser\n");
    printf("GCC Version: v%s\n", __VERSION__);

    if (!test_case1())
    {
        return 1;
    }

    if (!test_case2())
    {
        return 1;
    }

    if (!test_case3())
    {
        return 1;
    }

    if (!test_case4())
    {
        return 1;
    }

    if (!test_case5())
    {
        return 1;
    }

    if (!test_case6())
    {
        return 1;
    }

    if (!test_case7())
    {
        return 1;
    }

    if (!test_case8())
    {
        return 1;
    }

    if (!test_case9())
    {
        return 1;
    }

    if (!test_case10())
    {
        return 1;
    }

    if (!test_case11())
    {
        return 1;
    }

    printf("PASSED\n");
    return 0;
}
