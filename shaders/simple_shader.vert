#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in int layerIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUv;
layout(location = 4) flat out int fragLayerIndex;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};
struct DirectionalLight 
{
  vec4 position;  // ignore w
  vec4 color;     // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  DirectionalLight directionToLight;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform GameObjectBufferData {
  mat4 modelMatrix;
  mat4 normalMatrix;
} gameObject;

layout(push_constant) uniform Push {
  vec3 color;
} push;

void main() {
  vec4 positionWorld = gameObject.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;
  fragNormalWorld = normalize(mat3(gameObject.normalMatrix) * normal);
  fragPosWorld = positionWorld.xyz;
  fragColor = color * push.color;
  fragUv = uv;
  fragLayerIndex = layerIndex;
}
