openfx-yadif [![Build Status](https://api.travis-ci.org/devernay/openfx-yadif.png?branch=master)](https://travis-ci.org/devernay/openfx) [![Coverage Status](https://coveralls.io/repos/devernay/openfx-yadif/badge.png?branch=master)](https://coveralls.io/r/devernay/openfx?branch=master) [![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/devernay/openfx-yadif/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
============

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

Instructions for compilation and installation on MS Windows are in the readme-mswindows.txt file

To build and install on OSX, type in a terminal:
```
xcodebuild -configuration Release install
sudo mv /tmp/yadif.dst/Library/OFX/Plugins/yadif /Library/OFX/Plugins
```
