#include "../../common.slang"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

UNIFORM_VERT(Mat, 0) { float4x4 wvp; }

UNIFORM_FRAG(ShaderToyData, 1) {
  float4 iResolutionTimeMode;
  float4 iMousePulse;
}

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 0, 1);

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(wvp, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
}

struct FSInput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target;
};

static float hash21(float2 p) {
  p = frac(p * float2(123.34, 456.21));
  p += dot(p, p + 45.32);
  return frac(p.x * p.y);
}

static float noise(float2 p) {
  float2 i = floor(p);
  float2 f = frac(p);
  float2 u = f * f * (3.0 - 2.0 * f);

  float a = hash21(i);
  float b = hash21(i + float2(1.0, 0.0));
  float c = hash21(i + float2(0.0, 1.0));
  float d = hash21(i + float2(1.0, 1.0));

  return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

static float fbm(float2 p) {
  float v = 0.0;
  float a = 0.5;
  for (int i = 0; i < 5; ++i) {
    v += noise(p) * a;
    p = float2(p.x * 1.7 - p.y * 1.1, p.x * 1.1 + p.y * 1.7);
    a *= 0.5;
  }
  return v;
}

static float3 palette(float t) {
  float3 a = float3(0.45, 0.50, 0.56);
  float3 b = float3(0.46, 0.34, 0.31);
  float3 c = float3(1.00, 1.00, 1.00);
  float3 d = float3(0.04, 0.18, 0.34);
  return a + b * cos(6.28318 * (c * t + d));
}

static float tunnel(float2 p, float t, float pulse) {
  float a = atan2(p.y, p.x);
  float r = length(p);
  float rings = sin(18.0 * r - t * 3.0 + sin(a * 6.0 + t) * 0.9);
  float rays = sin(a * 9.0 + t * 2.1 + 1.8 / max(r, 0.08));
  return smoothstep(0.18, 0.95, rings * 0.55 + rays * 0.45 + pulse * 0.7);
}

static float orb(float2 p, float2 at, float radius) {
  return 1.0 - smoothstep(radius * 0.25, radius, length(p - at));
}

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;

  float2 resolution = max(iResolutionTimeMode.xy, float2(1.0, 1.0));
  float time = iResolutionTimeMode.z;
  float mode = iResolutionTimeMode.w;
  float2 mouse = iMousePulse.xy / resolution;
  float mouseDown = iMousePulse.z;
  float pulse = iMousePulse.w;

  float2 fragCoord = float2(input.uv.x, 1.0 - input.uv.y) * resolution;
  float2 p = (fragCoord * 2.0 - resolution) / min(resolution.x, resolution.y);
  float2 m = (mouse * 2.0 - 1.0) * float2(resolution.x / resolution.y, 1.0);

  float3 color = float3(0.0, 0.0, 0.0);

  if (mode < 1.5) {
    float2 q = p;
    q.x += sin(q.y * 3.0 + time * 0.8) * 0.12;
    q.y += cos(q.x * 2.5 - time * 0.7) * 0.10;

    float f = fbm(q * 2.1 + time * 0.17);
    float g = fbm(q * 4.0 - time * 0.11 + f);
    float beam = tunnel(q - m * 0.22, time, pulse);
    color = palette(f + g * 0.45 + time * 0.04);
    color *= 0.28 + g * 0.9 + beam * 0.8;
  } else if (mode < 2.5) {
    float d = length(p - m * 0.3);
    float a = atan2(p.y - m.y * 0.3, p.x - m.x * 0.3);
    float spiral = sin(log(max(d, 0.001)) * 7.5 - a * 5.0 - time * 2.3);
    float veins = fbm(float2(a * 1.8, d * 8.0 - time * 0.7));
    float edge = smoothstep(0.75, 0.05, d);
    color = palette(0.56 + spiral * 0.08 + veins * 0.18 + time * 0.03);
    color *= edge * (0.6 + spiral * 0.35 + veins * 0.55);
  } else {
    float2 grid = p * 4.0;
    float cells = fbm(grid + float2(time * 0.18, -time * 0.12));
    float scan = sin((p.y + cells * 0.3) * 80.0 - time * 7.0) * 0.5 + 0.5;
    float star = pow(hash21(floor((p + 10.0) * 36.0)), 24.0) * 2.5;
    float flare = orb(p, m * 0.55, 0.42 + pulse * 0.2);
    color = palette(cells * 0.8 + time * 0.05);
    color *= 0.18 + cells * 0.95 + scan * 0.15;
    color += float3(0.85, 0.96, 1.0) * (star + flare * (0.5 + mouseDown * 0.8));
  }

  float vignette = smoothstep(1.65, 0.25, length(p));
  color *= vignette;
  color *= sampledTexture.Sample(input.uv).rgb;
  color += pulse * float3(0.15, 0.20, 0.26);
  color = pow(saturate(color), float3(0.92, 0.92, 0.92));

  output.color = float4(color, 1.0);
  return output;
}
