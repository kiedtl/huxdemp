CMD      = @

LUA     ?= lua5.3
LUALIB  ?= lua5.3

VERSION  = 1.1.0
NAME     = huxd
PKGNAME  = $(NAME)-$(shell uname -s)-$(shell uname -m)-$(VERSION)

DESTDIR  =
PREFIX   = /usr/local

BUILTIN  = builtin/chip8.lua:chip8 builtin/uxn.lua:uxn builtin/ebcdic.lua:ebcdic
SRC      =
SRC3     =
OBJ      = $(SRC:.c=.o)
OBJ3     = $(SRC3:.c=.o)

WARNING  = -Wall -Wextra -Wold-style-definition -Wmissing-prototypes \
	   -Winit-self -Wfloat-equal -Wstrict-prototypes -Wredundant-decls \
	   -Wendif-labels -Wstrict-aliasing=2 -Woverflow -Wformat=2 -Wtrigraphs \
	   -Wmissing-include-dirs -Wunused-parameter -Werror=return-type \
	   -Wincompatible-pointer-types -Werror=implicit-function-declaration \
	   -Wno-unused-function

DEF      = -DVERSION=\"$(VERSION)\"
INCL     = -I/usr/include/$(LUA) -I/usr/local/include/$(LUA)
CC       = cc
CFLAGS   = $(DEF) $(INCL) $(WARNING) -funsigned-char
LD       = bfd
LDFLAGS  = -fuse-ld=$(LD) -L/usr/lib/$(LUA) -l$(LUALIB)

.PHONY: all
all: debug $(NAME).1

.PHONY: run
run: debug
	$(CMD)./$(NAME) examples/ctrlsilm.txt

.PHONY: debug
debug: O_CFLAGS  := -Og -g $(CFLAGS)
debug: O_LDFLAGS := $(LDFLAGS)
debug: $(NAME)

.PHONY: release
release: O_CFLAGS  := -O3 $(CFLAGS)
release: O_LDFLAGS := -flto -s -march=native -mtune=native $(LDFLAGS)
release: $(NAME)

main.c: lua.c tables.c utf8.c builtin.c

$(NAME): $(OBJ) $(OBJ3) main.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(O_CFLAGS) $(O_LDFLAGS)

%.o: %.c
	@printf "    %-8s%s\n" "CC" $@
	$(CMD)$(CC) -c $< -o $@ $(O_CFLAGS)

%.c: %.unuc unu
	@printf "    %-8s%s\n" "UNU" $@
	$(CMD)./unu $< < $< > $@

%.1: %.scd.1
	@printf "    %-8s%s\n" "SCDOC" $@
	$(CMD)scdoc < $^ > $@

builtin.c: embedc
builtin.c: $(foreach builtin,$(BUILTIN),$(firstword $(subst :, ,$(builtin))))
	@printf "    %-8s%s\n" "EMBEDC" $@
	$(CMD)./embedc $(BUILTIN) > $@

embedc: embedc.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

unu: unu.c
	@printf "    %-8s%s\n" "CCLD" $@
	$(CMD)$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf unu embedc builtin.c
	rm -rf main.c $(NAME) $(OBJ)
	rm -rf $(NAME).1
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
