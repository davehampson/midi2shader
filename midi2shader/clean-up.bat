pushd %~dp0
del *.idb *.ncb *.pdb *.ilk *.suo *.vcproj.*.user *.vcproj.user *.vcxproj.filters *.vcxproj.user *.sdf
rd /s /q obj
popd
