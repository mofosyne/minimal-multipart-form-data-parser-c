# Minimal Multipart/Form-Data Parser in C (minimal-multipart-form-data-parser-c)

<versionBadge>![Version 1.0.0](https://img.shields.io/badge/version-1.0.0-blue.svg)</versionBadge>
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI/CD Status Badge](https://github.com/mofosyne/minimal-multipart-form-data-parser-c/actions/workflows/ci.yml/badge.svg)](https://github.com/mofosyne/minimal-multipart-form-data-parser-c/actions)

A lightweight C library and micro-utility for parsing HTTP multipart/form-data streams. 
Designed for embedded systems, it handles only one file per stream with no validation, 
prioritizing small code size over speed or features.

This tool adheres to the [Unix Philosophy](https://en.wikipedia.org/wiki/Unix_philosophy): it focuses on a single, well-defined task 
and works seamlessly with other programs through standard input/output chaining.

## Functionality

Given an HTTP multipart stream ([example adapted from Stack Overflow](https://stackoverflow.com/questions/4238809/example-of-multipart-form-data)):

```bash
POST / HTTP/1.1
Host: localhost:8000
Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266
Content-Length: 554

-----------------------------9051914041544843365972754266
Content-Disposition: form-data; name="text"

text default
-----------------------------9051914041544843365972754266
...
```

The parser extracts and outputs the first file's content:

```
text default
```


## Usage

The library consists of two files, easily integrated into your project manually or via [clib](https://github.com/clibs/clib)

```bash
clib install mofosyne/minimal-multipart-form-data-parser-c
```

or copy these files manually to your source folder:

```bash
minimal_multipart_parser.c
minimal_multipart_parser.h
```

### API

The core function processes input streams character by character:

```c
MultipartParserEvent minimal_multipart_parser_process(MinimalMultipartParserContext *context, const char c);
```

It emits one of the following events:

* `MultipartParserEvent_None` : No event yet; awaiting a file stream.
* `MultipartParserEvent_FileStreamFound` : A file stream has been detected.
* `MultipartParserEvent_FileStreamStarting` : A file stream is starting.
* `MultipartParserEvent_DataBufferAvailable` : File data is available (usually a byte or small buffer).
* `MultipartParserEvent_DataStreamCompleted` : File stream has ended.

Retrieve available data upon `MultipartParserEvent_DataBufferAvailable` event:

```c
unsigned int minimal_multipart_parser_get_data_size(const MinimalMultipartParserContext *context);
char *minimal_multipart_parser_get_data_buffer(const MinimalMultipartParserContext *context);
```

Check if the file was fully received:

```c
bool minimal_multipart_parser_is_file_received(const MinimalMultipartParserContext *context);
```

Example:

```c
int c;
static MinimalMultipartParserContext state = {0};
while ((c = getc(stdin)) != EOF)
{
    // Processor handles incoming stream character by character
    const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)c);

    // Handle Special Events
    if (event == MultipartParserEvent_DataBufferAvailable)
    {
        // Data Available To Receive
        for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++)
        {
            const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
            // Output received data
            putc(rx, stdout);
        }
    }
    else if (event == MultipartParserEvent_DataStreamCompleted)
    {
        // Data Stream Finished
        break;
    }
}

// Check if file has been received
if (minimal_multipart_parser_is_file_received(&state))
{
    // File Received Successfully
}
else
{
    // File Reception Failed
}
```

### `multipart_extract` Micro-Utility

A microutility named `multipart_extract` is provided and is installable and uninstallable via
these two command below (`PREFIX=/usr/local` can be omitted or adjusted as needed to your preferred
installation directory. By default it will install to `/usr/local/bin`).

```bash
sudo make PREFIX=/usr/local install
sudo make PREFIX=/usr/local uninstall
```

It processes an HTTP multipart stream from standard input and outputs the first file's content to standard output.

```bash
echo -e "-----------------------------9051914041544843365972754266\r\n"\
"Content-Disposition: form-data; name="text"\r\n"\
"\r\n"\
"text default\r\n"\
"-----------------------------9051914041544843365972754266--\r\n" \
| ./multipart_extract
```

Will output `text default`.


## Size

The parser object (`minimal_multipart_parser.c`) is compiled with `-Os -g0` and
measured in isolation — no libc, no stdio — to reflect true embedded footprint.

You can expect this library to consume around <flashSizeUsage>440</flashSizeUsage> bytes in flash and <ramSizeUsage>0</ramSizeUsage> bytes in RAM.

Breakdown of the parser object's ELF sections:

| `.text` (flash/code) | `.data` (flash+RAM) | `.bss` (RAM) |
| ---                  | ---                 | ---          |
| <dotTextSize>440</dotTextSize> B | <dotDataSize>0</dotDataSize> B | <dotBSSSize>0</dotBSSSize> B |


## Purpose For Existence

For use in very constrant devices

* Use in bootloader (e.g. uboot)
* Use in CGI scripts (e.g. busybox)
* Use in embedded devices (e.g. esp32)

For this purpose these are the restrictions to this code:

* Stick to C99 standard
* Keep standard library usage to minimum to control code size to a predictable level
* Must be streamable, so don't expect users to read in the whole file first.
* Minimise size of code so it can fit in embedded systems
* Minimise validation checks to keep code size low (aside from security considerations)
* Have no external dependencies. This makes it easier to integrate.

These are not considerations I am taking:

* Speed and cpu efficency is not of concern here. You would not typically be using this because you care about speed.
* Will not be tolerant of only `\n` even if the spec allows for receiving it, in order to minimise code size. 
    - Will only follow CRLF (`\r\n`), because most browsers follow RFC2616.

To that end I was able to:

* Only need `stdbool.h` from the standard C99 library or higher
* No malloc was required
* Inputs can be streamed byte by byte
* Buffering size kept to around the size of the boundary. Ergo `\r\n--` plus up to 70 characters.

For all other cases, recommend using an actual webserver.

## Other Similar Repositories To Consider

All these are C based repositories that are more developed with more features than our 
minimal implementation that you may want to consider if you need more features than this repo.

* [fremouw/multipart](https://github.com/fremouw/multipart)
    - Does not appear to use malloc
    - Need all post data upfront, the multipart_post_t struct only contains pointers to the original data; not delimited with \0.
    - Parses name, filename and content type
    - Intended for ESP8266 using ESP8266_RTOS_SDK
    - [Project considered complete according to Fremouw](https://github.com/fremouw/multipart/issues/1#issuecomment-2603247235)

* [iafonov/multipart-parser-c](https://github.com/iafonov/multipart-parser-c)
    - Uses malloc
    - Uses callback functions on each data reception
    - You need to parse `Content-Type` from the response head yourself to get the boundary
        - This is because the init function of that implementation requires it.
        - Ours simply assumes that the first line of this form `\r\n--BOUNDARY\r\n` is the boundary which is a safe assumption to make

* [francoiscolas/multipart-parser](https://github.com/francoiscolas/multipart-parser)
    - Does not appear to use malloc
    - Uses callback functions on each data reception
    - Validates and throws an error if not of the exact form. Ours doesn't not for code size consideration.
    - You need to parse `Content-Type` from the response head yourself to get the boundary
        - This is because the init function of that implementation requires it.
        - Ours simply assumes that the first line of this form `\r\n--BOUNDARY\r\n` is the boundary which is a safe assumption to make

* [abiiranathan/libmultipart](https://github.com/abiiranathan/libmultipart)
    - Uses malloc
    - Assumes you have already stored the whole request body in memory, ours will process the http request byte by byte.
    - But has extra features and validation for processing filenames, mimetype, etc...

## Assumptions

* You only care about one file
* You don't care about the metadata
* You will inspect the file type by inspecting the file itself
    - In most cases that can be done via the magic number and checking the file structure etc...
* That http multipart-form boundary will always start with `\r\n--` followed by a sequence of printable ASCII character (7bit range ascii only) and ending with a `\r\n` or `--\r\n` to indicate end of transmission.
    - This gives us the ability to take a shortcut and not bother with parsing `Content-Type`
    - This also means if it's missing, then we can assume this
    - We only care about one file... so we can skip the logic of finding the `\r\n` marker and stop search immediately.

## Reference:

* Test Case 1 taken from <https://stackoverflow.com/questions/4238809/example-of-multipart-form-data>
* Good reading <https://stackoverflow.com/questions/27993445/is-this-a-well-formed-multipart-form-data-request>
    - Start of multipart boundary is `\r\n--BOUNDARY` where BOUNDARY is specified in `Content-Type`
        - But this marker is very distinctive, so don't really need to check `Content-Type`
