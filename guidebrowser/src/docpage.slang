// The document page drawn unlit, after the scene has been exposed and tone mapped: one quad in the world,
// textured with the page (an sRGB texture, so the sampler hands linear values and the sRGB target encodes them
// again: the page's bytes arrive as they were drawn). Compiled to docpage.spv by slangc and embedded in
// docpage_spv.cone by tools/gen_page_spv.py (the guide browser has no build step for shaders of its own).
//
// Group 0: the draw's parameters, the page's texture and its sampler.

struct PageParams {
    float4x4 transform;            // world to clip space (the camera's projection times its view)
    float4 rect;                   // the page in the world (z = 0): x0, y0 (bottom), x1, y1 (top)
    float4 extra;                  // x: the mip level bias (negative: sharper)
};

[[vk::binding(0, 0)]] ConstantBuffer<PageParams> page;
[[vk::binding(1, 0)]] Texture2D pageTexture;
[[vk::binding(2, 0)]] SamplerState pageSampler;

struct VertexOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float2 quadCorner(uint v) {
    float x = (v == 1u || v == 2u || v == 4u) ? 1.0 : 0.0;
    float y = (v == 2u || v == 4u || v == 5u) ? 1.0 : 0.0;
    return float2(x, y);
}

[shader("vertex")]
VertexOutput vertexMain(uint vid : SV_VulkanVertexID) {
    float2 c = quadCorner(vid);
    VertexOutput output;
    float3 p = float3(lerp(page.rect.x, page.rect.z, c.x), lerp(page.rect.y, page.rect.w, c.y), 0.0);
    output.position = mul(page.transform, float4(p, 1.0));
    output.uv = c;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float4 t = pageTexture.SampleBias(pageSampler, input.uv, page.extra.x);
    return float4(t.rgb, t.a * page.extra.y);       // extra.y: opacity (1 = as drawn)
}
