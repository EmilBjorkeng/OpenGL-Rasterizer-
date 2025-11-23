#version 330 core

#define MAX_TEXTURES 16
#define MAX_LIGHTS 16

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
flat in int TexID;
flat in vec3 DiffuseColor;
flat in float Opacity;

uniform sampler2D textures[MAX_TEXTURES];

uniform bool useLighting;
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];

uniform float ambientLight;
uniform vec3 ambientLightColor;

out vec4 FragColor;

void main()
{
    vec3 color = DiffuseColor;
    float alpha = Opacity;

    // Texture
    if (TexID >= 0) {
        vec4 texColor = texture(textures[TexID], TexCoord);
        color *= texColor.rgb;
        alpha *= texColor.a;
    }

    // Light
    vec3 diffuse = vec3(0.0);
    if (useLighting) {
        vec3 norm = normalize(Normal);

        for (int i = 0; i < numLights; ++i) {
            vec3 lightDir = normalize(lightPositions[i] - FragPos);
            float ndotl = max(dot(norm, lightDir), 0.0);

            // Distance attenuation
            float distance = length(lightPositions[i] - FragPos);
            if (distance < 1e-6) distance = 1e-6;
            float attenuation = 1.0 / (distance * distance);

            diffuse += color * lightColors[i] * lightIntensities[i] * ndotl * attenuation;
        }
    }

    vec3 ambient = color * ambientLightColor * ambientLight;
    vec3 finalColor = diffuse + ambient;

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), Opacity);
}