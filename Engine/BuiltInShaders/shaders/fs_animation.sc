$input v_worldPos, v_boneIDs, v_boneWeights

#include "../common/common.sh"
#include "uniforms.sh"

vec3 evalSh(vec3 dir) {
#	define k01 0.2820947918 // sqrt( 1/PI)/2
#	define k02 0.4886025119 // sqrt( 3/PI)/2
#	define k03 1.0925484306 // sqrt(15/PI)/2
#	define k04 0.3153915652 // sqrt( 5/PI)/4
#	define k05 0.5462742153 // sqrt(15/PI)/4

	vec3 shEnv[9];
	shEnv[0] = vec3( 0.967757057878229854,  0.976516067990363390,  0.891218272348969998); /* Band 0 */
	shEnv[1] = vec3(-0.384163503608655643, -0.423492289131209787, -0.425532726148547868); /* Band 1 */
	shEnv[2] = vec3( 0.055906294587354334,  0.056627436881069373,  0.069969936396987467);
	shEnv[3] = vec3( 0.120985157386215209,  0.119297994074027414,  0.117111965829213599);
	shEnv[4] = vec3(-0.176711633774331106, -0.170331404095516392, -0.151345020570876621); /* Band 2 */
	shEnv[5] = vec3(-0.124682114349692147, -0.119340785411183953, -0.096300354204368860);
	shEnv[6] = vec3( 0.001852378550138503, -0.032592784164597745, -0.088204495001329680);
	shEnv[7] = vec3( 0.296365482782109446,  0.281268696656263029,  0.243328223888495510);
	shEnv[8] = vec3(-0.079826665303240341, -0.109340956251195970, -0.157208859664677764);

	float sh[9];
	sh[0] =  k01;
	sh[1] = -k02 * dir.y;
	sh[2] =  k02 * dir.z;
	sh[3] = -k02 * dir.x;
	sh[4] =  k03 * dir.y * dir.x;
	sh[5] = -k03 * dir.y * dir.z;
	sh[6] =  k04 * (3.0 * dir.z * dir.z -1.0);
	sh[7] = -k03 * dir.x * dir.z;
	sh[8] =  k05 * (dir.x * dir.x - dir.y * dir.y);

	vec3 rgb = vec3_splat(0.0);
	rgb += shEnv[0] * sh[0] * 1.0;
	rgb += shEnv[1] * sh[1] * 2.0 / 2.5;
	rgb += shEnv[2] * sh[2] * 2.0 / 2.5;
	rgb += shEnv[3] * sh[3] * 2.0 / 2.5;
	rgb += shEnv[4] * sh[4] * 1.0 / 2.5;
	rgb += shEnv[5] * sh[5] * 0.5;
	rgb += shEnv[6] * sh[6] * 0.5;
	rgb += shEnv[7] * sh[7] * 0.5;
	rgb += shEnv[8] * sh[8] * 0.5;

	return rgb;
}

void main()
{
	bool foundBoneWeights = false;
	vec4 fragColor = vec4_splat(0.0);
	for(int i = 0; i < 4; ++i)
	{
		if(v_boneIDs[i] == 0)
		{
			if(v_boneWeights[i] >= 0.7)
			{
				fragColor = vec4(1.0, 0.0, 0.0, 0.0) * v_boneWeights[i];
			}
			else if(v_boneWeights[i] >= 0.4 && v_boneWeights[i] <= 0.6)
			{
				fragColor = vec4(0.0, 1.0, 0.0, 0.0) * v_boneWeights[i];
			}
			else if(v_boneWeights[i] >= 0.1)
			{
				fragColor = vec4(0.0, 0.0, 1.0, 0.0) * v_boneWeights[i];
			}
			
			foundBoneWeights = true;
			break;
		}
	}
	
	if(foundBoneWeights)
	{
		// debug bone weights
		gl_FragColor = fragColor;
	}
	else
	{
		// Spherical Harmonics
		vec3 flatNormal = normalize(cross(dFdx(v_worldPos), dFdy(v_worldPos)));
		gl_FragColor = vec4(evalSh(flatNormal), 1.0);
	}
}
