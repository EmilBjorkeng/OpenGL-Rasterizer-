#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in int aTexID;
layout(location = 4) in vec3 aDiffuseColor;
layout(location = 5) in float aOpacity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
flat out int TexID;
flat out vec3 DiffuseColor;
flat out float Opacity;

void main()
{
    // Transform the vertex into clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Passing attributes to the fragment shader
    TexCoord = aTexCoord; // Rasteriser will interpolate the UV
    TexID = aTexID;
    DiffuseColor = aDiffuseColor;
    Opacity = aOpacity;
}