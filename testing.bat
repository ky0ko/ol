:: https://en.wikipedia.org/wiki/Regression_testing
erase a.exe
set PATH=%PATH%;C:\MinGW\bin;C:\MinGW\msys\1.0\bin
gcc src/olvm.c src/boot.c src/testing.c -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -Ofast
a.exe 2>NUL