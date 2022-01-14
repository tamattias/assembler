CFLAGS := -Wall -ansi -pedantic -g

.PHONY: clean

assembler: assembler.o dynstr.o hashtable.o preprocessor.o instruction_set.o
	$(CC) -o $@ $^

assembler.o: assembler.c
	$(CC) $(CFLAGS) -c $< -o $@

dynstr.o: dynstr.c dynstr.h
	$(CC) $(CFLAGS) -c $< -o $@

hashtable.o: hashtable.c hashtable.h
	$(CC) $(CFLAGS) -c $< -o $@

preprocessor.o: preprocessor.c preprocessor.h dynstr.h hashtable.h constants.h
	$(CC) $(CFLAGS) -c $< -o $@

instruction_set.o: instruction_set.c instruction_set.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf assembler *.o



