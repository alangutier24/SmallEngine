#version 410 core
out vec4 FragColor;

void main() {
    vec3 background_color = vec3(0.5, 0.7, 1.0);
    float sigma_a = 0.1;
    float dist = 10.0;
    float T = exp(-dist * sigma_a);
    vec3 background_color_through_volume = T * background_color;

    FragColor = vec4(background_color_through_volume, 1.0);
}