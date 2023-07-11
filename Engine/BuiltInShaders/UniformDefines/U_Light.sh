// Include to:
// Light.sh
// Light.h

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2
#define SPHERE_LIGHT 3
#define DISK_LIGHT 4
#define RECTANGLE_LIGHT 5
#define TUBE_LIGHT 6

#define LIGHT_LENGTH 320

struct U_Light {
	// vec4 * 5
	float type;
	vec3 position;           // 1
	float intensity;
	vec3 color;              // 2
	float range;
	vec3 direction;          // 3
	float radius;
	vec3 up;                 // 4
	float width;
	float height;
	float lightAngleScale;
	float lightAngleOffeset; // 5
};