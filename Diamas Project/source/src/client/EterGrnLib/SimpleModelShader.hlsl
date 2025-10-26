struct VS_IN
{
    float3  Position        : POSITION0;
    float4  BoneWeights     : BLENDWEIGHT;
    int4  BoneIndices     : BLENDINDICES;
    float3  Normal          : NORMAL;
    float3  Tangent          : TANGENT;
    float3  Binormal          : BINORMAL;
    float2  Tex0            : TEXCOORD0;
};

struct VS_IN_RIGID
{
    float3  Position        : POSITION0;
    float3  Normal          : NORMAL;
    float3  Tangent          : TANGENT;
    float3  Binormal          : BINORMAL;
    float2  Tex0            : TEXCOORD0;
};

struct VS_IN_RIGID_2
{
    float3  Position        : POSITION0;
    float3  Normal          : NORMAL;
    float3  Tangent          : TANGENT;
    float3  Binormal          : BINORMAL;
    float2  Tex0            : TEXCOORD0;
    float2  Tex1            : TEXCOORD1;

};

struct VS_OUT
{
    float4 Position         : POSITION;
    float4 Diffuse          : COLOR0;
    float2 Tex0             : TEXCOORD0;
    float2 TexSpec          : TEXCOORD1;
    float  FogValue : TEXCOORD2;
};

struct VS_OUT_2
{
    float4 Position         : POSITION;
    float4 Diffuse          : COLOR0;
    float2 Tex0             : TEXCOORD0;
    float2 Tex1             : TEXCOORD1;
    float2 TexSpec          : TEXCOORD2;
    float  FogValue : TEXCOORD3;
};


// Must match the #defines in dx_sample.cpp
float4x3 ObjToWorld;
float4x4 g_viewProj;
float3 g_vCameraPos;
float3   DirFromLight = {0.5f, 1.0f, 0.5f};
float4   LightColour = {1.0f, 0.8f, 0.8f, 1.0f};
float4   g_vDiffuseColor = {1.0f, 1.0f, 1.0f, 1.0f};
float3      g_vFogColor;                        // color of fog (and likely the background)
float3      g_vFogParams = {300.0f, 2000.0f, 1700.0f};
float4      g_vMaterialDiffuse = {1.0f, 1.0f, 1.0f, 1.0f};
float4      g_vMaterialAmbient = {0.3f, 0.3f, 0.3f, 1.0f}; 
float4x3  BoneMatrices[64];
float4x4 g_mViewToLightProj;  // Transform from view space to light projection space
float g_fAlphaValue = 1.0f;
float4x4 shadowTexMatrix;
float3x3 g_specularMat;
bool g_specularEnable = false;
float g_specularPower = 1.0f;



void CalcDiffuseLight( float3 lightDir, float3 normal, float4 ambient, float4 diffuse,
	out float4 oDiffuse)
{
	float ndotl = saturate( dot( normal, lightDir ) );
	oDiffuse = ndotl * diffuse + (ambient * 0.5f);		
}

float FogValue(float fPoint)
{
    float fFogEnd = g_vFogParams.y;
    float fFogDist = g_vFogParams.z;
    
    return saturate((fFogEnd - fPoint) / fFogDist);
}
//#define TRILINEAR_SAMPLER sampler_state { MipFilter = LINEAR; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; }
texture diffuse_texture;
sampler2D diffuse_sampler = sampler_state
{
    Texture = <diffuse_texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};
