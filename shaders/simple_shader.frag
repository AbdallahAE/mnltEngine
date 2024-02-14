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
  mat4 invView;
  DirectionalLight directionLight;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  vec3 color;
} push;

void main() {  
  vec3 sunLightColor = ubo.directionLight.color.xyz * ubo.directionLight.color.w;
  vec3 sunLight = sunLightColor * max(dot(normalize(fragNormalWorld), normalize(ubo.directionLight.position.xyz)), 0);
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 diffuseLight = {0.0f, 0.0f, 0.0f};
  vec3 specularLight = vec3(0.0);

  vec3 cameraPosWorld = ubo.invView[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(normalize(fragNormalWorld), directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;

    // specular lighting
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(normalize(fragNormalWorld), halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
    specularLight += intensity * blinnTerm;
  }

  {
    vec3 directionToLight = normalize(ubo.directionLight.position.xyz);
    float cosAngIncidence = max(dot(normalize(fragNormalWorld), directionToLight), 0);
    sunLight += sunLightColor * cosAngIncidence;
    // specular lighting
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(normalize(fragNormalWorld), halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
    specularLight += sunLightColor * blinnTerm;
  }

  outColor = vec4(((diffuseLight + ambientLight + sunLight) * fragColor) + (specularLight * fragColor), 1.0);
}