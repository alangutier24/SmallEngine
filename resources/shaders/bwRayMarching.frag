#version 410 core

out vec4 FragColor;

in vec3 rayOrigin;
in vec3 vLocalPos;

bool intersectBox(vec3 orig, vec3 dir, out float tNear, out float tFar) {
    vec3 ta = (-0.5 - orig) / dir;
    vec3 tb = (0.5- orig) / dir;
    vec3 t1 = min(ta, tb);
    vec3 t2 = max(ta, tb);
    tNear = max(t1.x, max(t1.y, t1.z));
    tFar  = min(t2.x, min(t2.y, t2.z));
    return tFar > 0.0 && tNear < tFar;
}

void main()
{

    float tNear;
    float tFar;

    vec3 ray_dir = normalize(vLocalPos - rayOrigin);

    intersectBox(rayOrigin, ray_dir, tNear, tFar);

    float step_size = 0.2;
    float sigma_a = 0.1; // absorption coefficient
    float ns = ceil((tFar - tNear) / step_size);
    step_size = (tFar - tNear) / ns;

    vec3 light_dir = vec3( 0, 1, 0 );
    vec3 light_color = vec3( 1.3, 0.3, 0.9 );

    float transparency = 1; 
    vec3 result = vec3( 0 ); 

    for (int n = 0; n < ns; ++n) {
        float t = tNear - step_size * (n + 0.5);
        vec3 sample_pos= rayOrigin + t * ray_dir;

        float sample_attenuation = exp(-step_size * sigma_a);
        
        transparency *= sample_attenuation;

        float light_attenuation = exp(-tFar * sigma_a);
        result += light_color * light_attenuation * step_size;

        result *= sample_attenuation;
    }

    FragColor = vec4(vec3(0.2f, 0.3f, 0.3f)* transparency + result, 1.0f);
        
}