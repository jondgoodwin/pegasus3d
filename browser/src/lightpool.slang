// A pool of light painted on the ground: under a lantern, below a lit
// window. Not a light source: nothing is lit by it; it is light added to
// what is drawn behind it, so it is drawn see-through (render's ALPHA_BLEND:
// premultiplied alpha, and an alpha of 0 is pure addition), depth tested
// against the ground it lies on, after everything opaque. Its mesh is a
// grid lying on the ground (its vertices at the ground's height, a few
// centimetres up), its uvs 0 to 1 across the pool, so its shape is the
// mesh's: a disc's square under a lantern, a long rectangle under a window.
//
// THE FALLOFF, from the uvs' middle out (d: 0 there, 1 at the edge's
// middle): (1 - d^2)^2, which is 0 with no slope at the edge, so the pool
// has no rim, times a core of 0.4 + 0.6 exp(-k d^2) (k the params' w, its
// sharpness), so it is brightest under the lamp and soft at its fringe. Its
// colour is the params' (nits, the light's warm colour times its
// strength), times the copy's tint (a lantern brightened by nearness), times
// the exposure.
//
// Group 0 is render's frame; group 1 the material's: its parameters and
// render's sampler (unused): no textures. Compiled by Cone's
// tools/shaders/shaders.py and embedded in shaders.cone.

import renderframe;

struct Params {
    float4 color;        // rgb: the light at the pool's middle, nits; w: the core's sharpness
    float4 unused;
};

[[vk::binding(0, 1)]] ConstantBuffer<Params> params;
[[vk::binding(1, 1)]] SamplerState unused;

struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float facing : FACING;
    nointerpolation float4 tint : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    output.position = mul(frame.viewProjection, mul(inst.model, float4(input.position, 1.0)));
    output.uv = input.uv;
    // A ground too steep to hold the light (a cliff) takes less of it
    output.facing = saturate(normalize(worldNormal(inst.model, input.normal)).y * 1.5);
    output.tint = inst.tint;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float2 p = input.uv * 2.0 - 1.0;
    float d2 = dot(p, p);
    float edge = saturate(1.0 - d2);
    float f = edge * edge * (0.4 + 0.6 * exp(-params.color.w * d2)) * input.facing;
    float3 light = params.color.rgb * input.tint.rgb * f;
    return float4(light * frame.exposure.x, 0.0);
}
