// TODO : Preparing for PBC, camera need more members and uniforms.

uniform vec4 u_cameraPos;

struct Camera
{
	vec3 position;
};

Camera GetCamera() {
	Camera camera;
	camera.position = u_cameraPos.xyz;
	
	return camera;
}
