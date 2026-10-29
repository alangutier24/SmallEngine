#version 410 core

out vec4 FragColor;

in vec3 rayOrigin;   // camara en espacio local del cubo
in vec3 vLocalPos;   // posicion del fragmento en espacio local

uniform sampler3D uDensity;   // R32F, densidad ya bakeada en CPU

#define PI 3.14159265359

// ---------------------------------------------------------------- geometria

bool intersectBox(vec3 orig, vec3 dir, out float tNear, out float tFar) {
    vec3 ta = (vec3(-0.5) - orig) / dir;
    vec3 tb = (vec3( 0.5) - orig) / dir;
    vec3 t1 = min(ta, tb);
    vec3 t2 = max(ta, tb);
    tNear = max(t1.x, max(t1.y, t1.z));
    tFar  = min(t2.x, min(t2.y, t2.z));
    return tFar > 0.0 && tNear < tFar;
}

// ---------------------------------------------------------------- scattering

float phase(float g, float cosTheta) {
    float denom = 1.0 + g * g - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * (1.0 - g * g) / (denom * sqrt(denom));
}

float ign(vec2 q) {
    return fract(52.9829189 * fract(dot(q, vec2(0.06711056, 0.00583715))));
}

// ---------------------------------------------------------------- densidad

float density(vec3 pos) {
    return texture(uDensity, pos + 0.5).r;   // local [-0.5,0.5] -> uvw [0,1]
}

// ---------------------------------------------------------------- main

const int LIGHT_STEPS = 8;

void main()
{
    vec3 ray_dir = normalize(vLocalPos - rayOrigin);

    float tNear, tFar;
    if (!intersectBox(rayOrigin, ray_dir, tNear, tFar)) discard;
    tNear = max(tNear, 0.0);   // la camara puede estar dentro del cubo

    float sigma_a = 5.0;
    float sigma_s = 5.0;
    float sigma_t = sigma_a + sigma_s;

    float step_size = 0.03;
    int   ns = max(1, int(ceil((tFar - tNear) / step_size)));
    step_size = (tFar - tNear) / float(ns);

    vec3 light_dir   = normalize(vec3(0.0, 1.0, 0.0));
    vec3 light_color = vec3(20.0);

    float g  = 0.8;
    float ph = phase(g, dot(ray_dir, light_dir));

    float transmission = 1.0;
    vec3  result       = vec3(0.0);

    float jitter = ign(gl_FragCoord.xy);

    for (int n = 0; n < ns; ++n) {
        float t   = tNear + step_size * (float(n) + jitter);
        vec3  pos = rayOrigin + ray_dir * t;

        float d = density(pos);
        if (d <= 0.0) continue;

        float lNear, lFar;
        float lightAtten = 1.0;
        if (intersectBox(pos, light_dir, lNear, lFar)) {
            float l_step = lFar / float(LIGHT_STEPS);
            float tau    = 0.0;
            for (int m = 0; m < LIGHT_STEPS; ++m) {
                vec3 lpos = pos + light_dir * l_step * (float(m) + 0.5);
                tau += density(lpos) * l_step;
            }
            lightAtten = exp(-tau * sigma_t);
        }

        result += transmission * d * sigma_s * ph
                * lightAtten * light_color * step_size;

        transmission *= exp(-d * sigma_t * step_size);
    }

    vec3 background = vec3(0.2, 0.3, 0.3);
    FragColor = vec4(background * transmission + result, 1.0);
}