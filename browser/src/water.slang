// The browser's water shader: the sea round the island, opaque, one grid of
// vertices (x and z only) at the sea's level, moved by a few summed Gerstner
// waves on the scene's time and coloured per fragment: the sea bed seen
// through the water and tinted by its depth (Beer-Lambert), the sky and sun
// reflected by the Fresnel term, ripples, and foam at the shoreline. The
// depth comes from the same r32float heightfield the terrain is drawn from
// (island.cone), so the water meets the land where the land meets the sea
// level, at the shoreline of the terrain's own triangles.
//
// GERSTNER WAVES (Finch, "Effective Water Simulation from Physical Models",
// GPU Gems ch. 1, equations 9 to 12): wave i has a direction D, a wavelength
// L, frequency k = 2 pi / L, an amplitude A and a steepness Q:
//   theta = k D . (x, z) - w t + phase,   w = sqrt(g k)   (deep water)
//   x, z += Q A D cos(theta)              y += A sin(theta)
//   N = (-sum D.x k A cos, 1 - sum Q k A sin, -sum D.z k A cos)
// Five waves, from 60 m to 4.5 m, within 35 degrees of a south-west wind; the
// two longest move the vertices (the grid is 8 m), all five the normal, which
// is worked out per fragment. Over shallows the amplitude and the steepness
// shrink with the depth, so the waves flatten onto the beach.
//
// Group 0 is render's frame; group 1 the material's: its parameters, the
// heights (r32float) and render's sampler (unused).

import renderframe;

struct Params {
    float4 grid;                   // x0, z0 of the heights' first sample, the cell's metres, the sea's level
    float4 extent;                 // rgb the sky's radiance overhead, which the sea reflects (nits)
};

[[vk::binding(0, 1)]] ConstantBuffer<Params> params;
[[vk::binding(1, 1)]] Texture2D<float> heights;
[[vk::binding(2, 1)]] SamplerState unused;

struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 world : TEXCOORD0;
    float2 rest : TEXCOORD1;       // where the vertex was before the waves moved it
    nointerpolation float4 tint : COLOR;
};

static const int WAVES = 5;
static const float G = 9.81;

float heightAt(int x, int z) {
    uint w;
    uint h;
    heights.GetDimensions(w, h);
    // Past the grid, the sea floor's 30 m
    if (x < 0 || z < 0 || x >= int(w) || z >= int(h)) {
        return -30.0;
    }
    return heights.Load(int3(x, z, 0));
}

float fieldHeight(float2 xz) {
    float2 g = (xz - params.grid.xy) / params.grid.z;
    float2 f = frac(g);
    int2 c = int2(floor(g));
    float a = heightAt(c.x, c.y);
    float b = heightAt(c.x + 1, c.y);
    float cc = heightAt(c.x, c.y + 1);
    float d = heightAt(c.x + 1, c.y + 1);
    return lerp(lerp(a, b, f.x), lerp(cc, d, f.x), f.y);
}

static const float WAVE_L[5] = { 60.0, 34.0, 14.0, 8.0, 4.5 };
static const float WAVE_A[5] = { 0.16, 0.10, 0.05, 0.028, 0.014 };
// Directions: the wind's (towards the north-east: x east, z south) turned
// by these angles, in radians
static const float WAVE_TURN[5] = { 0.0, 0.43, -0.35, 0.61, -0.56 };
static const float WAVE_PHASE[5] = { 0.0, 1.7, 4.1, 2.3, 5.5 };
static const float WIND = -0.9273;   // the direction's angle: (cos, sin) = (0.6, -0.8)

float2 waveDir(int i) {
    float a = WIND + WAVE_TURN[i];
    return float2(cos(a), sin(a));
}

// The sum of the waves at (x, z) at time t and sea depth 'depth' (m): the
// displacement (x, y, z) and the normal
void gerstner(float2 xz, float t, float depth, int count, float dist, out float3 disp, out float3 normal) {
    // A pixel at this distance spans about this many metres: a wave shorter
    // than a few of them would shimmer, so it fades out
    float pixel = max(dist, 1.0) * 0.0014;
    float shoal = clamp(depth / 2.5, 0.12, 1.0);
    float3 d = float3(0.0, 0.0, 0.0);
    float nx = 0.0;
    float nz = 0.0;
    float ny = 0.0;
    for (int i = 0; i < WAVES; i++) {
        if (i >= count) {
            break;
        }
        float k = 6.2831853 / WAVE_L[i];
        float w = sqrt(G * k);
        float2 D = waveDir(i);
        float vis = saturate((WAVE_L[i] / pixel - 2.0) / 4.0);
        float A = WAVE_A[i] * shoal * vis;
        float Q = 0.8 * shoal * vis;
        float th = k * dot(D, xz) - w * t + WAVE_PHASE[i];
        float c = cos(th);
        float s = sin(th);
        d.x += Q * A * D.x * c;
        d.z += Q * A * D.y * c;
        d.y += A * s;
        nx += D.x * k * A * c;
        nz += D.y * k * A * c;
        ny += Q * k * A * s;
    }
    disp = d;
    normal = normalize(float3(-nx, 1.0 - ny, -nz));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    float2 xz = input.position.xz;
    float depth = max(0.0, params.grid.w - fieldHeight(xz));
    // The two long waves move the vertices; none at the grid's edge, where
    // the far ring of the sea takes over flat
    float edge = 1.0 - smoothstep(0.82, 0.97, max(abs(xz.x), abs(xz.y)) / 1024.0);
    float3 disp;
    float3 nrm;
    gerstner(xz, frame.viewport.z, depth, 2, 0.0, disp, nrm);
    float3 pos = float3(xz.x + disp.x * edge, params.grid.w + disp.y * edge, xz.y + disp.z * edge);
    float4 world = mul(inst.model, float4(pos, 1.0));
    output.position = mul(frame.viewProjection, world);
    output.world = world.xyz;
    output.rest = xz;
    output.tint = inst.tint;
    return output;
}

