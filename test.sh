#!/bin/bash

# Clean local build files
make clean

# Build using docker
docker build . -t openu-assembler

# Run tests inside container
docker rm -f openu-assembler
docker run --name openu-assembler openu-assembler cat /etc/os-release && make test
