#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

out vec2 tc;

uniform float scaling_factor, v1, v2, theta;

mat3 scaling = mat3(
    scaling_factor, 0.0f, 0.0f,  
    0.0f, scaling_factor, 0.0f, 
    0.0f, 0.0f, 1.0f
);

mat3 translation = mat3(
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    v1, v2, 1.0f
);

mat3 rotation = mat3(
    cos(theta), -sin(theta), 0.0f,
    sin(theta), cos(theta), 0.0f,
    0.0f, 0.0f, 1.0f
);

void main() {
	tc = texCoord;
    vec3 positions = translation * (rotation * (scaling * pos));
	gl_Position = vec4(positions, 1.0);
}