// ---- Noise ----

// A hash of a lattice point (an integer-valued float2), by integer arithmetic
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

// The slope of a noise layer at p: two scrolling layers of ripples
float2 rippleSlope(float2 p) {
    float e = 0.08;
    float x = vnoise(p + float2(e, 0.0)) - vnoise(p - float2(e, 0.0));
    float z = vnoise(p + float2(0.0, e)) - vnoise(p - float2(0.0, e));
    return float2(x, z) / (2.0 * e);
}

float foamTexture(float2 p) {
    // Cells of foam: the nearer of a few jittered points, as a soft web
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
    float2 xz = input.rest;
    float t = frame.viewport.z;
    float sea = params.grid.w;
    float3 toEye = frame.cameraPosition.xyz - p;
    float dist = length(toEye);
    float3 V = toEye / dist;

    float h0 = fieldHeight(xz);
    float depth0 = max(0.0, sea - h0);

    // The waves' normal (all five), and the ripples on it, fading with distance
    float3 disp;
    float3 n;
    gerstner(xz, t, depth0, WAVES, dist, disp, n);
    // Ripples fade out by 300 m, where their slopes would shimmer
    float fadeR = saturate(1.0 - dist / 300.0);
    fadeR *= fadeR;
    float2 r1 = rippleSlope(xz / 3.5 + float2(0.04, 0.02) * t / 3.5 * 4.0);
    float2 r2 = rippleSlope(xz / 0.9 - float2(0.05, 0.03) * t / 0.9 * 4.0 + 11.0);
    n = normalize(n + float3(r1.x * 0.07 + r2.x * 0.03, 0.0, r1.y * 0.07 + r2.y * 0.03) * fadeR);

    // The bed, seen through the water: the refraction shifts where we look a little
    float2 seen = xz + n.xz * depth0 * 0.15;
    float bedH = fieldHeight(seen);
    float depth = max(0.0, sea - bedH);

    float3 L = frame.lightDirection.xyz;
    float3 skyLight = frame.ambient.rgb;
    float3 sunLight = frame.lightColor.rgb * (max(L.y, 0.0) / RENDER_PI);
    float3 lightOn = skyLight + sunLight;

    // The bed's own colour: sand near the shore, darker in the deep
    float3 bedAlbedo = lerp(float3(0.55, 0.43, 0.25), float3(0.12, 0.10, 0.07), smoothstep(0.5, 8.0, depth));
    float3 bed = bedAlbedo * lightOn * exp(-depth * 0.25);
    // Beer-Lambert: red goes first, in clear water
    float3 sigma = float3(0.85, 0.24, 0.14);
    float3 T = exp(-depth * sigma);
    float3 body = float3(0.005, 0.055, 0.070) * lightOn;
    float3 water = bed * T + body * (1.0 - T);

    // Reflection: Schlick's Fresnel, the sky's colour towards the reflected direction and the sun's glint
    float c = saturate(dot(n, V));
    float F = 0.02 + 0.98 * pow(1.0 - c, 5.0);
    float shore = smoothstep(6.0, 0.0, depth);
    F *= 1.0 - 0.7 * shore;
    float3 R = reflect(-V, n);
    R.y = abs(R.y);
    // The sky's colour: the frame's ambient light is the sky's mean radiance
    // at this hour (the day's sky, dusk, the night's), the sea mirroring
    // about twice that overhead
    float3 skyColor = skyLight * 2.0;
    float glint = pow(max(dot(R, L), 0.0), 1500.0) * 400.0;
    float3 reflected = skyColor + frame.lightColor.rgb * (glint / RENDER_PI);
    float3 color = water * (1.0 - F) + reflected * F;

    // Foam: along the waterline in a band that comes and goes with the swash,
    // and a few whitecaps on the biggest crests
    float thr = 0.28 + 0.16 * sin(6.2831853 * t / 6.5 + 5.0 * vnoise(xz * 0.2));
    float cells = foamTexture(xz * 1.6 + float2(t * 0.03, 0.0));
    float foam = smoothstep(thr, 0.0, depth) * smoothstep(0.35, 0.7, cells * 0.7 + vnoise(xz * 3.0) * 0.5);
    foam += smoothstep(0.03, 0.0, depth);
    foam = saturate(foam) * (1.0 - smoothstep(120.0, 400.0, dist));
    float3 foamColor = float3(0.85, 0.85, 0.85) * lightOn;
    color = lerp(color, foamColor, foam);

    return float4(color * frame.exposure.x, 1.0);
}
