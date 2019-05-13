RM     ?= rm
CC      = gcc
CFLAGS  = -g -fPIC -std=c89 -Wall -Wpedantic
LFLAGS  = -g -fPIC -shared -Wall -Wpedantic

SRC=\
	luacurses.c\
	lc_lib.c\
	lc_window.c\
	lc_panel.c\
	lc_chstr.c
OBJ=$(SRC:.c=.o)
OUT=core.so

main: $(OBJ)
	$(CC) $(LFLAGS) -o $(OUT) $(OBJ)

$(SRC):
	$(CC) $(CFLAGS) $@

clean:
	@$(RM) $(OBJ) $(OUT)

.PHONY: clean
