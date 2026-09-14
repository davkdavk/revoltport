struct TransformedVertex
{
    float4 Position : POSITION0;
    float4 Diffuse  : COLOR0;
    float4 Specular : COLOR1;
    float2 Tex0     : TEXCOORD0;
    float2 Tex1     : TEXCOORD1;
};

struct TransformedPixel
{
    float4 Position : POSITION0;
    float4 Diffuse  : COLOR0;
    float4 Specular : COLOR1;
    float2 Tex0     : TEXCOORD0;
    float2 Tex1     : TEXCOORD1;
};

TransformedPixel transformed_vs(TransformedVertex input)
{
    TransformedPixel output;
    output.Position = input.Position;
    output.Diffuse = input.Diffuse;
    output.Specular = input.Specular;
    output.Tex0 = input.Tex0;
    output.Tex1 = input.Tex1;
    return output;
}

sampler Texture0 : register(s0);
sampler Texture1 : register(s1);

// Fog color pushed per draw by rv360_push_fog_constant (from FOG_COLOR).
// Fog factor arrives in specular alpha: the game bakes per-vertex fog on the
// CPU as FTOL3(fog) << 24. Approximation of the removed fixed-function fog;
// tune against hardware captures if the blend looks wrong.
float4 FogColor : register(c0);

float4 transformed_ps(TransformedPixel input) : COLOR0
{
    float4 tex = tex2D(Texture0, input.Tex0) * input.Diffuse;
    return lerp(tex, FogColor, input.Specular.a);
}

// Dual-texture variant for the pickup-flash effect (panel.cpp): replaces the
// fixed-function SUBTRACT combiner (stage1 texture minus current).
float4 transformed2_ps(TransformedPixel input) : COLOR0
{
    float4 t0 = tex2D(Texture0, input.Tex0);
    float4 t1 = tex2D(Texture1, input.Tex1);
    float4 tex = (t0 - t1) * input.Diffuse;
    return lerp(tex, FogColor, input.Specular.a);
}
