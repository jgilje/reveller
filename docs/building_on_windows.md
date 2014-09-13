The sidplayer subproject builds the dummy binary with both mingw32 and
msvc compiler tools.

Make sure http://www.cmake.org/ is installed and availble in your path.

Open a shell, and CD into your build folder. Run the appropriate command
for your toolset.

```Batchfile
cmake -G "Visual Studio 12" ..\..\..\sidplayer
msbuild sidplayer-dummy.vcxproj
```

```Batchfile
cmake -G "MinGW Makefiles" ..\..\..\sidplayer
mingw32-make
```
