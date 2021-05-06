/*__________________________________________________________________________________________________________
[ Sampler ]
____________________________________________________________________________________________________________*/
SamplerState g_samPointWrap			: register(s0);
SamplerState g_samPointClamp		: register(s1);
SamplerState g_samLinearWrap		: register(s2);
SamplerState g_samLinearClamp		: register(s3);
SamplerState g_samAnisotropicWrap	: register(s4);
SamplerState g_samAnisotropicClamp	: register(s5);


/*__________________________________________________________________________________________________________
[ Texture ]
____________________________________________________________________________________________________________*/
Texture2D g_TexDiffuse		: register(t0);	// Diffuse ����.
Texture2D g_TexNormal		: register(t1);	// ź��Ʈ ���� Normal Map.
Texture2D g_TexSpecular		: register(t2);	// Specular ����.
Texture2D g_TexShadowDepth	: register(t3);	// ShadowDepth
Texture2D g_TexDissolve		: register(t4); // Dissolve
/*__________________________________________________________________________________________________________
[ Constant Buffer ]
____________________________________________________________________________________________________________*/
cbuffer cbCamreaMatrix : register(b0)
{
	float4x4	g_matView		: packoffset(c0);
	float4x4	g_matProj		: packoffset(c4);
	float4		g_vCameraPos	: packoffset(c8);
	float		g_fProjFar		: packoffset(c9.x);
	
	float		g_fCMOffset1	: packoffset(c9.y);
	float		g_fCMOffset2	: packoffset(c9.z);
	float		g_fCMOffset3	: packoffset(c9.w);
}
cbuffer cbShaderMesh : register(b1)
{
	float4x4	g_matWorld			: packoffset(c0);
	float4x4	g_matLightView		: packoffset(c4);
	float4x4	g_matLightProj		: packoffset(c8);
	float4		g_vLightPos			: packoffset(c12);
	float		g_fLightPorjFar		: packoffset(c13.x);
    float		g_fDissolve			: packoffset(c13.y);
	float		g_fOffset1			: packoffset(c13.z);
	float		g_fOffset2			: packoffset(c13.w);
    float		g_fOffset3			: packoffset(c14.x);
    float		g_fOffset4			: packoffset(c14.y);
    float		g_fOffset5			: packoffset(c14.z);
    float		g_fOffset6			: packoffset(c14.w);
	float4		g_vAfterImgColor	: packoffset(c15);
	float4		g_vEmissiveColor	: packoffset(c16);
};
struct VS_IN
{
	float3 Pos				: POSITION;
	float3 Normal			: NORMAL;
	float2 TexUV			: TEXCOORD;
};

struct VS_OUT
{
	float4 Pos			: SV_POSITION;
	float2 TexUV		: TEXCOORD0;
	float3 Normal		: TEXCOORD1;
	float3 T			: TEXCOORD2;
	float3 B			: TEXCOORD3;
	float3 N			: TEXCOORD4;
	float4 ProjPos		: TEXCOORD5;
	float4 LightPos		: TEXCOORD6;
    float4 WorldPos		: TEXCOORD7;
    float2 AniUV		: TEXCOORD8;
};

