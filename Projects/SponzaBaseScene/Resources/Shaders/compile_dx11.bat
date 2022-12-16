set SHADER="shader"

rem Precompute
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeTransmittance.sc -o cs_ComputeTransmittance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeDirectIrradiance.sc -o cs_ComputeDirectIrradiance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeSingleScattering.sc -o cs_ComputeSingleScattering.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeScatteringDensity.sc -o cs_ComputeScatteringDensity.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeIndirectIrradiance.sc -o cs_ComputeIndirectIrradiance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/cs_ComputeMultipleScattering.sc -o cs_ComputeMultipleScattering.bin

rem AtmosphericScattering
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_atmSkyBox.sc -o vs_atmSkyBox.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PrecomputedAtmosphericScattering_LUT.sc -o fs_PrecomputedAtmosphericScattering_LUT.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_SingleScattering_RayMarching.sc -o fs_SingleScattering_RayMarching.bin

rem SkyRenderer
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_PBR_skybox.sc -o vs_PBR_skybox.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PBR_skybox.sc -o fs_PBR_skybox.bin

rem SceneRenderer
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_PBR.sc -o vs_PBR.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PBR.sc -o fs_PBR.bin --define USE_LIGHT

rem TerrainRenderer
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_terrain.sc -o vs_terrain.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_terrain.sc -o fs_terrain.bin

rem Post
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_fullscreen.sc -o vs_fullscreen.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_PBR_postProcessing.sc -o fs_PBR_postProcessing.bin

rem ImGui
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/vs_imgui.sc -o vs_imgui.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADER%/varying.def.sc -f %SHADER%/fs_imgui.sc -o fs_imgui.bin

echo Finish compiling...

pause
