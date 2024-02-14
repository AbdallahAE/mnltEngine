#version 450

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

layout(push_constant) uniform Push {
  float near;
  float far;
  int cordSize;
} push;

float near = push.near;
float far = push.far;
layout(location = 1) in vec3 nearPoint; // nearPoint calculated in vertex shader
layout(location = 2) in vec3 farPoint; // farPoint calculated in vertex shader
layout(location = 0) out vec4 outColor;

vec4 gridy(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.1, 0.1, 0.1, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}
vec4 gridx(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.yz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.1, 0.1, 0.1, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.y > -0.1 * minimumx && fragPos3D.y < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.y = 1.0;
    return color;
}
vec4 gridz(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xy * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.1, 0.1, 0.1, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.y > -0.1 * minimumx && fragPos3D.y < 0.1 * minimumx)
        color.x = 1.0;
    // x axis
    if(fragPos3D.x > -0.1 * minimumz && fragPos3D.x < 0.1 * minimumz)
        color.y = 1.0;
    return color;
}

void main() {
    float y = -nearPoint.y / (farPoint.y - nearPoint.y);
    float x = -nearPoint.x / (farPoint.x - nearPoint.x);
    float z = -nearPoint.z / (farPoint.z - nearPoint.z);

    vec3 fragPosY = nearPoint + y * (farPoint - nearPoint);
    vec3 fragPosX = nearPoint + x * (farPoint - nearPoint);
    vec3 fragPosZ = nearPoint + z * (farPoint - nearPoint);

    gl_FragDepth = 0.9999999;

    outColor = gridy(fragPosY, push.cordSize) * float(y > 0) + gridx(fragPosX, push.cordSize) * float(x > 0) + gridz(fragPosZ, push.cordSize) * float(z > 0);
}