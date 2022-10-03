echo off
set textures="textures"
set compiled="compiled"

set baseColor="aseColor"
set normal="ormal"
set roughness="oughness"

for %%i in (%textures%/*.*) do (
	echo compiling : %%i
	echo %%i|findstr %baseColor%>nul && (
		texturec -f %textures%/%%i -o %compiled%/%%~ni.dds -t BC3 -m -q f
	)
	echo %%i|findstr %normal%>nul && (
		texturec -f %textures%/%%i -o %compiled%/%%~ni.dds -t BC3 -m -q f -n
	)
	echo %%i|findstr %roughness%>nul && (
		texturec -f %textures%/%%i -o %compiled%/%%~ni.dds -t BC3 -m -q f --linear
	)
)
pause
