#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormals_in;
layout (location = 2) in vec2 texCoords_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelTransform;
uniform mat4 projectionTransform;
uniform mat3 normalTransform;
uniform mat4 viewTransform;

uniform float waveTime;
uniform float amplitude[4];
uniform float frequency[4];
uniform float phase[4];
// Specify the output of the vertex stage
out vec2 uvCoords;
out vec3 vertNormal;
out vec3 vertCoord;

float waveHeight(int waveIdx, float uvalue)
{
 return amplitude[waveIdx]*sin(2 * M_PI * (frequency[waveIdx] * uvalue + phase[waveIdx] + waveTime));
}

float waveDU(int waveIdx, float uvalue)
{
  return amplitude[waveIdx]*cos(2 * M_PI * (frequency[waveIdx] * uvalue + phase[waveIdx] + waveTime)) * 2 * M_PI * frequency[waveIdx];
}

void main()
{
    uvCoords = texCoords_in;
    vec3 coords = vertCoordinates_in;
    float derivative = 0;

    for (int i = 0; i < 4; ++i)
    {
      coords.z += waveHeight(i, uvCoords.x);
      derivative += waveDU(i, uvCoords.x);
    }

    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(coords, 1.0);
    vertCoord = vec3(modelTransform * vec4(coords, 1.0));
    // vertNormal  = normalTransform * vertNormals_in;
    vertNormal = normalize(vec3(-derivative, 0.0, 1.0));
}
