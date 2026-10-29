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

#define PI 3.14159265359

float phase(float g, float cosTheta)
{
    float denom = 1.0 + g * g - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * (1.0 - g * g) / (denom * sqrt(denom));
}

float ign(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

void main()
{

    float tNear;
    float tFar;

    vec3 ray_dir = normalize(vLocalPos - rayOrigin);

    intersectBox(rayOrigin, ray_dir, tNear, tFar);

    float step_size = 0.2;
    float sigma_a = 0.1; // absorption coefficient
    float sigma_s = 0.1;
    float ns = ceil((tFar - tNear) / step_size);
    step_size = (tFar - tNear) / ns;

    vec3 light_dir = vec3( 0, 1, 0 );
    vec3 light_color = vec3( 1.3, 0.3, 0.9 );

    float density = 0.2;
    float g = 0.25;

    float transparency = 1; 
    vec3 result = vec3( 0 ); 

    float jitter = ign(gl_FragCoord.xy); 

    for (int n = 0; n < ns; ++n) {
        float t = tNear + step_size * (float(n) + jitter);
        vec3 sample_pos= rayOrigin + t * ray_dir;

        float sample_attenuation = exp(-step_size * sigma_a);
        
        transparency *= sample_attenuation;

        float cosTheta = dot(ray_dir, light_dir);
        float transmittance = exp(-density * step_size * (sigma_a + sigma_s));
        float lightAtten = exp(-density * tFar * (sigma_a + sigma_s));
        result += transmittance * density * sigma_s * phase(g, cosTheta)
                * lightAtten * light_color * step_size;

        result *= sample_attenuation;
    }

    FragColor = vec4(vec3(0.2f, 0.3f, 0.3f)* transparency + result, 1.0f);
        
}