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



layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

// Grid position are in clipped space
vec3 gridPlane[18] = vec3[] (
    // Horizontal plane on XZ
    vec3(1, 0, 1), vec3(-1, 0, -1), vec3(-1, 0, 1),
    vec3(-1, 0, -1), vec3(1, 0, 1), vec3(1, 0, -1),
    // Vertical plane on YZ
    vec3(0, 1, 1), vec3(0, -1, -1), vec3(0, -1, 1),
    vec3(0, -1, -1), vec3(0, 1, 1), vec3(0, 1, -1),
    // Vertical plane on XY
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);


vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, ubo.view, ubo.projection).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, ubo.view, ubo.projection).xyz; // unprojecting on the far plane
    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
}

