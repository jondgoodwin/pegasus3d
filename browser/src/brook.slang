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
    float v = speed * (1.0 - 0.5 * across * across);
    float fadeR = saturate(1.0 - dist / 90.0);
    float2 q1 = float2(uv.x / 0.30, (uv.y - v * t) / 0.55);
    float2 q2 = float2(uv.x / 0.15 + 7.0, (uv.y - 1.7 * v * t) / 0.26);
    float2 r1 = rippleSlope(q1) * float2(1.0 / 0.30, 1.0 / 0.55);
    float2 r2 = rippleSlope(q2) * float2(1.0 / 0.15, 1.0 / 0.26);
    float amp = 0.020 + 0.010 * saturate(speed);
    float2 slope = (r1 * amp + r2 * (amp * 0.4)) * fadeR;
    float3 n = normalize(N - Tu * slope.x - Tv * slope.y);

    float3 L = frame.lightDirection.xyz;
    float3 skyLight = frame.ambient.rgb;
    float3 sunLight = frame.lightColor.rgb * (max(L.y, 0.0) / RENDER_PI);
    float3 lightOn = skyLight + sunLight;

    // The body, and the sky and sun reflected
    float c = saturate(dot(n, V));
    float F = 0.02 + 0.98 * pow(1.0 - c, 5.0);
    float3 body = params.body.rgb * lightOn;
    float3 R = reflect(-V, n);
    R.y = abs(R.y);
    float3 skyColor = skyLight * 2.0;
    float glint = pow(max(dot(R, L), 0.0), 240.0) * 60.0 * fadeR;
    float3 sparkle = frame.lightColor.rgb * (glint / RENDER_PI);
    float3 color = body * (1.0 - F) + skyColor * F;
    float a = saturate(0.30 + 0.12 * saturate(speed) + 0.62 * F);

    // Foam, broken up and drifting down
    float cells = foamTexture(float2(uv.x * 9.0, (uv.y - 1.3 * speed * t) * 7.0));
    float f = foam * smoothstep(0.30, 0.62, cells * 0.75 + vnoise(float2(uv.x * 6.0, (uv.y - speed * t) * 4.0)) * 0.45);
    float3 foamColor = float3(0.85, 0.86, 0.85) * lightOn;
    color = lerp(color, foamColor, f);
    a = max(a, f * 0.92);
    sparkle *= 1.0 - f;

    // The edge, and the end at the sea
    float edge = 1.0 - smoothstep(hw * 0.8, hw + 0.3, abs(uv.x));
    float end = 1.0 - smoothstep(params.course.x - params.course.y, params.course.x, uv.y);
    a *= edge * end;
    sparkle *= edge * end;
    return float4((color * a + sparkle) * frame.exposure.x, a);
}
