CFLAGS := -Wall -ansi -pedantic

.PHONY: clean

assembler: assembler.o
	$(CC) -o $@ $^


assembler.o: assembler.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf assembler *.o



