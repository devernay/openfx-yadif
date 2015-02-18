openfx-yadif [![Build Status](https://api.travis-ci.org/devernay/openfx-yadif.png?branch=master)](https://travis-ci.org/devernay/openfx) [![Coverage Status](https://coveralls.io/repos/devernay/openfx-yadif/badge.png?branch=master)](https://coveralls.io/r/devernay/openfx?branch=master) [![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/devernay/openfx-yadif/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
============

Important note
------------

This plugin is now part of the [openfx-misc](http://github.com/devernay/openfx-misc) set of plugin, where it is called "Deinterlace". openfx-misc is actively maintained and contains more than 60 plugins.

Description
---------

A port of Yadif (yet another deinterlacing filter) to OpenFX (for use with Sony Vegas, Nuke, Scratch, etc.)

http://avisynth.org.ru/yadif/yadif.html
http://mplayerhq.hu

Original port to OFX/Vegas by George Yohng http://yohng.com  (2011/02/07)
http://www.yohng.com/software/yadifvegas.html

Modifications by Frederic Devernay

All supplemental code is public domain, and
Yadif algorithm code part itself is licensed GPL 
(as the original yadif plugin)

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

### Unix/Linux/FreeBSD/OS X, using Makefiles

To compile an optimized version for a 64-bits machine: open a shell in
the toplevel directory, and type

	make DEBUGFLAG=-O3 BITS=64

Without the DEBUGFLAG flag, a debug version will be compiled, and use
BITS=32 to compile a 32-bits version.

The compiled plugins will be placed in subdiecories named after the
configuration, for example Linux-64-realease for a 64-bits Linux
compilation. In each of these directories, you will find a `*.bundle`
file, which has to be moved to the proper place (`/usr/OFX/Plugins`on
Linus, or `/Library/OFX/Plugins`on OS X), using a command like:
	sudo mv */*/*.bundle /usr/OFX/Plugins

### OS X, using Xcode

The plugins may be compiled by compiling the Xcode project called
`Misc.xcodeproj` in the toplevel directory. The bundles produced by
this project have to be moved to `/Library/OFX/Plugins`.

Alternatively, you can compile from the command-line using:

	xcodebuild -configuration Release install
	sudo mkdir /Library/OFX/Plugins
	sudo mv /tmp/yadif.dst/Library/OFX/Plugins/Misc /Library/OFX/Plugins

### MS Windows, using Visual Studio

Instructions for compilation and installation on MS Windows are in the readme-mswindows.txt file
