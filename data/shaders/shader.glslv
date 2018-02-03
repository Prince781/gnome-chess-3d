#version 150

in vec3 position;
in vec2 texcoord;
in vec3 normal;

out vec3 Position;
out vec2 Texcoord;
out vec3 Normal;

// transforms a model position to a position in the world
uniform mat4 model;
// transforms the world according to camera movement
uniform mat4 view;
// projection transform (for perspective)
uniform mat4 proj;

void main() {
	Position = position;
	Texcoord = texcoord;
	Normal = normal;

	gl_Position = proj * view * model * vec4(position, 1.0);
}
