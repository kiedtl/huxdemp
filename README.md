![logo](img/logo.png)

## huxdemp (aka `hxd`)

`huxdemp` is an advanced version of the venerable `hexdump` that uses
colors, bolding, and other terminal formatting to distinguish between `nul`
bytes, ASCII whitespace, UTF8-control characters, ASCII control characters,
and normal printable bytes.

**NOTE**: This should be considered `beta` software. Expect a multitude of
bugs and snakes, and count the directories in your $HOME after you use
`huxdemp`.

### Seeing is believing

![hxd reading /dev/input/mouse](img/mouse.png)
![hxd reading /dev/input/mouse](img/mouse2.png)
![hxd reading part of a PNG image](img/png.png)
![hxd reading /dev/random](img/rnd.png)
![hxd reading a snippet from The Silmarillion](img/silm.png)
![hxd reading some UTF8 text, demonstrating the -u flag](img/utf8.png)

### Features/Anti-features

- No octal dumping (this *might* be added later).
- "Highlights" bytes that "belong" to the same UTF8-encoded character.
- Readable source, written in literate programming using a dialect of `unu`
  from [RetroForth](https://forth.works/).
- Ability to print characters in IBM's code page 437 (see screenshots).
- Ability to print control characters using fancy Unicode glyphs (e.g. ␀
  for NUL, ␌ for FF (form feed), etc).
- Not written in Rust. (The horrors!)

### Install

#### Requirements

- A POSIX system. (Windows is not supported at present.)
- [`scdoc`](https://git.sr.ht/~sircmpwn/scdoc) (*for building from source*)
- A C99 compiler and GNU Make (*for building from source*)

---

Download a binary tarball from the releases, unpack, and move the
executable into your $PATH and the manpage (`hxd.1`) to someplace like
`/usr/local/share/man/man1/hxd.1`.

Or, to build from source, clone (or download a tarball from the releases),
make, and install:

```
$ cd huxdemp
$ make
    CCLD    unu
    UNU     main.c
    CCLD    hxd
    SCDOC  
$ sudo make install
[sudo] password for kiedtl:
$
```

As far as I am aware, there are no distributions that carry `huxdemp` in
their repositories.

### TODO

- hexdump's **-n* and *-s* option.
- An option to change the order/format of the columns.
- A $HXD_COLORS variable to change the display style of the ASCII/byte
  column.
- A flag to display the ASCII column in bold.
- A flag to "mark" the nth byte of the input.
- Use `pledge(2)`/`unveil(2)` on OpenBSD.
- Support Winbl^B^B^Kdows 10.

### License

This project is licensed under the GPLv3 license, with the exception of
`utf8.c`, which comes from [`termbox`](https://github.com/nsf/termbox) and
is licensed under the MIT license.
