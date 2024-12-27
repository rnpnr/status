status
======

A status line program for dwm.

Overview
--------
The status line is made up of individual "blocks". Blocks update
based on a timer interval or an inotify CLOSE_WRITE notification.
Each block has an `_init()` function which is called once at startup
which facilitates the creation and initialization of any static
data and any needed registration with inotify. Afterwords a blocks
'_update()' function is called on every program tick with a `dt`
value and is expected to return with a true or false value based
on if they updated or not. Blocks being called from the inotify
event dispatcher are passed the special value `dt = 0`. This way
blocks are free to determine for themselves when they should update.

Installation
------------

Copy `config.def.h` to `config.h` and modify to your liking then to build:

```
./build.sh
```

Then copy `status` to wherever you want.

Usage
-----

Run `status` to launch in the background. Alternatively, launch to
the foreground with `status -d`. When running in the foreground
`status` prints to `stdout` instead of setting the X root window
name.

Gotchas
-------

* alsalib is horribly broken. Don't compile with optimizations more
  exotic than `-O2` and expect alsa to work. If you aren't using alsa
  then you can use whatever meme flags you want.
