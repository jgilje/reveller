The sidplayer subproject builds the dummy binary with both MinGW and MSVC compiler tools.

Make sure http://www.cmake.org/ is installed and availble in your path.

Open a shell, and CD into your build folder. Run the appropriate command
for your toolset.

```Batchfile
cmake -G "Visual Studio 12" ..\..\..\sidplayer
msbuild sidplayer-dummy.vcxproj
```

```Batchfile
cmake -G "MinGW Makefiles" ..\..\..\sidplayer
mingw32-make sidplayer-dummy
```

MinGW can easily be obtained via the online installer at http://qt-project.org/downloads.

Visual Studio Express 2013 for Windows Desktop at http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx contains the required toolset for MSVC.
