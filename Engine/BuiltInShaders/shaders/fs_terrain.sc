$input v_worldPos, v_normal, v_texcoord0, v_alphaMapTexCoord

#include "../common/common.sh"

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texAlphaMap, 2);
SAMPLER2D(s_texRedTexture, 3);
SAMPLER2D(s_texGreenTexture, 4);
SAMPLER2D(s_texBlueTexture, 5);
SAMPLER2D(s_texAlphaTexture, 6);

void main()
{
    vec4 blendRatio = texture2D(s_texAlphaMap, v_alphaMapTexCoord);
    vec4 redChannelColor = texture2D(s_texRedTexture, v_alphaMapTexCoord)*blendRatio.r;
    vec4 greenChannelColor = texture2D(s_texGreenTexture, v_alphaMapTexCoord)*blendRatio.g;
    vec4 blueChannelColor = texture2D(s_texGreenTexture, v_alphaMapTexCoord)*blendRatio.b;
    vec4 alphaChannelColor = texture2D(s_texAlphaTexture, v_alphaMapTexCoord)*blendRatio.a;
	gl_FragColor = vec4((redChannelColor.xyz + greenChannelColor.xyz + blueChannelColor.xyz + alphaChannelColor.xyz), 1.0);
    // gl_FragColor = vec4(texture2D(s_texBaseColor, v_alphaMapTexCoord).xyz, 1.0);
}
