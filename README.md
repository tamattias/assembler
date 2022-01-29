# Assembler

Assembler for Systems Programming Lab university project.

## Build instructions

With GCC and GNU Make installed run:

```bash
make
```

## Usage

Pass the basenames of the source files to `assembler`.

For example:

```bash
./assembler test/ps test/good test/bad
```

## Run tests

```bash
make test
```

## Generate documentation

With Doxygen installed run

```bash
make docs
```

The generated documentation will reside in the `docs/html` folder. You can
open it in your browser to view documentation for the various project files.
