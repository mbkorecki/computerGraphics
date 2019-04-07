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

// Specify the output of the vertex stage
out vec3 vertNormal;
out vec3 vertCoordinates;
out vec3 texCoordinates;

void main()
{
    gl_Position = projectionTransform * modelTransform * vec4(vertCoordinates_in, 1.0);
    vec4 vertPos = modelTransform * vec4(vertCoordinates_in, 1.0);
    vertNormal = normalize(normalTransform * vertNormal_in);
    vertCoordinates = vec3(vertPos) / vertPos.w;
    texCoordinates = texCoordinates_in;
}
