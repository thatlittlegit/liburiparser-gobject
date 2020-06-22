# liburiparser-gobject

`liburiparser-gobject` is a
[GObject](https://developer.gnome.org/gobject/stable/) wrapper over the
[liburiparser](https://uriparser.github.io) library. It should eventually
support everything that liburiparser can do, but in an object-oriented GObject
manner.

It is primarily intended for use in Vala. In C, for the most part you can just
use liburiparser directly (although this will work too!); in other languages,
you should probably just use a parser native to your language.

## License
This code is licensed under the Lesser GNU General Public License, version 3 or
higher.

The documentation *in* the code, and the files in the `docs/` directory, are
licensed under the GNU Free Documentation License, version 1.3. There are
neither front- nor back-cover texts, and no invariant texts.
