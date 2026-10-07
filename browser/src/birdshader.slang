// The browser's birds: one small mesh (a body, a head and beak, a forked tail
// and two wings of seven stations from shoulder to tip), drawn once, instanced
// for every bird through render's custom pipelines, its wings flapped in the
// vertex stage. A bird costs one instance record and no CPU vertex work.
//
// What each instance carries (render's Instance: a model matrix and a tint)
// is this pipeline's own to read, since it is its own shader:
//   model, the upper 3 x 4: the bird's place and frame (right, up, back =
//          -forward as its columns, times its scale), as for any mesh
//   model's fourth row: x the wings' amplitude 0 to 1 (0 held out, 1 beating
//          hard), y unused, z unused, w 1 (never used as a matrix row here)
//   tint:  rgb a brightness and warmth each bird differs by; a the wings' phase, radians
//
// THE MESH (skyrig.cone's birdMesh): forward is -z, the wings flat in the
// y = 0 plane at rest, a wing's shoulder 0.018 m from the body's middle and
// its tip at 0.20; the mesh's uv.x is 1 for a wing vertex and 0 for the
// body, uv.y is 1 on the beak.
//
// THE FLAP. A wing is two segments, the arm (shoulder to wrist, 0.062 m) and
// the hand (wrist to tip, 0.165 m). The arm beats about the shoulder at
// theta1 = amp (0.70 sin(phase) + 0.10) (up 40 degrees, down 30 at full
// amplitude, a slight upward dihedral held at none); the hand follows its
// own beat, lagging a quarter of a cycle less 0.9 rad: theta2 = theta1 + amp
// 0.70 sin(phase - 0.9), and folds in on the upstroke (cos(phase) > 0: the
// beat is rising) to 65 per cent of its length at full amplitude; the tip's
// trailing edge also drags back with the lag. So the wing bends into a bow
// on the downstroke and tucks on the way up, as a small bird's does, with
// no other animation data. The body does not flap.
//
// LIGHTING: two-sided (a wing is seen from above and below): the normal is
// turned to face the eye, a wing seen from underneath is pale grey, from
// above dark brown; the body is pale below and dark above by the mesh's own
// normal; the beak orange. Lit by the frame's light and ambient, times the exposure.

import renderframe;

[[vk::binding(0, 1)]] ConstantBuffer<float4> params;
[[vk::binding(1, 1)]] SamplerState unused;

struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 restUp : TEXCOORD0;       // the rest normal's height: below the middle, the belly
    float3 world : TEXCOORD1;
    float2 flags : TEXCOORD2;        // x wing, y beak
    nointerpolation float4 tint : COLOR;
};

static const float SHOULDER = 0.018;
static const float WRIST = 0.062;     // along the wing from the shoulder
static const float SPAN = 0.227;      // shoulder to tip

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    float amp = inst.model[3][0];
    float phase = inst.tint.w;
    float3 p = input.position;
    float3 n = input.normal;
    if (input.uv.x > 0.5) {
        float side = (p.x < 0.0) ? -1.0 : 1.0;
        float xr = clamp(abs(p.x) - SHOULDER, 0.0, SPAN);
        float theta1 = amp * (0.70 * sin(phase) + 0.10);
        float theta2 = theta1 + amp * 0.70 * sin(phase - 0.9);
        float fold = 1.0 - 0.35 * amp * max(cos(phase), 0.0);
        float xi = min(xr, WRIST);
        float xo = max(xr - WRIST, 0.0) * fold;
        float horiz = xi * cos(theta1) + xo * cos(theta2);
        float vert = xi * sin(theta1) + xo * sin(theta2);
        float drag = 0.012 * (xr / SPAN) * (xr / SPAN) * sin(phase - 0.5) * amp;
        p = float3(side * (SHOULDER + horiz), p.y + vert, p.z + drag);
        float blend = smoothstep(WRIST - 0.03, WRIST + 0.03, xr);
        float theta = lerp(theta1, theta2, blend);
        // The rest normal is straight up; the wing's slope tilts it
        n = float3(-side * sin(theta), cos(theta), 0.0);
        if (input.normal.y < 0.0) { n = -n; }
    }
    float4 world = mul(inst.model, float4(p, 1.0));
    output.position = mul(frame.viewProjection, float4(world.xyz, 1.0));
    output.normal = worldNormal(inst.model, n);
    output.restUp = float3(input.normal.y, 0.0, 0.0);
    output.world = world.xyz;
    output.flags = input.uv;
    output.tint = inst.tint;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float3 n = normalize(input.normal);
    float3 view = normalize(frame.cameraPosition.xyz - input.world);
    bool wing = input.flags.x > 0.5;
    // Two-sided: face the eye, and know which side of a wing it is
    bool seenFromAbove = dot(n, view) >= 0.0;
    if (!seenFromAbove) { n = -n; }
    float3 back = float3(0.075, 0.062, 0.050);
    float3 belly = float3(0.55, 0.52, 0.46);
    float3 albedo;
    if (wing) {
        // Above dark brown with a paler edge's worth of variation by the tint, below a pale grey
        albedo = seenFromAbove ? back : float3(0.46, 0.44, 0.40);
    } else {
        albedo = lerp(belly, back, smoothstep(-0.35, 0.35, input.restUp.x));
        if (input.flags.y > 0.5) { albedo = float3(0.70, 0.45, 0.12); }
    }
    albedo *= input.tint.rgb;
    float diffuse = max(dot(n, frame.lightDirection.xyz), 0.0);
    // A wing lets some light through: the lit side shows a little of the other
    float through = wing ? 0.25 * max(dot(-n, frame.lightDirection.xyz), 0.0) : 0.0;
    float3 color = albedo * (frame.ambient.rgb + frame.lightColor.rgb * ((diffuse + through) / RENDER_PI));
    return float4(color * frame.exposure.x, 1.0);
}
