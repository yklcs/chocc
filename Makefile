CC 							= clang
CCFLAGS 				= -std=c90 -Wall -Wextra -Wpedantic -Werror
CCFLAGS_DEBUG 	= -g3 -fsanitize=address,undefined
OUT 						= chocc
SOURCES					= main.c parse.c token.c io.c

.PHONY: all debug build clean run

all: 		build
debug: 	CC += $(CCFLAGS_DEBUG)
debug: 	build

build: $(SOURCES)
	$(CC) $(CCFLAGS) $^ -o $(OUT)

clean:
	rm $(OUT)
