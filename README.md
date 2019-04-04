# libsched

A library and set of command line utilities for generating task sets
and performing schedulability analysis. The library, libsched.a is
shared among all of the command line utilities listed below.

## Known Dependencies
- gcc		7.3.0
- glibc		2.27
- valgrind	3.13.0
- CUnit		2.1 (ubuntu libcunit1-dev)
- GSL		1.16 (ubuntu libsgl0-dev)
  GNU Scientific Library

## Directories
ex/
    Contains example files for task sets, and task set generation
    parameters.

src/
    All source files including headers (yes, it's a mess
    contributions are welcome).

lib/
    This is where libsched.a will be placed.

bin/
    Binaries will be placed here after being built

## Build Instructions

> make # at the top level, apologies no configure.

## Implemented Schedulability Tests

### Threads Per Job

From Tessler & Fisher 2019, bin/tpj

### Non-Preemptive Chunks

From Baruah 2005, bin/maxchunks


## Utilities

### Task Set Print

Compactly displays the contents of a task set, bin/ts-print

### Task Merge

Converts a multi-threaded task to a single threaded one based on its
WCET values, bin/ts-merge

### Task Set Growth Factor

Assignment of growth factors to tasks according to Tessler &
Fisher 2019, bin/ts-gf

### Task Set Divide

Divides a multi-threaded task into smaller tasks according to Tessler
& Fisher 2019, bin/ts-divide

### Deadline Assignment

From Bertogna & Baruah 2010, bin/ts-deadline-bb


