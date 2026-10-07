// The browser's heightfield shader: the first shader of the browser's own
// that a part may be drawn by (a shaded Look, SHADER_HEIGHTFIELD), through
// render's custom pipelines. It is the shape terrain will take: a flat grid
// (mesh's plane, -1 to 1 in x and z, one vertex a texel) lifted in the
// vertex stage by an r32float texture of heights (the part's Field, in its
// own units), its normal from the heights about it, coloured from a low
// colour to a high one by height, and lit by render's one convention (the
// sun's colour an irradiance, so diffuse is albedo / pi times it; the
// ambient light a radiance), times the exposure; the high colour glows a
// little ('glow' x the high colour, a radiance in nits).
//
// Compiled ahead of time by Cone's tools/shaders/shaders.py, run over this
// folder, and embedded in shaders.cone.
//
// Group 0 is render's frame (renderframe.slang); group 1 is the material's,
// the custom pipeline's layout: its parameters, its texture, render's
// sampler (unused: an r32float texture is read texel by texel, Load, since
// not every GPU filters one).

import renderframe;

struct Params {
    float4 low;                    // rgb the colour at height 0; w the height the high colour is reached at
    float4 high;                   // rgb the colour there; w how much it glows, nits
};

[[vk::binding(0, 1)]] ConstantBuffer<Params> params;
[[vk::binding(1, 1)]] Texture2D<float> heights;
[[vk::binding(2, 1)]] SamplerState unused;

// Position only: the pipeline reads one vertex stream (shaders.cone)
struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float height : TEXCOORD;
    nointerpolation float4 tint : COLOR;
};

// The height at texel (x, z), clamped to the grid
float heightAt(int x, int z, int2 size) {
    return heights.Load(int3(clamp(x, 0, size.x - 1), clamp(z, 0, size.y - 1), 0));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    uint w;
    uint h;
    heights.GetDimensions(w, h);
    int2 size = int2(int(w), int(h));
    // The texel under this vertex: the plane runs -1 to 1, a vertex a texel
    int x = int(round((input.position.x + 1.0) * 0.5 * float(size.x - 1)));
    int z = int(round((input.position.z + 1.0) * 0.5 * float(size.y - 1)));
    float y = heightAt(x, z, size);
    // The normal from the heights either side (central differences)
    float dx = 2.0 / float(max(size.x - 1, 1));
    float dz = 2.0 / float(max(size.y - 1, 1));
    float sx = (heightAt(x + 1, z, size) - heightAt(x - 1, z, size)) / (2.0 * dx);
    float sz = (heightAt(x, z + 1, size) - heightAt(x, z - 1, size)) / (2.0 * dz);
    float3 n = float3(-sx, 1.0, -sz);
    float4 world = mul(inst.model, float4(input.position.x, y, input.position.z, 1.0));
    output.position = mul(frame.viewProjection, world);
    output.normal = worldNormal(inst.model, n);
    output.height = y;
    output.tint = inst.tint;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float3 n = normalize(input.normal);
    float t = saturate(input.height / max(params.low.w, 1e-6));
    float3 albedo = lerp(params.low.rgb, params.high.rgb, t) * input.tint.rgb;
    float diffuse = max(dot(n, frame.lightDirection.xyz), 0.0);
    float3 color = albedo * (frame.ambient.rgb + frame.lightColor.rgb * (diffuse / RENDER_PI))
        + params.high.rgb * (params.high.w * t * t);
    return float4(color * frame.exposure.x, 1.0);
}
