CFLAGS := -Wall -ansi -pedantic -g
SRCS := $(wildcard *.c)
DEPS := $(patsubst %.c, %.d, $(SRCS))
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: clean

assembler: $(OBJS)
	$(CC) -o $@ $(OBJS)

include $(SRCS:.c=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	-rm -rf assembler *.o *.d
