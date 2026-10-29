#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 vrayOrigin;
out vec3 vLocalPos;
out vec3 rayOrigin;

void main() {
    vLocalPos = aPos;    
    rayOrigin = vrayOrigin;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}