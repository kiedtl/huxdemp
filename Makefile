CMD      = @

VERSION  = 0.1.0
NAME     = hxd
PKGNAME  = $(NAME)-$(shell uname -s)-$(shell uname -m)-$(VERSION)

DESTDIR  =
PREFIX   = /usr/local

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
CC       = cc
CFLAGS   = $(DEF) $(INCL) $(WARNING) -funsigned-char
LD       = bfd
LDFLAGS  = -fuse-ld=$(LD) -L/usr/include

.PHONY: all
all: debug $(NAME).1

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

%.1: %.scd.1
	@printf "    %-8s%s\n" "SCDOC" $@
	$(CMD)scdoc < $^ > $@

unu: unu.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf unu main.c $(NAME) $(OBJ)
	rm -rf $(PKGNAME) $(PKGNAME).tar.xz

.PHONY: deepclean
deepclean: clean
	rm -rf $(OBJ3)

.PHONY: install
install: release $(NAME).1
	install -Dm755 $(NAME)   $(DESTDIR)/$(PREFIX)/bin/$(NAME)
	install -Dm644 $(NAME).1 $(DESTDIR)/$(PREFIX)/share/man/man1/$(NAME).1

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(NAME)
	rm -f $(DESTDIR)/$(PREFIX)/share/man/man1/$(NAME).1

.PHONY: dist
dist: release $(NAME).1
	$(CMD)mkdir $(PKGNAME)
	$(CMD)cp $(NAME)   $(PKGNAME)
	$(CMD)cp $(NAME).1 $(PKGNAME)
	$(CMD)tar -cf - $(PKGNAME) | xz -qcT0 > $(PKGNAME).tar.xz
	$(CMD)rm -rf $(PKGNAME)
