@echo off
call setEnv
set ipath=%BASE_PATH%ply\
set opath=%BASE_PATH%res_ply\
@echo %ipath%
@echo %opath%
call %BIN_PATH%ExpFold.exe -m w -o %opath% -i %ipath%