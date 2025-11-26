#version 430 core

out vec4 FragColour;

in vec2 texCoords;

uniform sampler2D stateTexture;

void main()
{
    FragColour = vec4(texture(stateTexture, texCoords));
}
