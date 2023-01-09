set SHADERS="shaders"
set PROJECT="../../Projects/SponzaBaseScene/Resources/Shaders"

rem Precompute
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeTransmittance.sc -o %PROJECT%/cs_ComputeTransmittance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeDirectIrradiance.sc -o %PROJECT%/cs_ComputeDirectIrradiance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeSingleScattering.sc -o %PROJECT%/cs_ComputeSingleScattering.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeScatteringDensity.sc -o %PROJECT%/cs_ComputeScatteringDensity.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeIndirectIrradiance.sc -o %PROJECT%/cs_ComputeIndirectIrradiance.bin
start /b shaderc --type c --platform windows -p cs_5_0 -O 1 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/cs_ComputeMultipleScattering.sc -o %PROJECT%/cs_ComputeMultipleScattering.bin

rem Atmospheric Scattering
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_atmSkyBox.sc -o %PROJECT%/vs_atmSkyBox.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_PrecomputedAtmosphericScattering_LUT.sc -o %PROJECT%/fs_PrecomputedAtmosphericScattering_LUT.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_SingleScattering_RayMarching.sc -o %PROJECT%/fs_SingleScattering_RayMarching.bin

rem Cube Map Renderer
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_PBR_skybox.sc -o %PROJECT%/vs_PBR_skybox.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_PBR_skybox.sc -o %PROJECT%/fs_PBR_skybox.bin

rem Scene Renderer
start /b shaderc --raw --type v --platform windows -p vs_5_0 -O 3 -f %SHADERS%/vs_PBR.hlsl -o %PROJECT%/vs_PBR.bin
start /b shaderc --raw --type f --platform windows -p ps_5_0 -O 3 -f %SHADERS%/fs_PBR.hlsl -o %PROJECT%/fs_PBR.bin
::start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_PBR.sc -o %PROJECT%/vs_PBR.bin
::start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_PBR.sc -o %PROJECT%/fs_PBR.bin

rem Terrain Renderer
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_terrain.sc -o %PROJECT%/vs_terrain.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_terrain.sc -o %PROJECT%/fs_terrain.bin

rem Postprocessing
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_fullscreen.sc -o %PROJECT%/vs_fullscreen.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_PBR_postProcessing.sc -o %PROJECT%/fs_PBR_postProcessing.bin

rem ImGui
start /b shaderc --type v --platform windows -p vs_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/vs_imgui.sc -o %PROJECT%/vs_imgui.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_imgui.sc -o %PROJECT%/fs_imgui.bin

rem Error Handling
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_loading_resources.sc -o %PROJECT%/fs_loading_resources.bin
start /b shaderc --type f --platform windows -p ps_5_0 -O 3 --varyingdef %SHADERS%/varying.def.sc -f %SHADERS%/fs_missing_textures.sc -o %PROJECT%/fs_missing_textures.bin

echo Finish compiling...
echo .bin files generated at %PROJECT%

pause
