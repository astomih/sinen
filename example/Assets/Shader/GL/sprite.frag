#version 330 core
precision mediump float;
in vec2 outUV;
// Normal (in world space)
in vec3 fragNormal;
// Position (in world space)
in vec3 fragWorldPos;
out vec4 outColor;
uniform sampler2D diffuseMap;
void main()
{
	vec3 uCameraPos = vec3(0,0,0);
	// Direction of light
	vec3 mDirection = vec3(-0.75,-0.75,0.75);
	// Diffuse color
	vec3 mDiffuseColor = vec3(1,1,1);
	// Specular color
	vec3 mSpecColor = vec3(1,1,1);
	// Specular power for this surface
	float uSpecPower = 100;
	// Ambient light level
	vec3 uAmbientLight = vec3(0.5,0.5,0.5);
	// Surface normal
	vec3 N = normalize(fragNormal);
	// Vector from surface to light
	vec3 L = normalize(-mDirection);
	// Vector from surface to camera
	vec3 V = normalize(uCameraPos - fragWorldPos);
	// Reflection of -L about N
	vec3 R = normalize(reflect(-L, N));

	// Compute phong reflection
	vec3 Phong = uAmbientLight;
	float NdotL = dot(N, L);
	if (NdotL > 0)
	{

		vec3 Diffuse = mDiffuseColor * NdotL;
		vec3 Specular = mSpecColor * pow(max(0.0, dot(R, V)), uSpecPower);
		Phong += Diffuse + Specular;
	}

	vec4 color = vec4(Phong, 1.0)*texture(diffuseMap,outUV);
	if(color.a < 0.5)
	{ 
		discard;
	}
	outColor = color;
}