#version 150

in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;

out vec4 outColor;

uniform sampler2D material;

uniform vec3 overrideColor;

void main() {
	vec4 texColor = texture(material, Texcoord);
	outColor = vec4(overrideColor, 1.0); /* * texColor; */
}
