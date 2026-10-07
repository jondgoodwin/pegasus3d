// The browser's terrain shader: the island's ground (src/island/island.cone),
// a grid mesh whose vertices carry only their x and z, lifted in the vertex
// stage by an r32float heightfield, and coloured per fragment: sand by the
// water, grass, rock on the steep slopes, forest litter and the path's packed
// earth, blended by height, slope and a painted mask, with procedural noise
// for every texture (nothing is an image but the mask).
//
// Two meshes use it, the same field: a fine one (a vertex a sample, 2 m) over
// the walkable country and the hill's foot, and a coarse one (every 4th
// sample) over the whole island with a hole where the fine one is. The
// fine mesh's border vertices that the coarse mesh has no vertex at are
// set to the line between the coarse ones, so the two meet with no crack.
// Inside the fine mesh the drawn triangles are exactly the walker's
// (island.cone's groundHeight): the quads are split by one rule,
// ((i + j) odd flips the diagonal), which the index buffers use too.
//
// Lit by render's one convention (the sun's colour an irradiance, so diffuse
// is albedo / pi times it; the ambient light a radiance), times the exposure.
//
// Compiled ahead of time by Cone's tools/shaders/shaders.py, run over this
// folder, and embedded in islandshaders.cone.
//
// Group 0 is render's frame; group 1 the material's: its parameters, the
// heights (r32float, read texel by texel), the mask (RGBA8, linear: red the
// forest litter, green the distance to the path over 16 m) and render's sampler.

import renderframe;

struct Params {
    float4 grid;                   // x0, z0 (the grid's first sample), the cell's metres, spare
    float4 inner;                  // the fine mesh's box in samples: i0, j0, i1, j1
};

[[vk::binding(0, 1)]] ConstantBuffer<Params> params;
[[vk::binding(1, 1)]] Texture2D<float> heights;
[[vk::binding(2, 1)]] Texture2D<float> pathDist;
[[vk::binding(3, 1)]] Texture2D<float4> maskTex;
[[vk::binding(4, 1)]] SamplerState smp;

struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 world : TEXCOORD0;
    nointerpolation float4 tint : COLOR;
};

float heightAt(int x, int z) {
    uint w;
    uint h;
    heights.GetDimensions(w, h);
    return heights.Load(int3(clamp(x, 0, int(w) - 1), clamp(z, 0, int(h) - 1), 0));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    Instance inst = instances[id];
    int i = int(round((input.position.x - params.grid.x) / params.grid.z));
    int j = int(round((input.position.z - params.grid.y) / params.grid.z));
    float y = heightAt(i, j);
    {
        // Only a fine vertex between the coarse mesh's can be off a multiple of 4
        int i0 = int(params.inner.x);
        int j0 = int(params.inner.y);
        int i1 = int(params.inner.z);
        int j1 = int(params.inner.w);
        // On the box's edge, between the coarse mesh's vertices: their line
        if ((j == j0 || j == j1) && (i % 4) != 0) {
            int a = (i / 4) * 4;
            y = lerp(heightAt(a, j), heightAt(a + 4, j), float(i - a) * 0.25);
        } else if ((i == i0 || i == i1) && (j % 4) != 0) {
            int a = (j / 4) * 4;
            y = lerp(heightAt(i, a), heightAt(i, a + 4), float(j - a) * 0.25);
        }
    }
    float4 world = mul(inst.model, float4(input.position.x, y, input.position.z, 1.0));
    output.position = mul(frame.viewProjection, world);
    output.world = world.xyz;
    output.tint = inst.tint;
    return output;
}

// ---- Noise ----

// A hash of a lattice point (an integer-valued float2), by integer arithmetic:
// the same bits for the same point from whichever cell asks, whatever the
// float rounding of the position it was found from
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

float fbm4(float2 p) {
    float s = 0.0;
    float a = 0.5;
    for (int o = 0; o < 4; o++) {
        s += a * vnoise(p);
        p = p * 2.03 + float2(17.1, 9.2);
        a *= 0.5;
    }
    return s * 1.07;
}

