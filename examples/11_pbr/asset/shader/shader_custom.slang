struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float3 pos : TEXCOORD1;
  float3 normal : NORMAL;
  float4 color : COLOR;
};

#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name //: register(c##bind, space1)
#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name //: register(c##bind, space1)

UNIFORM_VERT(Mat, 0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 user;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(mul(proj, mul(view, world)), float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  output.normal = normalize(mul((float3x3)world, input.normal));
  output.pos = mul(world, float4(input.position, 1.0)).xyz;
  return output;
};

struct FSInput {
  float2 uv : TEXCOORD0;
  float3 pos : TEXCOORD1;
  float3 normal : NORMAL;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : COLOR;
};
#define COMBINED_IMAGE_SAMPLER(name, bind)                                     \
  [[vk::binding(bind, 2)]]                                                     \
  Sampler2D name; //: register(t##bind, space##set) : register(s##bind,
                  //: space##set);

COMBINED_IMAGE_SAMPLER(albedoMap, 0)
COMBINED_IMAGE_SAMPLER(normalMap, 1)
COMBINED_IMAGE_SAMPLER(diffuseRoughnessMap,
                       2) // in DamagedHelmet.glb, the
                          // roughness is in the G channel
                          // and metallic is in the B channel
COMBINED_IMAGE_SAMPLER(metallicMap, 3)
COMBINED_IMAGE_SAMPLER(emissiveMap, 4)
COMBINED_IMAGE_SAMPLER(aoMap, 5)

// BRDF
float3 SchlickFresnel(float hDotV, float3 refl) {
  return refl + (1.0 - refl) * pow(1.0 - hDotV, 5.0);
}
#define PI 3.14159265358979323846
float ggxDistribution(float nDotH, float roughness) {
  float a = roughness * roughness * roughness * roughness;
  float d = nDotH * nDotH * (a - 1.0) + 1.0;
  d = PI * d * d;
  return (a / max(d, 0.0000001));
}

float geomSmith(float nDotV, float nDotL, float roughness) {
  float r = roughness + 1.0;
  float k = r * r / 8.0;
  float ik = 1.0 - k;
  float ggx1 = nDotV / (nDotV * ik + k);
  float ggx2 = nDotL / (nDotL * ik + k);
  return ggx1 * ggx2;
}

struct Light {
  float3 position;
  float3 color;
  float intensity;
};

UNIFORM_FRAG(Positions, 1) {
  float3 camPos;
  float3 lightPos;
}
float3 computePBR(float3 pos, float3 normal, float2 uv, Light lights[],
                  int lightCount = 1) {
  float3 albedo = albedoMap.Sample(uv).rgb;
  albedo = float3(albedo.x, albedo.y, albedo.z);

  float metallic = diffuseRoughnessMap.Sample(uv).b;
  float roughness = diffuseRoughnessMap.Sample(uv).g;

  float ao = clamp(aoMap.Sample(uv).r, 0.0, 1.0);

  float3 N = normal;
  // float3 N = normalMap.Sample(uv).xyz;
  float3 V = normalize(camPos - pos);
  float3 emissive = emissiveMap.Sample(uv).rgb;

  float3 baseRefl = lerp(float3(0.04), albedo.rgb, metallic);
  float3 lightAccum = float3(0.0);

  for (int i = 0; i < lightCount; i++) {
    float3 L = normalize(lights[i].position - pos);
    float3 H = normalize(V + L);
    float dist = length(lights[i].position - pos);
    float attenuation = 1.0 / (dist * dist * 0.23);
    float3 radiance = lights[i].color.rgb * lights[i].intensity * attenuation;

    // Cook-Torrance BRDF distribution function
    float nDotV = max(dot(N, V), 0.0000001);
    float nDotL = max(dot(N, L), 0.0000001);
    float hDotV = max(dot(H, V), 0.0);
    float nDotH = max(dot(N, H), 0.0);
    float D = ggxDistribution(nDotH, roughness);
    float G = geomSmith(nDotV, nDotL, roughness);
    float3 F = SchlickFresnel(hDotV, baseRefl);

    float3 spec = (D * G * F) / (4.0 * nDotV * nDotL);

    float3 kD = float3(1.0) - F;

    kD *= 1.0 - metallic;
    lightAccum += ((kD * albedo.rgb / PI + spec) * radiance * nDotL);
  }

  float3 ambient = albedo.rgb * 0.4 * ao;

  return (ambient + lightAccum + emissive);
}

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  Light light[1];

  light[0].position = lightPos;
  light[0].color = float3(1, 1, 1);
  light[0].intensity = 50.0;

  output.color =
      float4(computePBR(input.pos, input.normal, input.uv, light), 1.0);

  return output;
}

