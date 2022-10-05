set SHADER="shader"
set DX11="../../runtime/shaders/dx11"

shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_PBR_skybox.sc -o %SHADER%/vs_PBR_skybox.bin
shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PBR_skybox.sc -o %SHADER%/fs_PBR_skybox.bin

shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_fullscreen.sc -o %SHADER%/vs_fullscreen.bin
shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PBR_postProcessing.sc -o %SHADER%/fs_PBR_postProcessing.bin

pause
