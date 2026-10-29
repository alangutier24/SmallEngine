#version 410 core

out vec4 FragColor;

in vec3 rayOrigin;   // camara en espacio local del cubo
in vec3 vLocalPos;   // posicion del fragmento en espacio local

uniform int p[512];  // tabla de permutacion duplicada: p[i] == p[i+256]

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

// ---------------------------------------------------------------- perlin

float fade(float t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14) ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float perlin(vec3 P) {
    int X = int(floor(P.x)) & 255;
    int Y = int(floor(P.y)) & 255;
    int Z = int(floor(P.z)) & 255;

    vec3 f = P - floor(P);
    float x = f.x, y = f.y, z = f.z;

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A  = p[X]     + Y;
    int AA = p[A]     + Z;
    int AB = p[A + 1] + Z;
    int B  = p[X + 1] + Y;
    int BA = p[B]     + Z;
    int BB = p[B + 1] + Z;

    return mix( mix( mix(grad(p[AA    ], x,       y,       z      ),
                         grad(p[BA    ], x - 1.0, y,       z      ), u),
                     mix(grad(p[AB    ], x,       y - 1.0, z      ),
                         grad(p[BB    ], x - 1.0, y - 1.0, z      ), u), v),
                mix( mix(grad(p[AA + 1], x,       y,       z - 1.0),
                         grad(p[BA + 1], x - 1.0, y,       z - 1.0), u),
                     mix(grad(p[AB + 1], x,       y - 1.0, z - 1.0),
                         grad(p[BB + 1], x - 1.0, y - 1.0, z - 1.0), u), v), w);
}

// ---------------------------------------------------------------- densidad

// El cubo mide 1 unidad de lado, asi que sin escalar la posicion el ruido
// no llega ni a una celda del lattice. NOISE_FREQ es lo que da el detalle.

uniform float uTime;

const float NOISE_FREQ = 4.0;
const int   OCTAVES    = 4;
const float LACUNARITY = 2.0;
const float H          = 0.4;
const float RADIUS     = 0.5;   // media arista de tu cubo

float density(vec3 pos) {
    // pos ya viene en espacio local centrado en el origen -> no hay 'p - center'

    // envolvente: se calcula con la posicion SIN rotar
    float dist    = min(1.0, length(pos) / RADIUS);
    float falloff = smoothstep(0.8, 1.0, dist);

    float theta = uTime * 0.5;
    vec3 q = vec3( cos(theta) * pos.x + sin(theta) * pos.z,
                   pos.y,
                  -sin(theta) * pos.x + cos(theta) * pos.z );

    q *= NOISE_FREQ;
    float fbm = 0.0;
    for (int k = 0; k < OCTAVES; ++k) {
        fbm += perlin(q) * pow(LACUNARITY, -H * float(k));
        q   *= LACUNARITY;
    }

    return max(0.0, fbm) * (1.0 - falloff);
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
    vec3  light_color = vec3(20.0, 20.0, 20.0); 

    float g  = 0.8;
    float ph = phase(g, dot(ray_dir, light_dir));   // constante: fuera del bucle

    float transmission = 1.0;
    vec3  result       = vec3(0.0);

    float jitter = ign(gl_FragCoord.xy);

    for (int n = 0; n < ns; ++n) {
        float t   = tNear + step_size * (float(n) + jitter);
        vec3  pos = rayOrigin + ray_dir * t;

        float d = density(pos);
        if (d <= 0.0) continue;

        // --- rayo secundario hacia la luz: su PROPIO tFar ---
        float lNear, lFar;
        float lightAtten = 1.0;
        if (intersectBox(pos, light_dir, lNear, lFar)) {
            float l_step = lFar / float(LIGHT_STEPS);
            float tau    = 0.0;                       // profundidad optica
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
