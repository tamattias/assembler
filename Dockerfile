FROM ubuntu:16.04
WORKDIR /assembler
RUN apt-get update && apt-get install -y gcc make
COPY . .
CMD make test