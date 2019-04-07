#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormal_in;
layout (location = 2) in vec3 texCoordinates_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelTransform;
uniform mat4 projectionTransform;
uniform mat3 normalTransform;

uniform vec3 lightPosition;
uniform vec4 material;

// Specify the output of the vertex stage
out vec3 vertNormal;
out vec3 texCoordinates;

void main()
{
    gl_Position = projectionTransform * modelTransform * vec4(vertCoordinates_in, 1.0);

    vec3 lightVector = normalize(lightPosition - vertCoordinates_in);

    vec3 viewDir = normalize(-vertCoordinates_in);
    vec3 reflectDir = reflect(-lightVector, vertNormal_in);
    float specAngle = max(dot(reflectDir, viewDir), 0.01);
    float specular = pow(specAngle, material.w);

    float diffuse = max(dot(normalize(normalTransform * vertNormal_in), lightVector), 0.01);

    texCoordinates = texCoordinates_in;
    vertNormal = material.x + material.y * vec3(0.95, 0.95, 0.95) * diffuse + material.z * specular;
}
