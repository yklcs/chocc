CC 							= clang
CCFLAGS 				= -std=c99
CCFLAGS_DEBUG 	= -g3 -Wall -fsanitize=address
OUT 						= chocc
SOURCES					= main.c
HEADERS					=

.PHONY: all debug build clean run

all: 		build
debug: 	CC += $(CCFLAGS_DEBUG)
debug: 	build

build: $(SOURCES)
	$(CC) $(CCFLAGS) $^ -o $(OUT)

clean:
	rm $(OUT)
