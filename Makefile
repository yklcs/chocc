CC 							?= clang
CCFLAGS 				+= -std=c90 -Wall -Wextra -Wpedantic -Werror
CCFLAGS_DEBUG 	+= -g3 -fsanitize=address,undefined
BIN 						= chocc
LIB							= chocc.so
SOURCES					= main.c parse.c io.c lex.c cpp.c

.PHONY: all debug build clean test

all: 		build

debug: 	CC += $(CCFLAGS_DEBUG)
debug: 	build

build: $(BIN) $(LIB)

$(BIN): $(SOURCES)
	$(CC) $(CCFLAGS) $^ -o $@

$(LIB): $(SOURCES)
	$(CC) $(CCFLAGS) -shared $^ -o $@

clean:
	rm $(OUT) $(LIB)

test: clean $(LIB)
	pytest
