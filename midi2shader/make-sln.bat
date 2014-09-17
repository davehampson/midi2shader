@pushd %~d0
@setlocal

if "%VS90COMNTOOLS%" == "" goto :no2008
..\tools\premake4 vs2008
goto :end
:no2008

if "%VS100COMNTOOLS%" == "" goto :end
..\tools\premake4 vs2010
:end

@endlocal
@popd
