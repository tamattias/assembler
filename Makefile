#
# Assembler Makefile
# Author: Tamir Attias
#

# Compiler flags.
CFLAGS := -Wall -ansi -pedantic -g

# Source files.
SRCS := $(wildcard *.c)

# Dependency files.
DEPS := $(patsubst %.c, %.d, $(SRCS))

# Object files.
OBJS := $(patsubst %.c, %.o, $(SRCS))

# Phony targets.
.PHONY: docs test clean

# Assembler executable target.
assembler: $(OBJS)
	$(CC) -o $@ $(OBJS)

# Include dependency files.
include $(SRCS:.c=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#
# The recipe generates a dependency Makefile for each source file.
# Taken from the GNU Makefile manual.
# See https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
# CPPFLAGS was changed to CFLAGS.
#
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

test: assembler
	@echo Testing good source file.
	./assembler test/good
	@echo Testing bad first pass source file.
	-./assembler test/bad_first
	@echo Testing bad second pass source file.
	-./assembler test/bad_second
	@echo Testing example in course workbook.
	./assembler test/ps

# Target for easy debugging with GDB.
debug: assembler
	gdb -ex run --args ./assembler test/ps

# Generate documentation using Doxygen
docs:
	doxygen

clean:
	-rm -rf assembler *.o *.d docs/
