call premake_clean.bat
call premake_templates.bat
chdir ..\..\..
tools\win\bin\premake4.exe --os=windows --platform=x32 --no-glsl-optimizer --directX vs2010
cd tools\win\scripts
timeout /T 30