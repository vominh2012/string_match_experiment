@echo off

ctime -begin string_match.ctm

set BUILD_FLAGS=/W3 /Od /Z7 /wd4996 /nologo /link
cl string_match.cpp %BUILD_FLAGS%
set LastError=%ERRORLEVEL%

ctime -end string_match.ctm %LastError%