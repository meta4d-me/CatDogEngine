// @brief Returns Camera object.
// 
// Camera GetCamera();

struct Camera
{
	vec3 position;
	// TODO : Preparing for PBC, camera need more members and uniforms.
};

uniform vec4 u_cameraPos;

Camera GetCamera() {
	Camera camera;
	camera.position = u_cameraPos.xyz;
	
	return camera;
}