texture opacity_texture;
sampler2D opacity_sampler = sampler_state
{
    Texture = <opacity_texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

texture g_specularTex;
sampler2D specular_sampler = sampler_state
{
    Texture = <g_specularTex>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

texture  g_txShadow;
sampler2D g_samShadow =
sampler_state
{
    Texture = <g_txShadow>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
    AddressU = Clamp;
    AddressV = Clamp;
};

VS_OUT SkinnedVS( VS_IN In )
{
    VS_OUT Out;

    float4 InPos     = float4( In.Position, 1 );
    float4 InNormal  = float4( In.Normal, 0 );
    float3 WorldPos    = 0;
    float3 WorldNormal = 0;

    for( int i = 0; i < 4; ++i )
    {
        float4x3 BoneMatrix = BoneMatrices[In.BoneIndices[i]];
        WorldPos    += In.BoneWeights[i] * mul( InPos, BoneMatrix );
        WorldNormal += In.BoneWeights[i] * mul( InNormal, BoneMatrix );
    }

    float4 ClipPos = mul(float4(WorldPos, 1), g_viewProj);

    float3 l = normalize( -DirFromLight );
	float3 n = normalize( WorldNormal );
    
	CalcDiffuseLight( l, n, g_vMaterialAmbient, LightColour * g_vMaterialDiffuse, Out.Diffuse );

    Out.Position   = ClipPos;
    Out.Tex0    = In.Tex0;
    Out.FogValue = FogValue(Out.Position.z);
    
    float3 EyeVec = g_vCameraPos.xyz - WorldPos.xyz;
    Out.TexSpec= mul(reflect(normalize(EyeVec), n), g_specularMat).xy;


    return Out;
}

VS_OUT RigidVS( VS_IN_RIGID In )
{
    VS_OUT Out;
    float3 WorldPos = mul(float4(In.Position, 1), ObjToWorld);
    float4 ClipPos  = mul(float4(WorldPos, 1), g_viewProj);
    float4 InNormal  = float4( In.Normal, 0);
    float3 ObjNormal = mul(InNormal, ObjToWorld);
    float3 l = normalize( -DirFromLight );
	float3 n = normalize( ObjNormal );
    
	CalcDiffuseLight( l, n, g_vMaterialAmbient, LightColour * g_vMaterialDiffuse, Out.Diffuse );

    Out.Position = ClipPos;
    Out.Tex0     = In.Tex0;
    Out.FogValue = FogValue(Out.Position.z);
     
    float3 EyeVec = g_vCameraPos.xyz - WorldPos.xyz;
    Out.TexSpec= mul(reflect(normalize(EyeVec), n), g_specularMat).xy;

    return Out;
}


VS_OUT_2 RigidVS2( VS_IN_RIGID_2 In )
{
    VS_OUT_2 Out;
    float3 WorldPos = mul(float4(In.Position, 1), ObjToWorld);
    float4 ClipPos  = mul(float4(WorldPos, 1), g_viewProj);
    float4 InNormal  = float4( In.Normal, 0);
    float3 ObjNormal = mul(InNormal, ObjToWorld);
    float3 l = normalize( -DirFromLight );
	float3 n = normalize( ObjNormal );
    
	CalcDiffuseLight( l, n, g_vMaterialAmbient, LightColour * g_vMaterialDiffuse, Out.Diffuse );

    Out.Position = ClipPos;
    Out.Tex0     = In.Tex0;
    Out.Tex1     = In.Tex1;
    Out.FogValue = FogValue(Out.Position.z);
     
    float3 EyeVec = g_vCameraPos.xyz - WorldPos.xyz;
    Out.TexSpec= mul(reflect(normalize(EyeVec), n), g_specularMat).xy;

    return Out;
}

void SkinnedShadowVS(   
    VS_IN In,
    out float4 oPos : POSITION,
    out float2 Depth : TEXCOORD0
    )
{

    float BoneWeights[4] = (float[4])In.BoneWeights;
    int BoneIndices[4]   = (int[4])In.BoneIndices;

    float4 InPos     = float4( In.Position, 1 );
    float4 InNormal  = float4( In.Normal, 0 );
    float3 WorldPos    = 0;
    float3 WorldNormal = 0;

    for( int i = 0; i < 4; ++i )
    {
        float4x3 BoneMatrix = BoneMatrices[BoneIndices[i]];
        WorldPos    += BoneWeights[i] * mul( InPos, BoneMatrix );
        WorldNormal += BoneWeights[i] * mul( InNormal, BoneMatrix );
    }

    float4 ClipPos = mul(float4(WorldPos, 1), g_viewProj);

    oPos = ClipPos;
    Depth = ClipPos.zw;
}

void RigidShadowVS( 
    VS_IN_RIGID In,
    out float4 oPos : POSITION,
    out float2 Depth : TEXCOORD0
    )
{
    float3 WorldPos = mul(float4(In.Position, 1), ObjToWorld);
    float4 ClipPos  = mul(float4(WorldPos, 1), g_viewProj);
    oPos = ClipPos;
    Depth = ClipPos.zw;
}


float4 GrannyPS ( VS_OUT In ) : COLOR
{

    if(In.FogValue <= 0.0f)
        discard;
        
	float4 baseColor = tex2D( diffuse_sampler, In.Tex0 );
	float4 specularColor = tex2D(specular_sampler, In.TexSpec);

	float3 color = baseColor.rgb * In.Diffuse.rgb;
	
	float4 outColor = float4( color, In.Diffuse.a );
    outColor.rgb = outColor.rgb + (baseColor.a * specularColor.rgb * g_specularPower);

    outColor.xyz = lerp(g_vFogColor, outColor.xyz, In.FogValue).xyz;
    outColor.rgb = outColor.rgb * g_vDiffuseColor.rgb;

    return outColor;
}

float4 GrannyPS2 ( VS_OUT_2 In ) : COLOR
{

    if(In.FogValue <= 0.0f)
        discard;
        
	float4 baseColor = tex2D( diffuse_sampler, In.Tex0 );
	float4 specularColor = tex2D(specular_sampler, In.TexSpec);
	float4 tex2Color = tex2D( opacity_sampler, In.Tex1 );

	float3 color = baseColor.rgb * In.Diffuse.rgb;
	
	float4 outColor = float4( color, In.Diffuse.a );
    outColor.rgb = outColor.rgb + (baseColor.a * specularColor.rgb * g_specularPower);
    outColor.rgb = outColor.rgb * g_vDiffuseColor.rgb;
    outColor = outColor * tex2Color;
    outColor.xyz = lerp(g_vFogColor, outColor.xyz, In.FogValue).xyz;

    return outColor;
}

float4 GrannyPSBlend2 ( VS_OUT_2 In ) : COLOR
{

        if(In.FogValue <= 0.0f)
        discard;
    if(g_fAlphaValue < 0.1f)
        discard;

    if(In.FogValue <= 0.0f)
        discard;
        
	float4 baseColor = tex2D( diffuse_sampler, In.Tex0 );
	float4 tex2Color = tex2D( opacity_sampler, In.Tex1 );

	float3 color = baseColor.rgb * In.Diffuse.rgb;
	
	float4 outColor = float4( color, baseColor.a * In.Diffuse.a );
	outColor.rgb = outColor.rgb * g_vDiffuseColor.rgb;
    outColor = outColor * tex2Color;
    outColor.xyz = lerp(g_vFogColor, outColor.xyz, In.FogValue).xyz;

    return float4(outColor.xyz, baseColor.a);
}

float4 GrannyPSBlend ( VS_OUT In ) : COLOR
{
    if(In.FogValue <= 0.0f)
        discard;
    if(g_fAlphaValue < 0.1f)
        discard;

    if(In.FogValue <= 0.0f)
        discard;
        
	float4 baseColor = tex2D( diffuse_sampler, In.Tex0 );
	
	float3 color = baseColor.rgb * In.Diffuse.rgb;
	
	float4 outColor = float4( color, baseColor.a * In.Diffuse.a );
	outColor.rgb = outColor.rgb * g_vDiffuseColor.rgb;

    outColor.xyz = lerp(g_vFogColor, outColor.xyz, In.FogValue).xyz;

    return float4(outColor.xyz, g_fAlphaValue);
}

float4 GrannyShadowPS(
    float4 oPos : POSITION,
    float2 Depth : TEXCOORD0) : COLOR
{
    float fColor = Depth.x / Depth.y;
	return float4( fColor,fColor, fColor, 1.f );
}

technique Rigid
{
    pass Base
    {
        CullMode = none;
        AlphaBlendEnable = false;
        ZEnable = true;
        ZWriteEnable = true;
        VertexShader = compile vs_3_0 RigidVS();
        PixelShader = compile ps_3_0 GrannyPS();

    }

    pass Shadow
    {
        VertexShader = compile vs_3_0 RigidShadowVS();
        PixelShader = compile ps_3_0 GrannyShadowPS();
    }

    pass Blend
    {
        CullMode = none;
		AlphaBlendEnable = true;
        ZEnable = true;
        ZWriteEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;
        VertexShader = compile vs_3_0 RigidVS();
        PixelShader = compile ps_3_0 GrannyPSBlend();

    }

    pass Opacity
    {
        CullMode = none;
        AlphaBlendEnable = true;
        ZEnable = true;
        ZWriteEnable = true;
        AlphaBlendEnable = false;
        AlphaTestEnable = true;
        AlphaFunc = Greater;
        AlphaRef = 0;
        VertexShader = compile vs_3_0 RigidVS();
        PixelShader = compile ps_3_0 GrannyPS();

    }
}

technique Skinned
{
    pass P0
    {

        CullMode = none;
        AlphaBlendEnable = false;
        ZEnable = true;
        ZWriteEnable = true;
        VertexShader = compile vs_3_0 SkinnedVS();
        PixelShader = compile ps_3_0 GrannyPS();

    }

    pass P1
    {
        VertexShader = compile vs_3_0 SkinnedShadowVS();
        PixelShader = compile ps_3_0 GrannyShadowPS();
    }
    
    pass P2
    {
        ZEnable = true;
        ZWriteEnable = true;
        CullMode = none;
		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;
        VertexShader = compile vs_3_0 SkinnedVS();
        PixelShader = compile ps_3_0 GrannyPSBlend();

    }

    pass Opacity
    {
        CullMode = none;
        AlphaBlendEnable = false;
        ZEnable = true;
        ZWriteEnable = true;
        AlphaBlendEnable = false;
        AlphaTestEnable = true;
        AlphaFunc = Greater;
        AlphaRef = 0;
        VertexShader = compile vs_3_0 RigidVS();
        PixelShader = compile ps_3_0 GrannyPS();

    }
}

technique RigidTwoTexture
{
    pass Base
    {
        CullMode = none;
        AlphaBlendEnable = false;
        ZEnable = true;
        ZWriteEnable = true;
        VertexShader = compile vs_3_0 RigidVS2();
        PixelShader = compile ps_3_0 GrannyPS2();

    }

    pass Shadow
    {
        VertexShader = compile vs_3_0 RigidShadowVS();
        PixelShader = compile ps_3_0 GrannyShadowPS();
    }

    pass Blend
    {
        CullMode = none;
		AlphaBlendEnable = true;
        ZEnable = true;
        ZWriteEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;
        VertexShader = compile vs_3_0 RigidVS2();
        PixelShader = compile ps_3_0 GrannyPSBlend2();

    }

    pass Opacity
    {
        CullMode = none;
        AlphaBlendEnable = true;
        ZEnable = true;
        ZWriteEnable = true;
        AlphaBlendEnable = false;
        AlphaTestEnable = true;
        AlphaFunc = Greater;
        AlphaRef = 0;
        VertexShader = compile vs_3_0 RigidVS2();
        PixelShader = compile ps_3_0 GrannyPS2();

    }
}