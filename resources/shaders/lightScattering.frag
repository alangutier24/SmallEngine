
#version 410 core

out vec4 FragColor;

void main() {

    float sigma_a = 0.05; 
    float distance_travelled_by_light = 10;
    float absorption_coefficient = 0.05; 
    float light_intensity = 10; // just a number, it could be anything 
    float T = exp(-distance_travelled_by_light * absorption_coefficient); 
    float light_intensity_attenuation = T * light_intensity;

    FragColor = vec4(0, 0, 0, 1.0 - light_intensity_attenuation);
}

