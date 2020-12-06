struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 vertex_normal : VERTEX_NORMAL;
	float3 tess_normal : TESSELATION_NORMAL;
	float3 poly_normal : POLY_NORMAL;
	
	uint shading_mode : SHADING_MODE;
	float3 diffuse : DIFFUSE;
	float emissive : EMISSIVE;
};

struct CameraLight {
	float3 normal;
	float3 color;
	float intensity;
	float radius;
};

struct LightResult {
	float shading;
};

cbuffer Uniform : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	matrix perspective;
	float z_near;
	float z_far;
	
	CameraLight lights[4];
};

static const float PI = 3.14159265f;

// Trowbridge-Reitz GGX normal distribution function
float normalDistributionFunction(float NdotH, float a)
{
	float a2     = pow(a, 2);
	float NdotH2 = pow(NdotH, 2);
	
	float nom    = a2;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
	denom        = PI * pow(denom, 2);
	
	return nom / denom;
}

float3 fresnelSchlick(float cos_theta, float3 F0)
{
	return F0 + (1. - F0) * pow(1. - cos_theta, 5.);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
	float k = pow(roughness + 1., 2.) / 8.;

	float num   = NdotV;
	float denom = NdotV * (1. - k) + k;
	
	return num / denom;
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
	
	float roughness = 0.5;
	float metallic = 0;
	float specular = 0.04;

	float3 L = -lights[0].normal;  // light vector
	float3 V = -camera_forward;  // view/eye/camera vector
	
	float3 N = surface_normal;  // normal of the surface pointing up
	float3 H = normalize(L + V);  // half vector between light and camera vectors
	
	// Normal Distribution Function
	float NdotH = max(dot(N, H), 0.);
	float D = normalDistributionFunction(NdotH, roughness);
	
	// Fresnel
	// the original source code uses HdotV like Unreal Engine 4 but it didn't seem to make sense
	float LdotN = max(dot(L, N), 0.);
	float3 F;
	{
		float3 F0 = lerp(specular.xxx, input.diffuse, metallic);
		F = fresnelSchlick(LdotN,  F0);
	}
	
	// Geometry
	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);
	float G;
	{
		// GeometrySmith
		float ggxL  = geometrySchlickGGX(NdotL, roughness);
		float ggxV  = geometrySchlickGGX(NdotV, roughness);
		G = ggxL * ggxV; 
	}
	
	// Cook-Torrance Specular BRDF
	float3 num = N * G * F;
	float denom = 4. * max(dot(N, L), 0.) * max(dot(N, V), 0.);
	float3 specular_BRDF = num / max(denom, 0.001);
	
	// Diffuse BRDF
	float3 diffuse_BRDF = input.diffuse / PI;
	
	// kD term
	float3 kS = F;
    float3 kD = float1(1.).xxx - kS;
    kD *= 1.0 - metallic;
	
	float3 Lo = (kD * diffuse_BRDF + specular) * NdotL;
	
	float3 color = Lo;
	color = color / (color + float(1.).xxx);
    color = pow(color, float(1.0 / 2.2).xxx); 
	
	return float4(color, 1);
}