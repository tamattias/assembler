#!/bin/bash

# Build image.
docker build . -t openu-assembler

# Build assembler inside container.
docker rm -f openu-assembler # Delete old container.
docker run --name openu-assembler openu-assembler

# Clean local build files.
make clean

# Copy built executable here.
docker cp openu-assembler:/assembler/assembler .
