// The brook's water: a ribbon of see-through water lying in the trench the land has
// (island/brook.cone), a mesh along the course (bakedworld/brook.cone), seven vertices across,
// a pair of stations every half metre or so. Not a sea: its waves are a metre across and move
// along the ribbon, so the shader scrolls noise along the course instead of summing waves in
// the world.
//
// THE VERTEX DATA, packed into the three streams every mesh has:
//   uv      x: across, metres from the course (the wet half width is the normal's z; the
//              ribbon goes 0.3 m beyond it, a feather that fades out)
//           y: along, metres from the spring
//   normal  x: the water's speed, metres a second
//           y: foam, 0 to 1 (the rocks' wakes, the foot of each step)
//           z: the wet half width, metres
//
// THE FLOW. Two layers of value noise, a coarse and a fine, scroll downstream at speeds that
// differ (the second 1.7 times the first), the middle of the stream faster than its edges
// (1 - 0.5 across^2); their slopes tilt the surface's normal, in the ribbon's own frame (the
// tangents found from the screen derivatives of the world position and the uvs). Noise with no
// end does not repeat, so unlike Valve's flow maps (Vlachos, "Water Flow in Portal 2", SIGGRAPH
// 2010) there is no restart to hide.
//
// THE COLOUR is the sea's way (water.slang), with no depth buffer: a faint tea-green body, the
// sky reflected by Schlick's Fresnel term, the sun's glint off the ripples (what makes a brook
// read as moving), and foam where the vertices say, broken up by the scrolling cells. It is
// see-through (premultiplied alpha): the land's wet pebbles show through. The alpha fades
// across the last fifth of the width, and away over the last metres of the course, where it
// reaches the sea.
//
// Group 0 is render's frame; group 1 the material's: its parameters and render's sampler
// (unused): no textures. Compiled by Cone's tools/shaders/shaders.py and embedded in shaders.cone.

import renderframe;

struct Params {
    float4 course;       // x: the course's length (m), y: metres at its end over which the water fades away
    float4 body;         // rgb: the water's body colour (an albedo)
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
    float3 world : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3 flow : TEXCOORD2;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    float4 world = mul(inst.model, float4(input.position, 1.0));
    output.position = mul(frame.viewProjection, world);
    output.world = world.xyz;
    output.uv = input.uv;
    output.flow = input.normal;
    return output;
}

// ---- Noise (the sea's) ----

float hash21(float2 p) {
    uint2 q = uint2(int2(p));
    q = q * 1664525u + 1013904223u;
    q.x += q.y * 1664525u;
    q.y += q.x * 1664525u;
    q ^= q >> 16u;
    q.x += q.y * 1664525u;
    q.y += q.x * 1664525u;
    q ^= q >> 16u;
    return float(q.x >> 8u) * (1.0 / 16777216.0);
}

float vnoise(float2 p) {
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + float2(1.0, 0.0));
    float c = hash21(i + float2(0.0, 1.0));
    float d = hash21(i + float2(1.0, 1.0));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

// The slope of a noise layer at p, per noise cell
float2 rippleSlope(float2 p) {
    float e = 0.08;
    float x = vnoise(p + float2(e, 0.0)) - vnoise(p - float2(e, 0.0));
    float z = vnoise(p + float2(0.0, e)) - vnoise(p - float2(0.0, e));
    return float2(x, z) / (2.0 * e);
}

