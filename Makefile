CC      ?= cc
CP      ?= cp -f
RM      ?= rm -f
MKDIR   ?= mkdir -p
INSTALL ?= install

PREFIX  ?= /usr/local

CFLAGS += -Wall -std=c99 -pedantic


.PHONY: all
all: multipart_extract test readme_update

# Dev Note: $ is used by both make and AWK. Must escape $ for use in AWK within makefile.
.PHONY: readme_update
readme_update:
	./parser_size.sh | ./readme_update.sh

.PHONY: install
install: multipart_extract
	$(MKDIR) $(PREFIX)/bin
	$(INSTALL) multipart_extract $(PREFIX)/bin/multipart_extract

.PHONY: uninstall
uninstall:
	$(RM)  $(PREFIX)/bin/multipart_extract

.PHONY: multipart_extract
multipart_extract: multipart_extract.c minimal_multipart_parser_embedded.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -g0 -Os $^ -o $@
	size multipart_extract
	./multipart_extract_test.sh

.PHONY: test
test: test.c minimal_multipart_parser_with_debug.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -g2 -O0 $^ -o $@
	size test
	@./test

.PHONY: format
format:
	# pip install clang-format
	clang-format -i *.c
	clang-format -i *.h

.PHONY: clean
clean:
	$(RM) *.o *.so *.aarch64.elf 
	$(RM) multipart_extract
	$(RM) test

# Static Library - Standard
minimal_multipart_parser.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@

# Static Library - Embedded - No debug (-g0) and optimize for size (-Os)
minimal_multipart_parser_embedded.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c -g0 -Os $^ -o $@

# Static Library - Development - Max debug (-g2) and optimize for compile speed and debuggability (-O0)
minimal_multipart_parser_with_debug.o: minimal_multipart_parser.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -c -g2 -O0 $^ -o $@
