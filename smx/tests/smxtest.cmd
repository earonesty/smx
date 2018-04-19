@echo off

pushd tests 2> nul

if "%1"=="" goto :loop
goto :test

:loop
for %%c in (test-*.smx) DO call smxtest.cmd %%c

goto :cleanup

:test
echo "%1"

if EXIST "%1.skip"  ] goto :skip 

..\smx %1 > %1.tmp
diff -b %1.ok %1.tmp
if errorlevel 1 goto :error

goto :end

:cleanup
REM del /Q *.tmp

popd

goto :end

:error
echo "ERROR during test '%1'"

:skip
echo "skipping"

:end