float foamTexture(float2 p) {
    float2 i = floor(p);
    float2 f = frac(p);
    float best = 1.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            float2 o = float2(float(x), float(y));
            float2 j = float2(hash21(i + o), hash21(i + o + 7.7));
            best = min(best, length(o + j - f));
        }
    }
    return smoothstep(0.55, 0.15, best);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float3 p = input.world;
    float2 uv = input.uv;
    float speed = input.flow.x;
    float foam = saturate(input.flow.y);
    float hw = max(input.flow.z, 0.05);
    float t = frame.viewport.z;
    float3 toEye = frame.cameraPosition.xyz - p;
    float dist = length(toEye);
    float3 V = toEye / dist;

    // The ribbon's own frame: along and across, in the world, from the screen derivatives
    float3 dpx = ddx(p);
    float3 dpy = ddy(p);
    float2 dux = ddx(uv);
    float2 duy = ddy(uv);
    float det = dux.x * duy.y - dux.y * duy.x;
    float3 Tu = float3(1.0, 0.0, 0.0);
    float3 Tv = float3(0.0, 0.0, 1.0);
    if (abs(det) > 1e-14) {
        Tu = (dpx * duy.y - dpy * dux.y) / det;
        Tv = (dpy * dux.x - dpx * duy.x) / det;
    }
    float lu = length(Tu);
    float lv = length(Tv);
    Tu = lu > 1e-6 ? Tu / lu : float3(1.0, 0.0, 0.0);
    Tv = lv > 1e-6 ? Tv / lv : float3(0.0, 0.0, 1.0);
    float3 N = cross(Tu, Tv);
    N = (length(N) > 1e-6) ? normalize(N) : float3(0.0, 1.0, 0.0);
    if (N.y < 0.0) {
        N = -N;
    }

    // Ripples: two layers scrolling downstream, the middle quicker than the edges
    float across = clamp(uv.x / hw, -1.0, 1.0);
    float mid = 1.0 - across * across;     // 1 mid-stream (the deep), 0 at the edge (the shallows)
    // The ripples run a little ahead of the water (2.2 times its speed) so that a stream reads as moving
    // from a walker's distance; bigger than a hand's width (0.6 m across, 1.2 m long) and a smaller layer
    float v = speed * (1.0 - 0.5 * across * across) * 2.2;
    float fadeR = saturate(1.0 - dist / 90.0);
    float2 q1 = float2(uv.x / 0.60, (uv.y - v * t) / 1.2);
    float2 q2 = float2(uv.x / 0.28 + 7.0, (uv.y - 1.7 * v * t) / 0.55);
    float2 r1 = rippleSlope(q1) * float2(1.0 / 0.60, 1.0 / 1.2);
    float2 r2 = rippleSlope(q2) * float2(1.0 / 0.28, 1.0 / 0.55);
    float amp = 0.060 + 0.030 * saturate(speed);
    float2 slope = (r1 * amp + r2 * (amp * 0.45)) * fadeR;
    float3 n = normalize(N - Tu * slope.x - Tv * slope.y);

    float3 L = frame.lightDirection.xyz;
    float3 skyLight = frame.ambient.rgb;
    float3 sunLight = frame.lightColor.rgb * (max(L.y, 0.0) / RENDER_PI);
    float3 lightOn = skyLight + sunLight;

    // The body, and the sky and sun reflected
    float c = saturate(dot(n, V));
    float F = 0.02 + 0.98 * pow(1.0 - c, 5.0);
    // Deeper (the middle) is darker and more opaque: the bed's pebbles show at the edges
    float3 body = params.body.rgb * lightOn * lerp(1.0, 0.30, mid);
    float3 R = reflect(-V, n);
    R.y = abs(R.y);
    // The sky in the water: bluer overhead, paler and warmer toward the horizon
    float3 skyColor = skyLight * lerp(float3(1.3, 1.5, 1.9), float3(2.6, 2.6, 2.7), pow(1.0 - R.y, 2.0));
    float glint = pow(max(dot(R, L), 0.0), 240.0) * 60.0 * fadeR;
    float3 sparkle = frame.lightColor.rgb * (glint / RENDER_PI);
    float3 color = body * (1.0 - F) + skyColor * F;
    // Long streaks of light running with the current, in the faster middle
    float streak = pow(vnoise(float2(uv.x * 7.0 + 3.0, (uv.y - v * t) * 0.8)), 3.0);
    color += skyLight * (0.9 * streak * mid * fadeR) * (1.0 - F);
    float a = saturate(0.34 + 0.34 * mid + 0.08 * saturate(speed) + 0.6 * F);

    // The mouth: in the last metres the water fans out into the sea over the sand in braided rills
    float mouth = smoothstep(params.course.x - 16.0, params.course.x - 2.0, uv.y);
    float braid = smoothstep(0.30, 0.60, vnoise(float2(uv.x * 1.9, uv.y * 0.55 - v * t * 0.2)) + 0.25 * vnoise(float2(uv.x * 5.0, uv.y * 0.9)));
    a *= lerp(1.0, 0.25 + 0.75 * braid, mouth * 0.85);

    // Foam, as lace: stretched along the current in streaks and bubbles, thick where the vertices say
    // (strong in the middle of a patch, ragged at its edge), drifting down at the water's own pace
    float cells = foamTexture(float2(uv.x * 7.0, (uv.y - 1.1 * v * t) * 4.0));
    float lace = vnoise(float2(uv.x * 12.0, (uv.y - v * t) * 2.2)) * 0.55 + vnoise(float2(uv.x * 29.0 + 5.0, (uv.y - 1.3 * v * t) * 5.0)) * 0.45;
    float froth = lace * 0.8 + cells * 0.35 + foam * 0.75 - 0.55;
    float f = smoothstep(0.0, 0.22, froth) * smoothstep(0.02, 0.2, foam);
    float3 foamColor = float3(0.92, 0.93, 0.91) * lightOn * (0.8 + 0.4 * lace);
    color = lerp(color, foamColor, f);
    a = max(a, f * 0.95);
    sparkle *= 1.0 - f;

    // The edge, and the end at the sea
    float edge = 1.0 - smoothstep(hw * 0.8, hw + 0.3, abs(uv.x));
    float end = 1.0 - smoothstep(params.course.x - params.course.y, params.course.x, uv.y);
    a *= edge * end;
    sparkle *= edge * end;
    return float4((color * a + sparkle) * frame.exposure.x, a);
}
