openfx-yadif [![Build Status](https://api.travis-ci.org/devernay/openfx-yadif.png?branch=master)](https://travis-ci.org/devernay/openfx) [![Coverity Scan Build Status](https://scan.coverity.com/projects/5245/badge.svg)](https://scan.coverity.com/projects/5245 "Coverity Badge")
============

Important note
------------

This plugin is now part of the [openfx-misc](http://github.com/devernay/openfx-misc) set of plugin, where it is called "Deinterlace".  [openfx-misc](http://github.com/devernay/openfx-misc) is actively maintained, contains more than 60 plugins, and works on the same OpenFX hosts as openfx-yadif (including Nuke and DaVinci Resolve).

Description
---------

A port of Yadif (yet another deinterlacing filter) to OpenFX (for use with Sony Vegas, Nuke, Scratch, DaVinci Resolve, etc.)

http://avisynth.org.ru/yadif/yadif.html
http://mplayerhq.hu

Original port to OFX/Vegas by George Yohng http://yohng.com  (2011/02/07)
http://www.yohng.com/software/yadifvegas.html

Modifications by Frederic Devernay.

All supplemental code is public domain, and Yadif algorithm code part itself is licensed LGPL (as the original yadif plugin).

See the comments in this source.

The original precompiled MS Windows binary is available from http://www.yohng.com/software/yadifvegas.html

Installation
------------

If you want to compile the plugins from source, you may either use the
provided Unix Makefile, the Xcode project, or the Visual Studio project.

### Getting the sources from github

To fetch the latest sources from github, execute the following commands:

	git clone https://github.com/devernay/openfx-yadif.git
	cd openfx-yadif
	git submodule update -i -r

In order to get a specific tag, corresponding to a source release, do `git tag -l`
to get the list of tags, and then `git checkout tags/<tag_name>`
to checkout a given tag.

### Compiling (Unix/Linux/FreeBSD/OS X, using Makefiles)

On Unix-like systems, the plugins can be compiled by typing in a
terminal:
- `make [options]` to compile as a single combined plugin (see below
  for valid options).
- `make nomulti [options]` to compile as separate plugins (useful if
only a few plugins are is needed, for example). `make` can also be
executed in any plugin's directory.

The most common options are `CONFIG=release` to compile a release
version, `CONFIG=debug` to compile a debug version. Or
`CONFIG=relwithdebinfo` to compile an optimized version with debugging
symbols.

Another common option is `BITS=32`for compiling a 32-bits version,
`BITS=64` for a 64-bits version, and `BITS=Universal` for a universal
binary (OS X only).

See the file `Makefile.master`in the toplevel directory for other useful
flags/variables.

The compiled plugins are placed in subdirectories named after the
configuration, for example Linux-64-realease for a 64-bits Linux
compilation. In each of these directories, a `*.bundle` directory is
created, which has to be moved to the proper place
(`/usr/OFX/Plugins`on Linux, or `/Library/OFX/Plugins`on OS X), using
a command like the following, with the *same* options used for
compiling:

	sudo make install [options]

### OS X, using Xcode

The latest version of Xcode should be installed in order to compile this plugin.

Open the "Terminal" application (use spotlight, or browse `/Applications/Utilities`), and paste the following lines one-by-one (an administrator password will be asked for after the second line):

	xcodebuild -configuration Release install
	sudo mkdir /Library/OFX/Plugins
	sudo mv /tmp/yadif.dst/Library/OFX/Plugins/yadif/yadif.ofx.bundle /Library/OFX/Plugins

The plugins may also be compiled by compiling the Xcode project called
`yadif.xcodeproj` in the toplevel directory. The bundles produced by
this project have to be moved to `/Library/OFX/Plugins`.

### MS Windows, using Visual Studio

Instructions for compilation and installation on MS Windows are in the readme-mswindows.txt file
