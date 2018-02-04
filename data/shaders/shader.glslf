#version 150

in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;

out vec4 outColor;

uniform bool hasMaterial;
uniform sampler2D material;

uniform vec3 overrideColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

void main() {
  float amt = clamp(dot(Normal, lightDirection), 0, 1);
  outColor = vec4(overrideColor * lightColor * amt, 1.0);

  if (hasMaterial)
    outColor *= texture(material, Texcoord);
}
