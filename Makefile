CC 							?= clang
CCFLAGS 				+= -std=c90 -Wall -Wextra -Wpedantic -Werror
CCFLAGS_DEBUG 	+= -g3 -fsanitize=address,undefined
BIN 						= chocc
LIB							= chocc.so
SOURCES					= parse.c io.c lex.c cpp.c error.c

.PHONY: all debug build clean test

all: 		build

debug: 	CC += $(CCFLAGS_DEBUG)
debug: 	build

build: $(BIN) $(LIB)

$(BIN): $(SOURCES) main.c
	$(CC) $(CCFLAGS) $^ -o $@

$(LIB): $(SOURCES)
	$(CC) $(CCFLAGS) -fPIC -shared $^ -o $@

clean:
	rm $(OUT) $(LIB)

test: clean $(LIB)
	pytest