/*__________________________________________________________________________________________________________
[ �׸��� (X) ]
____________________________________________________________________________________________________________*/
VS_OUT VS_MAIN(VS_IN vs_input)
{
    VS_OUT vs_output = (VS_OUT) 0;
  
    float4x4 matworld = g_matWorld;
    //matworld._11 *= g_fOffset1;
    //matworld._22 *= g_fOffset1;
    //matworld._33 *= g_fOffset1;
    float4x4 matWV, matWVP;
    matWV = mul(matworld, g_matView);
    matWVP = mul(matWV, g_matProj);
	
    vs_output.Pos = mul(float4(vs_input.Pos, 1.0f), matWVP);
    vs_output.TexUV = vs_input.TexUV;
    vs_output.Normal = vs_input.Normal;
	
	// N
    float3 WorldNormal = mul(vs_input.Normal, (float3x3) matworld);
   
    vs_output.N = normalize(WorldNormal);
	
	// T
    float3 Tangent = cross(float3(0.f, 1.f, 0.f), (float3) vs_input.Normal);
    float3 WorldTangent = mul(Tangent, (float3x3) matworld);
    vs_output.T = normalize(WorldTangent);
	
	// B
    float3 Binormal = cross((float3) vs_input.Normal, Tangent);
    float3 WorldBinormal = mul(Binormal, (float3x3) matworld);
    vs_output.B = normalize(WorldBinormal);
	
	// ProjPos
    vs_output.ProjPos = vs_output.Pos;
	
    return (vs_output);
}
struct PS_OUT
{
    float4 Effect : SV_TARGET0;
};
PS_OUT PS_MAGIC_CIRCLE(VS_OUT ps_input) : SV_Target
{
    PS_OUT ps_out = (PS_OUT) 0;
    float4 Diffuse = g_TexDiffuse.Sample(g_samLinearWrap, ps_input.TexUV);
    float4 TexNormal = g_TexNormal.Sample(g_samLinearWrap, ps_input.TexUV);
    float4 Spec = g_TexSpecular.Sample(g_samLinearWrap, ps_input.TexUV);
    float4 color = lerp(lerp(Diffuse, Spec, 0.5), TexNormal, 0.5);
    ps_out.Effect = color;
    return ps_out;
}

PS_OUT PS_RAINDROP(VS_OUT ps_input) : SV_TARGET
{
   
    PS_OUT ps_output = (PS_OUT) 0;
	
	// Diffuse
    ps_output.Effect = (g_TexDiffuse.Sample(g_samLinearWrap, ps_input.AniUV * 10.f));
	
    return (ps_output);
}

float4 PS_EFFECT_SHPERE(VS_OUT ps_input) : SV_Target
{
    //float u = (ps_input.TexUV.x / g_vEmissiveColor.x) + g_vEmissiveColor.z * (1.0f / g_vEmissiveColor.x);
    //float v = (ps_input.TexUV.y / g_vEmissiveColor.y) + g_vEmissiveColor.w * (1.0f / g_vEmissiveColor.y);
   // float2 TexUV = float2((u + g_fOffset1*0.002f),( v + g_fOffset1*0.0005f));
   // TexUV.x *= 2.f;
    float2 TexUV = float2(ps_input.TexUV.x, ps_input.TexUV.y);
    clip(ps_input.TexUV.y + g_fOffset1);
    float4 Color = g_TexDiffuse.Sample(g_samLinearWrap, TexUV);
   
 
    float4 Color2 = float4(1, 1, 1, 1);
    return (Color);
}
float4 PS_ICESTORM(VS_OUT ps_input) : SV_Target
{
    float4 D = g_TexDiffuse.Sample(g_samLinearWrap, ps_input.TexUV);
    float4 N = g_TexNormal.Sample(g_samLinearWrap, ps_input.TexUV);
    
    float4 S = g_TexSpecular.Sample(g_samLinearWrap, ps_input.TexUV);
    float4 E = float4(0, 0, 0, 1);
    float Normal_fDissolve = g_TexDissolve.Sample(g_samLinearWrap, ps_input.TexUV).r;

    if ((0.05f > (1.f - g_fDissolve) - Normal_fDissolve) && ((1.f - g_fDissolve) - Normal_fDissolve) > 0.f)
    {
        E = N;
    }
    if ((0.05f > (1.f - g_fOffset1) - Normal_fDissolve) && ((1.f - g_fOffset1) - Normal_fDissolve) > 0.f)
    {
        E += N;
    }
    clip((1.f - g_fDissolve) - Normal_fDissolve);
    
    return lerp((D + N), E,0.5f);
}