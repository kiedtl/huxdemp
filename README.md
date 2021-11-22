![logo](img/logo.png)

## huxdemp (aka `huxd`)

`huxdemp` is an advanced version of the venerable `hexdump` that uses
colors, bolding, and other terminal formatting to distinguish between `nul`
bytes, ASCII whitespace, UTF8-control characters, ASCII control characters,
and normal printable bytes.

**NOTE**: This should be considered `beta` software. Expect a multitude of
bugs and snakes, and count the directories in your `$HOME` after you use
this tool.

### Features/Anti-features

- "Highlights" bytes that "belong" to the same UTF8-encoded character.
- Ability to print characters in IBM's code page 437 (see screenshots).
- Ability to print control characters using fancy Unicode glyphs (e.g. ␀
  for NUL, ␌ for FF (form feed), etc).
- Automatic display through `less(1)` when needed.
- Readable source, written in literate programming using a dialect of `unu`
  from [RetroForth](https://forth.works/).

### Anti-features

- No Windows support.
- No octal dumping (this *might* be added later).
- Not written in Rust. (The horrors!)

### Seeing is believing

![huxd reading a snippet from The Silmarillion](img/silm.png)
![huxd reading some UTF8 text, demonstrating the -u flag](img/utf8.png)
![huxd reading /dev/input/mouse](img/mouse.png)
![huxd reading /dev/input/mouse](img/mouse2.png)
![huxd reading part of a PNG image](img/png.png)
![huxd reading /dev/random](img/rnd.png)

### Install

#### Requirements

- A POSIX system. (Windows is not supported at present.)
- [`scdoc`](https://git.sr.ht/~sircmpwn/scdoc) (*for building from source*)
- A C99 compiler and GNU Make (*for building from source*)

#### Arch Linux

```
yay huxd
```

#### Download

Download a binary tarball from the releases, unpack, and move the
executable into your $PATH and the manpage (`huxd.1`) to someplace like
`/usr/local/share/man/man1/huxd.1`.

---

#### Building from source

Clone (or download a tarball from the releases), make, and install:

```
$ cd huxdemp
$ make
    CCLD    unu
    UNU     main.c
    CCLD    huxd
    SCDOC  
$ sudo make install
[sudo] password for kiedtl:
$
```

### TODO

- A flag to "mark" the nth byte of the input.
- An option to change the order/format of the columns.
- A flag to display the ASCII column in bold.
- A $HXD_COLORS variable to change the display style of the ASCII/byte column.
- Unicode rune pane.
- Use skeeto's branchless utf8 decoder design to make this tool Blazing Fast™.
- Use `pledge(2)`/`unveil(2)` on OpenBSD.
- Support Windows 10/11.

### License

This project is licensed under the GPLv3 license, with the exception of
`utf8.c`, which comes from [`termbox`](https://github.com/nsf/termbox) and
is licensed under the MIT license.
