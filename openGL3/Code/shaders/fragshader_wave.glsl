#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the inputs to the fragment shader
// These must have the same type and name!
in vec2 uvCoords;
in vec3 vertNormal;
in vec3 vertCoord;
// Specify the Uniforms of the fragment shaders

// uniform vec3 lightPosition; // for example
// Lighting model constants.
uniform vec4 material;
uniform vec3 lightColour;
uniform vec3 lightPosition;
uniform vec3 waveColor;
uniform sampler2D textureSampler;

// Texture sampler
// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

void main()
{
    // Ambient colour does not depend on any vectors.
    vec3 ambientIntensity    = material.x * waveColor;
    // Calculate light direction vectors in the phong model.
    vec3 lightDirection   = normalize(lightPosition - vertCoord);

    // Diffuse colour.
    vec3 diffuseIntensity = material.y * waveColor * lightColour * max(dot(vertNormal, lightDirection), 0);


    // Specular colour.
    vec3 viewDirection     = normalize(-vertCoord); // The camera is always at (0, 0, 0).
    vec3 reflectDirection  = reflect(-lightDirection, vertNormal);
    vec3 specularIntesity = lightColour * waveColor * material.z *  pow(max(dot(reflectDirection, viewDirection), 0), material.w);


    float smoothedVal = smoothstep(-2.0f, 2.0f, vertCoord.y);
    vec4 heightColor = vec4(0.0f, 1.0f, mix(1.0f, 0.7f, smoothedVal), 1.0f);

    fColor = vec4(ambientIntensity + diffuseIntensity + specularIntesity, 1.0) * heightColor;
}
