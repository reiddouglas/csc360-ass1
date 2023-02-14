
# Inspired by Ahmad Abdullah's makefile template

COMPILER=gcc
OPTIONS=-g

COMPILE=$(COMPILER) $(OPTIONS)
BUILD=build

PROGRAM=PMan

all: program


program: PMan.c $(BUILD)/linkedList.o
	$(COMPILE) $< $(BUILD)/*.o -o $(PROGRAM)


$(BUILD)/linkedList.o: linkedList.c PMan.h  build
	$(COMPILE) -c $< -o $@

$(BUILD)/foo.o: foo.c  build
	$(COMPILE) -c $< -o $@

# Make the build directory if it doesn't exist
build:
	mkdir -p $(BUILD)

# Delete the build directory and program
clean:
	rm -rf $(BUILD) program *.bak

# These rules do not correspond to a specific file
.PHONY: build clean