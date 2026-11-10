// ==============================================================================
// PROJECT NEON CITY - DirectX 11 HLSL Shader Pipeline (Basic3D.hlsl)
// PBR-Lite Lighting with Directional Sun, Neon Emissives, and Atmospheric Fog
// ==============================================================================

cbuffer cbPerFrame : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
    float3   gCamPos;
    float    gTime;
    float3   gLightDir;
    float    gPadding0;
    float4   gLightColor;
    float4   gFogColor;
    float    gFogStart;
    float    gFogRange;
    float2   gPadding1;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4   gBaseColor;
    float    gRoughness;
    float    gMetallic;
    float    gEmissiveIntensity;
    float    gPadding2;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

struct PS_INPUT
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

// ------------------------------------------------------------------------------
// Vertex Shader
// ------------------------------------------------------------------------------
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    
    // Transform vertex position from local to world space
    float4 posW = mul(float4(input.Position, 1.0f), gWorld);
    output.PosW = posW.xyz;
    
    // Transform normal to world space (assuming orthogonal world transform)
    output.NormalW = mul(input.Normal, (float3x3)gWorld);
    output.NormalW = normalize(output.NormalW);
    
    // Transform position to homogeneous clip space
    float4 posV = mul(posW, gView);
    output.PosH = mul(posV, gProj);
    
    output.TexCoord = input.TexCoord;
    output.Color = input.Color * gBaseColor;
    
    return output;
}

// ------------------------------------------------------------------------------
// Pixel Shader
// ------------------------------------------------------------------------------
float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.NormalW);
    float3 V = normalize(gCamPos - input.PosW);
    float3 L = normalize(-gLightDir);
    
    // 1. Ambient lighting (Sky / Ground gradient)
    float hemiFactor = saturate(N.y * 0.5f + 0.5f);
    float3 skyAmbient = float3(0.08f, 0.12f, 0.22f);
    float3 groundAmbient = float3(0.03f, 0.04f, 0.06f);
    float3 ambient = lerp(groundAmbient, skyAmbient, hemiFactor);
    
    // 2. Diffuse lighting (Lambertian)
    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = NdotL * gLightColor.rgb;
    
    // 3. Specular lighting (Blinn-Phong approximation)
    float3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0f);
    float shininess = lerp(8.0f, 256.0f, 1.0f - gRoughness);
    float specFactor = pow(NdotH, shininess) * (1.0f - gRoughness);
    float3 specular = specFactor * gLightColor.rgb * lerp(0.04f, input.Color.rgb, gMetallic);
    
    // 4. Emissive Neon glow contribution
    float3 emissive = input.Color.rgb * gEmissiveIntensity;
    
    // Combine surface lighting
    float3 surfaceColor = (ambient + diffuse) * input.Color.rgb + specular + emissive;
    
    // 5. Atmospheric distance fog
    float distToCam = length(gCamPos - input.PosW);
    float fogFactor = saturate((distToCam - gFogStart) / gFogRange);
    float3 finalColor = lerp(surfaceColor, gFogColor.rgb, fogFactor);
    
    return float4(finalColor, input.Color.a);
}
