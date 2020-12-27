struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 vertex_normal : VERTEX_NORMAL;
	float3 tess_normal : TESSELATION_NORMAL;
	float3 poly_normal : POLY_NORMAL;
	
	uint shading_mode : SHADING_MODE;
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
};

struct CameraLight {
	float3 normal;
	float3 color;
	float intensity;
};

cbuffer Uniform : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	matrix perspective;
	float z_near;
	float z_far;
	
	CameraLight lights[8];
	float ambient_intensity;
};


static const float PI = 3.14159265f;


float DistributionGGX(float NdotH, float a)
{
	float a2 = a * a;
	
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.) + 1.);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float4 main(VertexInput input) : SV_TARGET
{
	float3 surface_normal;
	switch (input.shading_mode) {
	// Vertex
	case 0:
		surface_normal = input.vertex_normal;
		break;
		
	// Poly
	case 1:
		surface_normal = input.poly_normal;
		break;
		
	// Tesselation
	default:
		surface_normal = input.tess_normal;
		break;
	}
	
	float roughness = input.roughness;
	float metallic  = input.metallic;
	float specular  = input.specular;
	
	float3 N    = surface_normal;
	float3 V    = -camera_forward;
	float NdotV = max(dot(N, V), 0.);

	// Fresnel F0
	float3 F0 = specular;
	F0 = lerp(F0, input.albedo_color, input.metallic);
	
	// reflectance equation
	float3 Lo = 0.;
	
	for(int i = 0; i < 8; ++i) 
	{
		if (lights[i].intensity == 0.) {
			continue;
		}
		
		// calculate per-light radiance
		float3 L = -lights[i].normal;
		float3 H = normalize(V + L);
		
		float NdotH = max(dot(N, H), 0.);
		float NdotL = max(dot(N, L), 0.);
		float HdotV = max(dot(H, V), 0.);

		float3 radiance = lights[i].color * lights[i].intensity;
		
		// cook-torrance brdf
		float NDF = DistributionGGX(NdotH, roughness * roughness);
		float G   = GeometrySmith(NdotV, NdotL, roughness);
		float3 F  = fresnelSchlick(HdotV, F0);
		
		// diffuse blending for metals
		float3 kS = F;
		float3 kD = 1. - kS;
		kD *= 1.0 - metallic;
		
		float3 numerator  = NDF * G * F;
		float denominator = 4. * NdotV * NdotL;
		float3 specular_BRDF = numerator / max(denominator, 0.001);

		// add to outgoing radiance Lo
		Lo += (kD * input.albedo_color / PI + specular_BRDF) * radiance * NdotL;
	}
	
	float3 ambient = ambient_intensity * input.albedo_color;
	float3 color = Lo + ambient;
	
	return float4(color, 1.);
}