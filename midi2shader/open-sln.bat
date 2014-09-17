@pushd %~d0
@setlocal

call make-sln.bat

if "%VS90COMNTOOLS%" == "" goto :no2008
call %VS90COMNTOOLS%\vsvars32.bat
start VCExpress MyApplication.sln
goto :end
:no2008

if "%VS100COMNTOOLS%" == "" goto :end
call %VS100COMNTOOLS%\vsvars32.bat
start devenv MyApplication.sln
:end

@endlocal
@popd
