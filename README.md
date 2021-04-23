status
======

A status line program for dwm.

Inspired by and drawing from
[dsblocks](https://github.com/ashish-yadav11/dsblocks) and
[slstatus](https://tools.suckless.org/slstatus/).

Installation
------------

Modify `config.h` and `config.mk` to your liking and use the following
to install (using root as needed):

```
make clean install
```

Usage
-----

Execute with `status` or use `status &` to run in the background. `status`
also accepts the `-d` option to print the line to stdout instead of
setting the x root window name.

Gotchas
-------

* alsalib is horribly broken. Don't compile with optimizations more
  exotic than `-O2` and expect alsa to work. If you aren't using alsa
  then you can use whatever meme flags you want.

* If you want a smaller binary remove blocks you aren't using from
  `config.mk`. You can also remove their linker options. Additionally,
  consider using a better compiler like [tcc](https://bellard.org/tcc/).
