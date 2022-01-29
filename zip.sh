#!/bin/bash

# Build executable in container and copy here.
./build.sh

# Make zip file in parent folder.
(cd ../ && zip -r assembler.zip \
    assembler/assembler \
    assembler/*.c \
    assembler/*.h \
    assembler/test/*.as \
    assembler/Makefile)