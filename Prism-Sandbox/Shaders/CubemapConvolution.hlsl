#define CUBEMAP_RESOLUTION 1024

SamplerState g_samPointWrap : register(s0);
SamplerState g_samPointClamp : register(s1);
SamplerState g_samLinearWrap : register(s2);
SamplerState g_samLinearClamp : register(s3);
SamplerState g_samAnisotropicWrap : register(s4);
SamplerState g_samAnisotropicClamp : register(s5);
SamplerComparisonState g_samShadow : register(s6);

TextureCube g_skybox : register(t0);
RWTexture2DArray<float4> g_convSkybox : register(u0);

static const uint NumSamples = 1024 * 4;
static const float InvNumSamples = 1.f / float(NumSamples);

static const float PI = 3.14159265359f;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

float3 RotateVectorAroundAxis(float3 vec, float3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return
		float3((oc * axis.x * axis.x + c) * vec.x + (oc * axis.x * axis.y - axis.z * s) * vec.y + (oc * axis.z * axis.x + axis.y * s) * vec.z,
			   (oc * axis.x * axis.y + axis.z * s) * vec.x + (oc * axis.y * axis.y + c) * vec.y + (oc * axis.y * axis.z - axis.x * s) * vec.z,
			   (oc * axis.z * axis.x - axis.y * s) * vec.x + (oc * axis.y * axis.z + axis.x * s) * vec.y + (oc * axis.z * axis.z + c) * vec.z);
}

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i)
{
	return float2(i * InvNumSamples, RadicalInverse_VdC(i));
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
float3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0, 1.0 - u1 * u1));
	return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

float3 GetSamplingVector(int3 threadID)
{
	float x = ((float) threadID.x / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;
	float y = ((float) threadID.y / (CUBEMAP_RESOLUTION - 1.f)) * 2.f - 1.f;

	float3 direction;
	if (threadID.z == 0)
		direction = float3(1.f, y, -x);
	else if (threadID.z == 1)
		direction = float3(-1.f, y, x);
	else if (threadID.z == 2)
		direction = float3(x, -1.f, y);
	else if (threadID.z == 3)
		direction = float3(x, 1.f, -y);
	else if (threadID.z == 4)
		direction = float3(x, y, 1.f);
	else if (threadID.z == 5)
		direction = float3(-x, y, -1.f);
	
	direction = normalize(direction);
	return direction;
}

void ComputeBasisVectors(const float3 N, out float3 B, out float3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	B = normalize(cross(N, T));
}

// Convert point from tangent/shading space to world space.
float3 TangentToWorld(const float3 v, const float3 N, const float3 B, const float3 T)
{
	return B * v.x + T * v.y + N * v.z;
}

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID,
		  int3 dispatchThreadID : SV_DispatchThreadID)
{
	float3 N = -GetSamplingVector(dispatchThreadID);
	float3 B, T;
	ComputeBasisVectors(N, B, T);
	
	float3 irradiance = 0.f;
	for (uint i = 0; i < NumSamples; ++i)
	{
		float2 u = SampleHammersley(i);
		float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, B, T);
		float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division by pdf.
		irradiance += 2.f * g_skybox.SampleLevel(g_samLinearClamp, Li, 0).rgb * cosTheta;
	}
	irradiance /= (float)NumSamples;
	
	g_convSkybox[dispatchThreadID] = float4(irradiance, 1.f);
}
