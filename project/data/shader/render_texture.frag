#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 5) in mat4 inUser;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D diffuseMap;

vec4 GetRadialBlurredColor(sampler2D t, vec2 uv_coord, vec2 center_uv_coord,
                           float blur_length, int blur_sample_count) {
  if (blur_length == 0.0) {
    return texture(t, uv_coord);
  }
  vec2 vec_to_center = uv_coord - center_uv_coord;
  float distance =
      sqrt(uv_coord.x * uv_coord.x + center_uv_coord.y * center_uv_coord.y);
  vec2 vec_to_target = (vec_to_center / distance) * blur_length;

  vec3 sum_color = vec3(0.0, 0.0, 0.0);
  float sum_weight = 0.0;
  float mult = 1.0 / float(blur_sample_count);
  for (int i = 0; i <= blur_sample_count; ++i) {
    float ratio = float(i) * mult;
    vec2 fetch_uv_coord = uv_coord - vec_to_target * ratio;
    float weight = 1.0 - ratio * ratio;
    sum_color += texture(t, fetch_uv_coord).rgb * weight;
    sum_weight += weight;
  }

  return vec4(sum_color / sum_weight, 1.0);
}

void main() {
  vec4 color = inColor * GetRadialBlurredColor(diffuseMap, inUV, vec2(0.5, 0.5),
                                               inUser[0][0], 10);
  outColor = color;
}