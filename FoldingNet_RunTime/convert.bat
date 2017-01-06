@echo off
call setEnv
set ipath=%DATA_PATH%
set opath=%BASE_PATH%ply\
@echo %ipath%
@echo %opath%
for /f "delims=\" %%i in ('dir /b "%ipath%box_*.txt"') do (
	@echo %%i
	@echo %opath%
	@echo %ipath%%%i
	call %BIN_PATH%ExpFold.exe -m c -o %opath% -f %ipath%%%i
)
pause