// ---- The field, smoothly (for the normal and the layers) ----

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

// The distance to the path (a field of the same grid), smoothly
float pathAt(float2 xz) {
    uint w;
    uint h;
    pathDist.GetDimensions(w, h);
    float2 g = (xz - params.grid.xy) / params.grid.z;
    float2 f = frac(g);
    int2 c = int2(floor(g));
    int2 hi = int2(int(w) - 1, int(h) - 1);
    float a = pathDist.Load(int3(clamp(c, int2(0, 0), hi), 0));
    float b = pathDist.Load(int3(clamp(c + int2(1, 0), int2(0, 0), hi), 0));
    float cc = pathDist.Load(int3(clamp(c + int2(0, 1), int2(0, 0), hi), 0));
    float d = pathDist.Load(int3(clamp(c + int2(1, 1), int2(0, 0), hi), 0));
    return lerp(lerp(a, b, f.x), lerp(cc, d, f.x), f.y);
}

float3 fieldNormal(float2 xz) {
    // Differences across four cells: the field is piecewise linear, so its
    // slope jumps at every cell line, and a nearer pair would show them
    float e = params.grid.z * 2.0;
    float hx = fieldHeight(xz + float2(e, 0.0)) - fieldHeight(xz - float2(e, 0.0));
    float hz = fieldHeight(xz + float2(0.0, e)) - fieldHeight(xz - float2(0.0, e));
    return normalize(float3(-hx, 2.0 * e, -hz));
}

// ---- The layers ----

static const float3 SAND = float3(0.60, 0.47, 0.27);
static const float3 GRASS_A = float3(0.085, 0.20, 0.030);
static const float3 GRASS_B = float3(0.15, 0.26, 0.045);
static const float3 SOIL = float3(0.050, 0.040, 0.022);
static const float3 LEAF = float3(0.17, 0.085, 0.03);
static const float3 ROCK_A = float3(0.13, 0.12, 0.105);
static const float3 ROCK_B = float3(0.23, 0.20, 0.16);
static const float3 EARTH = float3(0.24, 0.15, 0.085);
static const float PATH_HALF = 0.9;

