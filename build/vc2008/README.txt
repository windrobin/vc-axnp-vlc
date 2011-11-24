How to build the Webplugins under Visual Studio 2008:
-----------------------------------------------------

1) Download "msinttypes" package from
http://code.google.com/p/msinttypes/
and extract to a folder.

2) Download "Gecko SDK" (at least 8.0) from
https://developer.mozilla.org/en/Gecko_SDK
and extract to a folder.

3) Download "vlc" and extract/install to a folder.

4) Download libvlc.lib from
http://code.google.com/p/vc-libvlc/downloads/list
and extract to a folder.

5) In VS, add path to "msinttypes", "Gecko SDK" include(xulrunner-sdk\include)
and vlc SDK include (sdk\include) to VC Include directories
(Tools > Options > Projects and Solutions > VC++ Directories > Include files).

6) In VS, add path to libvlc.lib to VC Library directories
(Tools > Options > Projects and Solutions > VC++ Directories > Library files).

7)* Optional, if you wish .rc files generated from .rc.in files.
Prepare MinGW/MSys build enveronment as described in
http://wiki.videolan.org/Win32CompileMSYSNew,
then run "autogen.sh" and "configure".

8) Open build\vc2008\axnp.sln in Visual Studio and enjoy.
