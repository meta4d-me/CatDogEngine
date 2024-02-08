// Light types
#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2
#define SPHERE_LIGHT 3
#define DISK_LIGHT 4
#define RECTANGLE_LIGHT 5
#define TUBE_LIGHT 6

#define LIGHT_LENGTH 21	
#define LIGHT_TRANSFORM_LENGTH 12 

/*
LIGHT_LENGTH = num of lights(3) * num of total vec4 in one light(7)
LIGHT_TRANSFORM_LENGTH  = num of lights(3) * max num of total mat4 of light transform for different kind of light(4)
*/
struct U_Light {
	// vec4 * 7
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
	int shadowType; 
	int lightViewProjOffset;
	int cascadeNum;
	float shadowBias;		 // 6
	vec4 frustumClips;		 // 7
};
