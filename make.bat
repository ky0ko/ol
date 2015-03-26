@echo off
set PATH=%PATH%;C:\MinGW\bin;C:\MinGW\msys\1.0\bin

if exist boot.fasl move /Y boot.fasl boot.fasl.bak

:: ������� ������������� (� ��������������� ���������������� ������� ������� �� ������)
echo echo | set /p test=Compiling ol virtual machine... 
gcc src/olvm.c -DSTANDALONE src/boot.c -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -O3 -std=c11
if ERRORLEVEL 1 exit
echo Ok.

:: � ������ �������� ���������� ������ ������
echo.
echo Compiling new oL system image:
a.exe src/ol.scm
if ERRORLEVEL 1 exit
if not exist boot.fasl exit

:: ������������ ����������� ������
echo.
call :TEST-ALL

:: ������������� ��� � C � ������� ��� ������
echo.
echo echo | set /p test=Preparing new interpreter... 
a.exe src/to-c.scm >bootstrap
gcc src/olvm.c -x c bootstrap -DSTANDALONE -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -O3 -std=c11 -g0 -o ol.exe
if not ERRORLEVEL 0 exit
echo (display "Ok!") |ol
exit

::recompile
::echo Making ol.exe:
::gcc src/olvm.c -x c image src/repl.c -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -Ofast -o ol.exe
::if not ERRORLEVEL 0 exit
::ol.exe -e "(display \"Ok\")"


:: � ����������� ������ ������
::gcc src/olvm.c -DSTANDALONE -DNOLANGUAGE -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -O3 -std=c11 -o vm.exe
::vm ok.bin



:: ������ ����� �� ������ �����������
::ol.exe src/ol.scm
::echo Preparing new boot.c...
::ol.exe src/to-c.scm >boot2.c
::echo Comparing old and new boot files
::fc boot.c boot2.c /B >NUL
::if errorlevel 1 (
::	echo Different
::	copy boot2.c boot.c
::	goto recompile
::) ELSE (
::	echo ok
::)

::echo Making new repl2.exe...
::gcc src/olvm.c boot2.c src/repl.c -IC:\MinGW\include\ -LC:\MinGW\lib\ -lws2_32 -Ofast -o repl2.exe
::repl2.exe -e "(print \"Ok\")"

:TEST
	echo | set /p test=Testing tests/%1... 
::	a.exe tests/%1 >txt
	a.exe boot.fasl <tests/%1 >txt
	copy tests\%1.ok ok >NUL
	fc /L txt ok >NUL
	if ERRORLEVEL 1 goto :failed
	echo Ok.
GOTO:EOF

:TEST-ALL
	setlocal EnableDelayedExpansion
	for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
		set "DEL=%%a"
	)
	<nul > X set /p ".=."

	echo Testing tests:
for %%f in (
	apply.scm
	banana.scm
	bisect-rand.scm
	callcc.scm
	case-lambda.scm
	echo.scm
	ellipsis.scm
	eval.scm
	factor-rand.scm
	factorial.scm
	fasl.scm
	ff-call.scm
	ff-del-rand.scm
	ff-rand.scm
	fib-rand.scm
	hashbang.scm
	iff-rand.scm
	library.scm
	macro-capture.scm
	macro-lambda.scm
	mail-order.scm
	math-rand.scm
	par-nested.scm
	par-nested-rand.scm
	par-rand.scm
	perm-rand.scm
	por-prime-rand.scm
	por-terminate.scm
	queue-rand.scm
	r5rs.scm
	r7rs.scm
	record.scm
	rlist-rand.scm
	seven.scm
	share.scm
	stable-rand.scm
	str-quote.scm
	string.scm
	suffix-rand.scm
	theorem-rand.scm
	toplevel-persist.scm
	utf-8-rand.scm
	vararg.scm
	vector-rand.scm
	numbers.scm
) do call :TEST %%f
::	circle.scm
::	dir.scm
::	opengl.scm
::	bingo-rand.scm // failed, no "i has all" output
::	circular.scm (��������� �� ������ ���� � stderr � �� �� ��������)
::	file.scm    (vec-len: not a vector: #false of type 13)
::	mail-async-rand.scm // failed, no "ok 300" message
::	process.scm (error)
::	regex.scm (hangs)
GOTO:EOF

:color
	set "param=^%~2" !
	set "param=!param:"=\"!"
	findstr /p /A:%1 "." "!param!\..\X" nul
	<nul set /p ".=%DEL%%DEL%%DEL%%DEL%%DEL%%DEL%%DEL%"
	exit /b

:failed
	call :color 0c "Failed."
	echo(
	goto :eof