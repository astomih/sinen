precision mediump float;
uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
in vec2 outUV;
in vec4 outRgba;
// Normal (in world space)
in vec3 fragNormal;
// Position (in world space)
in vec3 fragWorldPos;
in vec4 ShadowCoord;
out vec4 outColor;

/*
float simple_shadow( vec3 proj_pos ) {
 float shadow_distance = max( ( texture( shadowMap, proj_pos.xy).r ), 0.0 );
 float distance = proj_pos.z-0.005;
 if( shadow_distance < distance ) return 0.5;
 return 1.0;
 }
 */

void main()
{
	vec3 uCameraPos = vec3(0);
	vec3 mDirection = vec3(0.0,-0.25,-0.25);
	vec3 mDiffuseColor = vec3(1.0,1.0,1.0);
	vec3 mSpecColor = vec3(1.0);
	float uSpecPower = 100.0;
	vec3 uAmbientLight = vec3(0.2);
	vec3 N = normalize(fragNormal);
	vec3 L = normalize(-mDirection);
	vec3 V = normalize(uCameraPos - fragWorldPos);
	vec3 R = normalize(reflect(-L, N));


	vec3 Phong = uAmbientLight;
	float NdotL = dot(N, L);
	
	//float visibility = simple_shadow(ShadowCoord.xyz);
	if (NdotL > 0.0)
	{
		vec3 Diffuse = mDiffuseColor * NdotL;//*visibility;
		Phong += Diffuse;
	}
	vec4 color = vec4(Phong, 1.0)*outRgba*texture(diffuseMap, outUV);
	outColor = color;
}
