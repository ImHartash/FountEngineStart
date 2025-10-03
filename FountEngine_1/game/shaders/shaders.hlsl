cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
}

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, worldMatrix);
    pos = mul(pos, viewMatrix);
    pos = mul(pos, projectionMatrix);
    
    output.position = pos;
    
    float3 normal = mul(float4(input.normal, 0.0f), worldMatrix).xyz;
    output.normal = normalize(normal);
    
    output.texcoord = input.texcoord;
    
    return output;
}

float4 PS(VertexOutput input) : SV_Target
{
    float3 lightDir = normalize(float3(1.0f, -1.0f, 1.0f));
    float3 normal = normalize(input.normal);
    float intensity = saturate(dot(normal, -lightDir));
    
    return float4(intensity, intensity, intensity, 1.0f);
}