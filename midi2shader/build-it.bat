@pushd %~d0
@setlocal

call make-sln.bat

if "%VS90COMNTOOLS%" == "" goto :no2008
call %VS90COMNTOOLS%\vsvars32.bat
VCExpress MyApplication.sln /build
goto :end
:no2008

if "%VS100COMNTOOLS%" == "" goto :end
call %VS100COMNTOOLS%\vsvars32.bat
devenv MyApplication.sln /build
:end

@endlocal
@popd