float3 rockColor(float3 p, float3 n) {
    // Three planes of noise blended by the normal (a triplanar of procedural
    // textures): strata across the vertical ones
    float3 w = pow(abs(n), float3(4.0, 4.0, 4.0));
    w /= (w.x + w.y + w.z);
    float cx = fbm4(p.yz * 0.9) * w.x;
    float cy = fbm4(p.xz * 0.9) * w.y;
    float cz = fbm4(p.xy * 0.9) * w.z;
    float rough = cx + cy + cz;
    float strata = 0.5 + 0.5 * sin(p.y * 3.1 + fbm4(p.xz * 0.25) * 6.0);
    float t = saturate(rough * 0.9 + strata * 0.35 - 0.15);
    return lerp(ROCK_A, ROCK_B, t);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float3 p = input.world;
    float2 xz = p.xz;
    float h = fieldHeight(xz);
    float3 n = fieldNormal(xz);
    float t = frame.viewport.z;

    uint mw;
    uint mh;
    heights.GetDimensions(mw, mh);
    float2 uv = ((xz - params.grid.xy) / params.grid.z + 0.5) / float2(float(mw), float(mh));
    float4 mask = maskTex.SampleLevel(smp, uv, 0.0);
    float litter = mask.r;
    float pathD = pathAt(xz);
    float dist = length(frame.cameraPosition.xyz - p);
    float near = saturate(1.0 - dist / 80.0);

    float macro = fbm4(xz * 0.018);
    float sandBreak = (fbm4(xz * 0.07) - 0.5) * 1.4;
    float wSand = smoothstep(2.7, 1.3, h + sandBreak);
    float rockBreak = (fbm4(xz * 0.05 + 4.0) - 0.5) * 0.12;
    float wRock = smoothstep(0.86, 0.74, n.y + rockBreak);
    float wLitter = litter * 0.72 * (1.0 - wSand) * (1.0 - wRock);
    float wGrass = (1.0 - wSand) * (1.0 - wRock) * (1.0 - wLitter);
    wSand *= (1.0 - wRock);

    // Sand: ripples across it and grain, darker where the swash wets it
    float swash = 0.30 + 0.20 * sin(6.2831853 * t / 7.0 + 6.0 * vnoise(xz * 0.15));
    float wet = smoothstep(swash + 0.30, swash - 0.05, h);
    float grain = vnoise(xz * 38.0) * 0.5 + vnoise(xz * 9.0) * 0.5;
    float ripple = vnoise(float2(xz.x * 2.0 + vnoise(xz * 0.5) * 6.0, xz.y * 0.35));
    float3 sand = SAND * (0.82 + 0.22 * ripple + 0.2 * (grain - 0.5) * near + 0.15 * (macro - 0.5));
    sand *= lerp(1.0, 0.55, wet);

    // Grass: two greens by the large patches, fine blades, a few dry stalks
    float blades = vnoise(xz * 22.0) * 0.6 + vnoise(xz * 63.0) * 0.4;
    // Patches of lusher and drier grass at two scales (about 50 m and 8 m)
    float mottle = fbm4(xz * 0.13 + 31.0);
    float3 grass = lerp(GRASS_A, GRASS_B, smoothstep(0.3, 0.7, macro * 0.6 + mottle * 0.4));
    grass *= 0.85 + 0.3 * mottle;
    grass *= 0.72 + 0.55 * lerp(0.5, blades, near);
    // Higher up the grass is thinner and darker
    grass *= lerp(1.0, 0.62, smoothstep(20.0, 120.0, h));
    grass = lerp(grass, grass * float3(1.5, 1.2, 0.6), step(0.93, vnoise(xz * 7.0)) * 0.6);

    // Forest litter: soil with leaves lying on it, in clumps
    float leafMask = smoothstep(0.58, 0.70, vnoise(xz * 17.0 + 3.3) * 0.6 + vnoise(xz * 5.1) * 0.4);
    float3 litterCol = lerp(SOIL * (0.7 + 0.6 * vnoise(xz * 11.0)), LEAF * (0.6 + 0.8 * vnoise(xz * 29.0)), leafMask * near);
    // Undergrowth: grass comes through the leaf mould, more in the distance
    litterCol = lerp(litterCol, grass * 0.55, 0.3 + 0.4 * (1.0 - near));

    float3 rock = rockColor(p, n);

    // The path: packed earth, pebbles, an uneven edge
    float edge = pathD + (fbm4(xz * 1.6) - 0.5) * 0.5;
    float wPath = smoothstep(PATH_HALF + 0.35, PATH_HALF - 0.2, edge);
    float pebble = smoothstep(0.78, 0.86, vnoise(xz * 26.0 + 9.0));
    float3 earth = EARTH * (0.8 + 0.4 * vnoise(xz * 14.0)) + pebble * 0.07 * near;
    // The edge's shoulder: a little darker where the verge meets it
    earth *= 1.0 - 0.25 * smoothstep(PATH_HALF + 0.9, PATH_HALF, edge) * (1.0 - wPath);

    float3 albedo = sand * wSand + grass * wGrass + litterCol * wLitter + rock * wRock;
    albedo = lerp(albedo, earth, wPath);
    albedo *= input.tint.rgb;    // A touch of bump from the fine noise, near the eye
    float bx = vnoise(xz * 9.0 + float2(0.7, 0.0)) - vnoise(xz * 9.0 - float2(0.7, 0.0));
    float bz = vnoise(xz * 9.0 + float2(0.0, 0.7)) - vnoise(xz * 9.0 - float2(0.0, 0.7));
    float3 nn = normalize(n + float3(bx, 0.0, bz) * 0.18 * near);

    float diffuse = max(dot(nn, frame.lightDirection.xyz), 0.0);
    // Ambient: more from above than from the side
    float3 amb = frame.ambient.rgb * (0.65 + 0.35 * nn.y);
    float3 color = albedo * (amb + frame.lightColor.rgb * (diffuse / RENDER_PI));
    return float4(color * frame.exposure.x, 1.0);
}
