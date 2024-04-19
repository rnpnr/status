status
======

A status line program for dwm.

Inspired by and drawing from
[dsblocks](https://github.com/ashish-yadav11/dsblocks) and
[slstatus](https://tools.suckless.org/slstatus/).

Installation
------------

Copy `config.def.h` to `config.h` and modify to your liking then to build:

```
./build.sh
```

Then copy `status` to wherever you want.

Usage
-----

Execute with `status` or use `setsid -f status` to run in the
background. `status` also accepts the `-d` option to print the
line to `stdout` instead of setting the X root window name.

Gotchas
-------

* alsalib is horribly broken. Don't compile with optimizations more
  exotic than `-O2` and expect alsa to work. If you aren't using alsa
  then you can use whatever meme flags you want.
