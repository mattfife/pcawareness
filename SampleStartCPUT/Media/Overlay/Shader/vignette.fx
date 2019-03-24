// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD2; // Object space position 
};
// ********************************************************************************************************
    Texture2D    TEXTURE0 : register( t0 );
    SamplerState SAMPLER0 : register( s0 );
    Texture2D    _Shadow  : register( t1 );
    SamplerComparisonState SAMPLER1 : register( s1 );
// ********************************************************************************************************
cbuffer cbPerModelValues
{
    row_major float4x4 World : WORLD;
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    row_major float4x4 InverseWorld : INVERSEWORLD;
              float4   LightDirection;
              float4   EyePosition;
    row_major float4x4 LightWorldViewProjection;
};
// ********************************************************************************************************
// TODO: Note: nothing sets these values yet
cbuffer cbPerFrameValues
{
    row_major float4x4  View;
    row_major float4x4  Projection;
};
// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;	
	//output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
	
	
	float4 swizzle  = mul( float4( input.Pos, 1.0f), World );
	output.Pos 		= swizzle; //float4(swizzle.x, swizzle.y, swizzle.z, 1.0f);
	//output.Pos      = mul( float4( input.Pos, 1.0f), World );
    //output.Pos      = float4(input.Pos.x*500, input.Pos.y*500, 1.0f, 1.0f); 
    //output.Position = float4( input.Pos.x*500, input.Pos.z*500, 1.0f, 1.0f);
    output.Norm = input.Norm;
    output.Uv   = float2(input.Uv.x, input.Uv.y);
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    return TEXTURE0.Sample( SAMPLER0, input.Uv );
	//return float4( 1.0f, 1.0f, 1.0f, 1.0f );
    //return float4( diffuseTexture, 1.0f );
}

// ********************************************************************************************************
struct VS_INPUT_NO_TEX
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
};
struct PS_INPUT_NO_TEX
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD0; // Object space position 
};
// ********************************************************************************************************
PS_INPUT_NO_TEX VSMainNoTexture( VS_INPUT_NO_TEX input )
{
    PS_INPUT_NO_TEX output = (PS_INPUT_NO_TEX)0;
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;
    // TODO: transform the light into object space instead of the normal into world space
    output.Norm = mul( input.Norm, (float3x3)World );
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMainNoTexture( PS_INPUT_NO_TEX input ) : SV_Target
{
    float3 lightUv = input.LightUv.xyz / input.LightUv.w;
    float2 uv = lightUv.xy * 0.5f + 0.5f;
    float2 uvInvertY = float2(uv.x, 1.0f-uv.y);
    float shadowAmount = _Shadow.SampleCmp( SAMPLER1, uvInvertY, lightUv.z );
    float3 eyeDirection = normalize(input.Position - EyePosition.xyz);
    float3 normal       = normalize(input.Norm);
    float  nDotL = saturate( dot( normal, -normalize(LightDirection.xyz) ) );
    nDotL = shadowAmount * nDotL;
    float3 reflection   = reflect( eyeDirection, normal );
    float  rDotL        = saturate(dot( reflection, -LightDirection.xyz ));
    float  specular     = 0.2f * pow( rDotL, 4.0f );
    specular = min( shadowAmount, specular );
    return float4( (nDotL + specular).xxx, 1.0f);
}
