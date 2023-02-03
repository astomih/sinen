precision mediump float;
uniform sampler2D diffuseMap;
in vec2 outUV;
in vec4 outRgba;
in vec3 fragNormal;
in vec3 fragWorldPos;
out vec4 outColor;

void main() {
  vec3 uCameraPos = vec3(0.0);
  vec3 mDirection = vec3(0.0, -0.25, -0.25);
  vec3 mDiffuseColor = vec3(1.0, 1.0, 1.0);
  vec3 mSpecColor = vec3(1.0);
  float uSpecPower = 100.0;
  vec3 uAmbientLight = vec3(0.5);
  vec3 N = normalize(fragNormal);
  vec3 L = normalize(-mDirection);
  vec3 V = normalize(uCameraPos - fragWorldPos);
  vec3 R = normalize(reflect(-L, N));

  vec3 Phong = uAmbientLight;
  float NdotL = dot(N, L);

  if (NdotL > 0.0) {
    vec3 Diffuse = mDiffuseColor * NdotL;
    Phong += Diffuse;
  }
  vec4 color = vec4(Phong, 1.0) * outRgba * texture(diffuseMap, outUV);
  outColor = color;
}
