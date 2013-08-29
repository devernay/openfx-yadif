This is a port Yadif (yet another deinterlacing filter)
http://avisynth.org.ru/yadif/yadif.html
http://mplayerhq.hu

Port to OFX/Vegas by George Yohng http://yohng.com  (2011/02/07)

All supplemental code is public domain, and
Yadif algorithm code part itself is licensed GPL 
(as the original yadif plugin)

See the comments in this source.

Instruction for installing:

  Please copy included
    Yadif Deinterlace.ofx.bundle 
         to 
    C:\Program Files\Sony\Vegas Pro 10.0\OFX Video Plug-Ins

  If you have a different Sony Vegas installation path, then copy it 
  accordingly to the inner OFX Video Plug-Ins folder.


Instruction for using:

  1. For the target clip, in the properties set 'Progressive', even
     though it is interlaced, so that Vegas does not deinterlace it.

     Alternatively set Deinterlace method in the project properties to 'None'.

  2. Insert Yadif Deinterlace as Media FX (note, NOT video FX or track FX, but Media FX)
     on the clip.

  3. Setup Yadif Deinterlace parameters to select field order and rendering parity 
     (parity defines which field will be left unchanged and which interpolated).

     Default options are OK in most of cases.


Alternative way:

  If you want to insert it to all interlaced media in the Vegas project 
  at once, then run the script 'Apply Yadif to All Media.cs', which is 
  enclosed in the archive.
  
  To run the script, select Tools -> Scripting -> Run Script... in 
  the Vegas menu bar and navigate to the script file.

  If you insert more media to the project, just re-run the script. It 
  will not touch already processed files.

  The script does automatically the same procedure as described above.


Please note, that due to restrictions of how Vegas works, this plugin cannot be 
used when making smooth slow-motion from interlaced video (stretching the video 2x).

Compilation instructions (for developers):

  1. Place all OFX SDK default and OFX SDK Support includes into
     ../includes folder relative to the plugin source code.

  2. Place OFX SDK Support source files (ofx*) to ../Library
     relative to the plugin source code.

  3. Run nmake -f Makefile32 under 32-bit compiler environment
  4. Run nmake -f Makefile64 under 64-bit compiler environment


George Yohng 
georgey2#oss3d.com
