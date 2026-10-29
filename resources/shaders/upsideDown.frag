#version 330

in vec3 color;
in vec2 TexCoord;

out vec4 fragment;

uniform sampler2D ourTexture;

void main()
{
    fragment = texture(ourTexture, TexCoord) * vec4(color, 1.0);  
}