// The browser's fog: depth slabs. There is no pass that reads the depth, so
// the fog is a stack of see-through quads, each facing the camera at its own
// distance and covering the view, drawn after the opaque things with the
// depth test on (and the depth not written): a quad is drawn only over
// what is beyond it, so a thing is veiled by every slab in front of it and
// by none behind it, and the veil grows with its distance, as fog does. All
// the slabs are one colour, so the order they blend in does not matter.
//
// What each slab carries is an instance of render's (a model matrix, a tint):
//   model  where the quad is: the unit plane (mesh plane, -1 to 1 in x and
//          z) scaled to cover the view at its distance, turned to face the eye
//   tint   rgb the fog's colour times the slab's alpha, in the frame's
//          exposed units (blended as premultiplied alpha, the pipeline's
//          ALPHA_BLEND), a the slab's alpha: 1 - e^(-density x its thickness)
//
// Fog is a low thing: it lies in the air near the ground and the sky above is
// clear of it, so a slab is faded out where the ray through it climbs
// (the sky is a far-plane draw, so every slab is in front of it): full below
// the horizon and to 8 degrees above it, gone by 40 degrees. The sky draws
// its own haze (skyshader.slang) for the band near the horizon.
//
// Compiled by tools/shaders/shaders.py and embedded in skyrig.cone.

import renderframe;

[[vk::binding(0, 1)]] ConstantBuffer<float4> params;
[[vk::binding(1, 1)]] SamplerState unused;

struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 world : TEXCOORD0;
    nointerpolation float4 tint : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    float4 world = mul(inst.model, float4(input.position, 1.0));
    output.world = world.xyz;
    output.position = mul(frame.viewProjection, float4(world.xyz, 1.0));
    output.tint = inst.tint;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float3 dir = normalize(input.world - frame.cameraPosition.xyz);
    float low = 1.0 - smoothstep(0.14, 0.64, dir.y);
    return input.tint * low;
}
