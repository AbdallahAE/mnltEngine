#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

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
  DirectionalLight directionToLight;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {  
  vec3 sunLightColor = ubo.directionToLight.color.xyz * ubo.directionToLight.color.w;
  vec3 sunLight = sunLightColor * max(dot(normalize(fragNormalWorld), normalize(ubo.directionToLight.position.xyz)), 0);
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 diffuseLight = {0.0f, 0.0f, 0.0f};

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    float cosAngIncidence = max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;
    diffuseLight += intensity * cosAngIncidence;
  }

  outColor = vec4((diffuseLight + ambientLight + sunLight) * fragColor, 1.0);
}