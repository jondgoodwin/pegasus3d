// The browser's view surfaces (viewsurface.cone): a quad that shows the scene drawn again from another camera,
// a mirror or a portal. It takes the render package's mirror material (viewmat.slang) and adds what a portal and
// a cheap mirror need:
//
// - A WINDOW. The picture is not the whole frame's view, only the part of it the quad covers (the quad's screen
//   rectangle, a little over): drawn into a smaller texture, from a camera whose frustum is cut to that rectangle,
//   so the second pass draws, shadows and culls only what the quad can show. 'crop' says where the window lies on
//   the frame: its middle (x, y) in the frame's clip space (x right, y up, -1 to 1), and the reciprocals of its
//   half sizes there (z, w).
// - A MIRROR OR A PORTAL. A mirror's picture is drawn mirrored left to right (a reflected camera: camera.cone), so
//   it is read at the window's x taken the other way round ('mode'.x 1); a portal's is drawn the right way round
//   ('mode'.x 0). Both read it at the fragment's own place in the frame, which is what makes the quad a hole
//   or a looking-glass and not a picture on a wall.
//
// Group 1: the parameters (binding 0), the view's texture (1) and the sampler (2).
// Compiled by tools/shaders/shaders.py and embedded in viewsurface.cone.

import renderframe;

struct SurfaceParams {
    float4 tint;                   // 0: rgb the colour multiplying the picture
    float4 crop;                   // 16: the window: middle x, y, then 1 / half width, 1 / half height (frame clip space)
    float4 mode;                   // 32: x 1 for a mirror (picture mirrored in x), 0 for a portal
    float4 spare;                  // 48
};

[[vk::binding(0, 1)]] ConstantBuffer<SurfaceParams> params;
[[vk::binding(1, 1)]] Texture2D viewTexture;
[[vk::binding(2, 1)]] SamplerState viewSampler;

// (the position stream alone: the pane's normal and uvs are not read, a surface is read at its place on the frame)
struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    nointerpolation float4 tint : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    float4 world = mul(inst.model, float4(input.position, 1.0));
    output.position = mul(frame.viewProjection, world);
    // A SURFACE IS NOT CUT AWAY BY THE NEAR PLANE. An eye about to go through a portal's pane is within the near plane's distance of it
    // for the last frame or two, and the pane would vanish then and show what is behind it (the wall the glass is in, the dead end behind the
    // mouth). Clamping the depth at the near plane keeps it, drawn in front of everything, until the eye is at its plane (the clip against
    // the eye's own plane, w, still cuts what is behind the eye). What is nearer than the near plane (the first-person head) stays cut.
    output.position.z = max(output.position.z, 0.0);
    output.tint = inst.tint;
    return output;
}

[shader("fragment")]
float4 surfaceFragment(VertexOutput input) : SV_Target {
    float2 s = input.position.xy / frame.viewport.xy;
    float2 clip = float2(s.x * 2.0 - 1.0, 1.0 - s.y * 2.0);
    float2 p = (clip - params.crop.xy) * params.crop.zw;
    if (params.mode.x > 0.5) { p.x = -p.x; }
    float2 uv = saturate(float2(p.x * 0.5 + 0.5, 0.5 - p.y * 0.5));
    float3 c = viewTexture.Sample(viewSampler, uv).rgb;
    return float4(c * params.tint.rgb * input.tint.rgb, 1.0);
}
