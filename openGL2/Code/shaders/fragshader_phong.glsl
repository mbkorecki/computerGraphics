#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the inputs to the fragment shader
// These must have the same type and name!
in vec3 vertNormal;
in vec3 vertCoordinates;
in vec3 texCoordinates;

uniform vec3 lightPosition;
uniform vec4 material;
uniform sampler2D texColor;

// const vec3 ambientColor = vec3(0.0, 0.0, 0.1);
// const vec3 diffuseColor = vec3(0.0, 0.0, 0.5);
// const vec3 specColor = vec3(1.0, 1.0, 1.0);
// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

void main()
{
    vec3 lightVector = normalize(lightPosition - vertCoordinates);

    vec3 viewDir = normalize(-vertCoordinates);
    vec3 reflectDir = reflect(-lightVector, vertNormal);

    float specAngle = max(dot(reflectDir, viewDir), 0.01);
    float specular = pow(specAngle, material.w);

    float diffuse = max(dot(lightVector, vertNormal), 0.01);

    vec4 textureColor = texture(texColor, vec2(texCoordinates.x, texCoordinates.y));

    vec3 color = material.x + material.y * vec3(0.95, 0.95, 0.95) * diffuse + material.z * specular;
    fColor = vec4(color, 1.0) * textureColor;
}
