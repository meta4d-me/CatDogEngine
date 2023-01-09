uniform vec4 u_params[8];

#define u_mtx0          u_params[0]
#define u_mtx1          u_params[1]
#define u_mtx2          u_params[2]
#define u_mtx3          u_params[3]

#define u_cameraPos     u_params[4].xyz
#define u_EV100         u_params[4].w

#define u_doDirDiffuse  u_params[5].x
#define u_doDirSpecular u_params[5].y
#define u_doEnvDiffuse  u_params[5].z
#define u_doEnvSpecular u_params[5].w

#if defined(DEBUG_RENDERING)
	#define u_doWF u_params[6].x
	#define u_doSH u_params[6].y
#endif
