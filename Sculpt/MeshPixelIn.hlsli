
struct Instance {
	float3 inst_pos : INSTANCE_POSITION;
	float4 inst_rot : INSTANCE_ROTATION;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
	float3 wireframe_tess_front_color : WIREFRAME_TESS_FRONT_COLOR;
	float4 wireframe_tess_back_color : WIREFRAME_TESS_BACK_COLOR;
	float wireframe_tess_split_count : WIREFRAME_TESS_SPLIT_COUNT;
	float wireframe_tess_gap : WIREFRAME_TESS_GAP;
};

struct PixelIn {
	float4 pos : SV_POSITION;
	float3 world_pos : WORLD_POS;
	float3 vertex_normal : VERTEX_NORMAL;
	
	nointerpolation uint vertex_id : VERTEX_ID;
	nointerpolation uint primitive_id : PRIMITIVE_ID;
	nointerpolation uint instance_id : INSTANCE_ID;
	
	float tess_edge : TESS_EDGE;
	float tess_edge_dir : TESS_EDGE_DIRECTION;
};


static const float PI = 3.14159265f;


// Physical Based Rendering /////////////////////////////////////////////////////////////

struct MeshTriangle {
	float3 poly_normal;
	float3 tess_normal;
	uint tess_vertex_0;
	uint tess_vertex_1;
};

struct CameraLight {
	float3 normal;
	float3 color;
	float intensity;
};

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
	float k = (r * r) / 8.0;

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

float3 calcPhysicalBasedRendering(float3 surface_normal, float3 camera_backward, CameraLight lights[8],
	float roughness, float metallic, float specular,
	float3 albedo_color, float ambient_intensity)
{	
	float3 N    = surface_normal;
	float3 V    = camera_backward;
	float NdotV = max(dot(N, V), 0.);

	// Fresnel F0
	float3 F0 = specular;
	F0 = lerp(F0, albedo_color, metallic);
	
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
		Lo += (kD * albedo_color / PI + specular_BRDF) * radiance * NdotL;
	}
	
	float3 ambient = ambient_intensity * albedo_color;
	float3 color = Lo + ambient;
	
	return color;
}


// 1 Dimension Processing ///////////////////////////////////////////////////////////////////////////////

float calcStripe(float gradient, float split_count, float gap_size)
{
	return floor(frac(gradient * split_count) + gap_size);
}
