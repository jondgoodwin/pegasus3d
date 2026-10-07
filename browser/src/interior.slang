// A lit room seen through a window: a card just behind the glass, its uvs 0
// to 1 across the window, painted by this shader. By day it is a dim
// interior: its colour (the params' second row) lit by a share of the
// ambient light, as a room is darker than outside. At night it glows: the
// params' first row (nits), set by the world's Glow at dusk and dawn, times
// a pattern of the room: a lamp's bright shade a little left of the middle
// and its warm light falling off round it, the dark back of a sofa across
// the lower third, curtains gathered at both sides (darker, folded), a
// darker sill below and ceiling above. Opaque: the glass in front of it is
// drawn see-through over it.
//
// Group 0 is render's frame; group 1 the material's: its parameters and
// render's sampler (unused): no textures. Compiled by Cone's
// tools/shaders/shaders.py and embedded in shaders.cone.

import renderframe;

struct Params {
    float4 glow;         // rgb: the room's light at its brightest, nits
    float4 color;        // rgb: the room's colour by day (an albedo); w: the share of the ambient it gets
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
    float lit : FACING;
    nointerpolation float4 tint : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    output.position = mul(frame.viewProjection, mul(inst.model, float4(input.position, 1.0)));
    output.uv = input.uv;
    // The card faces out of the window: lit by the ambient light only from the front
    output.lit = 0.5 + 0.5 * saturate(length(input.normal));
    output.tint = inst.tint;
    return output;
}

// The room as seen through the window, 0 to 2: brightest at the lamp's
// shade, a little left of the middle, its light falling off round it; a
// dark sofa's back across the lower third, its top a soft curve
float roomPattern(float2 uv) {
    float2 d = uv - float2(0.42, 0.6);
    float lamp = 0.35 + 0.65 * exp(-(d.x * d.x * 6.0 + d.y * d.y * 8.0));
    // The shade itself: a small bright trapezoid, wider at its foot
    float shadeHalf = 0.07 + 0.05 * saturate((0.66 - uv.y) / 0.12);
    if (abs(uv.x - 0.42) < shadeHalf && uv.y > 0.54 && uv.y < 0.66) {
        lamp = 2.0;
    }
    float sofaTop = 0.3 + 0.03 * sin(uv.x * 9.0);
    float sofa = uv.y < sofaTop ? 0.35 + 0.15 * smoothstep(sofaTop - 0.06, sofaTop, uv.y) : 1.0;
    lamp = lamp * sofa;
    float curtain = 1.0;
    float side = min(uv.x, 1.0 - uv.x);
    if (side < 0.17) {
        // Folds across the gathered cloth, darker towards the window's edge
        curtain = (0.35 + 0.25 * (side / 0.17)) * (0.8 + 0.2 * sin(uv.x * 70.0));
    }
    float sill = uv.y < 0.1 ? 0.55 : 1.0;
    float ceiling = uv.y > 0.93 ? 0.7 : 1.0;
    return lamp * curtain * sill * ceiling;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float pattern = roomPattern(input.uv);
    float3 day = params.color.rgb * frame.ambient.rgb * (params.color.w * input.lit);
    float3 night = params.glow.rgb * input.tint.rgb;
    return float4((day + night) * pattern * frame.exposure.x, 1.0);
}
