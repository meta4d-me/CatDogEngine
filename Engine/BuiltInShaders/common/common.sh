#include "bgfx_shader.sh"
#include "shaderlib.sh"

#define CD_PI 3.1415926536
#define CD_PI2 9.8696044011
#define CD_PI_INV 0.3183098862
#define CD_PI_INV2 0.1013211836

// https://baddogzz.github.io/2020/03/04/SGA-Pow-Opt/
float pow_low(float x, float n)
{
	// 1.4427f --> 1/ln(2)
	n = n * 1.4427 + 1.4427;
	return exp2(x * n - n);
}
