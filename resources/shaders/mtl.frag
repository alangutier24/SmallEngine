#version 410 core

in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3  uKd;
uniform vec3  uKa;
uniform vec3  uKe;
uniform float ud;
uniform int   uIllum;

uniform vec3 uIa;

void main() {

    vec3 base = uKd;

    vec3 color = base + uKe;

    if (uIllum >= 1) {
        vec3 amb = uKa;
        color += amb * uIa;
    }

    FragColor = vec4(color, ud);
    
}