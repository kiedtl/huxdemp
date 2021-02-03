CMD      = @

VERSION  = 0.1.0
NAME     = hxd
SRC      =
SRC3     =
OBJ      = $(SRC:.c=.o)
OBJ3     = $(SRC3:.c=.o)

WARNING  = -Wall -Wpedantic -Wextra -Wold-style-definition -Wmissing-prototypes \
	   -Winit-self -Wfloat-equal -Wstrict-prototypes -Wredundant-decls \
	   -Wendif-labels -Wstrict-aliasing=2 -Woverflow -Wformat=2 -Wtrigraphs \
	   -Wmissing-include-dirs -Wno-format-nonliteral -Wunused-parameter \
	   -Wincompatible-pointer-types \
	   -Werror=implicit-function-declaration -Werror=return-type

DEF      = -DVERSION=\"$(VERSION)\"
INCL     =
CC       = clang
CFLAGS   = $(DEF) $(INCL) $(WARNING) -funsigned-char
LD       = bfd
LDFLAGS  = -fuse-ld=$(LD) -L/usr/include

.PHONY: all
all: debug

.PHONY: run
run: debug
	$(CMD)./$(NAME) examples/ctrlsilm.txt

.PHONY: debug
debug: O_CFLAGS  := -Og -g $(CFLAGS)
debug: O_LDFLAGS :=
debug: $(NAME)

.PHONY: release
release: O_CFLAGS  := -O3 $(CFLAGS)
release: O_LDFLAGS := -flto -s -march=native -mtune=native
release: $(NAME)

main.c: tables.c utf8.c

$(NAME): $(OBJ) $(OBJ3) main.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(O_CFLAGS) $(O_LDFLAGS)

%.o: %.c
	@printf "    %-8s%s\n" "CC" $@
	$(CMD)$(CC) -c $< -o $@ $(O_CFLAGS)

%.c: %.unuc unu
	@printf "    %-8s%s\n" "UNU" $@
	$(CMD)./unu < $^ > $@

unu: unu.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf unu $(NAME) $(OBJ)

.PHONY: deepclean
deepclean: clean
	rm -rf $(OBJ3)
