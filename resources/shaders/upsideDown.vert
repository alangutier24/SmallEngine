#version 330

uniform mat4 vTransform;
uniform mat4 MVP;

in vec3 vCol;
in vec2 uv;
in vec3 vPos;

out vec3 color;
out vec2 TexCoord;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
    TexCoord = uv;
}