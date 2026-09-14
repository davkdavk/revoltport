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

float4 transformed_ps(TransformedPixel input) : COLOR0
{
    return tex2D(Texture0, input.Tex0) * input.Diffuse;
}
