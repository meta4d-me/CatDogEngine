echo off
set textures="textures"
set compiled="compiled"

set baseColor="aseColor"
set normal="ormal"
set roughness="oughness"

for %%i in (%textures%/*.*) do (
	echo compiling : %%i
	echo %%i|findstr %baseColor%>nul && (
		start /b texturec -f %textures%/%%i -o %%~ni.dds -t BC3 -m -q f
	)
	echo %%i|findstr %normal%>nul && (
		start /b texturec -f %textures%/%%i -o %%~ni.dds -t BC3 -m -q f -n
	)
	echo %%i|findstr %roughness%>nul && (
		start /b texturec -f %textures%/%%i -o %%~ni.dds -t BC3 -m -q f --linear
	)
)
pause
