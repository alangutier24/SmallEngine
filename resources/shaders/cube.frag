#version 410 core

out vec4 FragColor;

void main() {

    float sigma_a = 0.03; 
    float distance = 10; 
    float T = exp(-distance * sigma_a); 

    FragColor = vec4(0, 0, 0, 1.0 - T);
}