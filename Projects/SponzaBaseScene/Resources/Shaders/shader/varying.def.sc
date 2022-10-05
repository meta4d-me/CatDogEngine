vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_worldPos  : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3 v_skyboxDir : TEXCOORD2 = vec3(0.0, 0.0, 0.0);
vec3 v_bc        : TEXCOORD3 = vec3(0.0, 0.0, 0.0);
vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
mat3 v_TBN       : TEXCOORD4;

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec3 a_tangent   : TANGENT;
vec2 a_texcoord0 : TEXCOORD0;
vec3 a_color1    : COLOR1;
