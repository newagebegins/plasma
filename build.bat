@echo off
if not exist build mkdir build
pushd build
cl /MTd /nologo /fp:fast /Gm- /GR- /EHa- /Zo /Od /Oi /WX /W4 /FC /Z7 /wd4189 /wd4505 /wd4201 /wd4100 ..\plasma.cpp /link /INCREMENTAL:NO /OPT:REF user32.lib gdi32.lib
popd
