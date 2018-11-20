DEBUG    := 0
CFLAGS   := -Wall -Wextra -Werror -Wpedantic -Wstrict-aliasing -O3 -DDEBUG=$(DEBUG)
LDFLAGS  := -O3
SRC      := $(wildcard *.c)
OBJ      := $(SRC:.c=.o)

main: $(OBJ)

.PHONY: debug
debug:
	$(MAKE) DEBUG=1

.PHONY: run
run:
	./main

.PHONY: format
format:
	clang-format -i *.c *.h

.PHONY: clean
clean:
	$(RM) *.o main
