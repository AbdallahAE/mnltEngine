#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec3 sunLightPosition;
  vec4 sunLightColor;
  vec4 ambientLightColor; // w is intensity
  vec3 pointLightPosition;
  vec4 pointLightColor;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 directionToPointLight = ubo.pointLightPosition - fragPosWorld;
  float attenuation = 1.0 / dot(directionToPointLight, directionToPointLight); // distance squared
  
  vec3 sunLightColor = ubo.sunLightColor.xyz * ubo.sunLightColor.w;
  vec3 sunLight = sunLightColor * max(dot(normalize(fragNormalWorld), normalize(ubo.sunLightPosition)), 0);

  vec3 pointLightColor = ubo.pointLightColor.xyz * ubo.pointLightColor.w * attenuation;
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 diffuseLight = pointLightColor * max(dot(normalize(fragNormalWorld), normalize(directionToPointLight)), 0);

  outColor = vec4((diffuseLight + ambientLight + sunLight) * fragColor, 1.0);
}