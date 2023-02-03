precision mediump float;
in vec2 outUV;
in vec4 outRgba;
in mat4 outUser;
out vec4 outColor;
uniform sampler2D diffuseMap;
vec4 GetRadialBlurredColor(sampler2D texture, vec2 uv_coord,
                           vec2 center_uv_coord, float blur_length,
                           int blur_sample_count) {
  if (blur_length == 0.0)
    return texture2D(texture, uv_coord);
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
    sum_color += texture2D(texture, fetch_uv_coord).rgb * weight;
    sum_weight += weight;
  }

  return vec4(sum_color / sum_weight, 1.0);
}
void main() {
  vec4 color =
      outRgba * GetRadialBlurredColor(diffuseMap, outUV, vec2(0.5, 0.5),
                                      outUser[0][0], 10);
  outColor = color;
}
