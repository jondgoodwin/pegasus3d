// The browser's sky: one full-screen draw at the far plane, its colour from
// the analyticdaylight package's sky (Preetham, twilight, the night's floor,
// the sun's disc, the moon's and the stars; import analyticdaylight) with two
// layers of cloud over it, lit by the sun's colour (so they redden at dusk)
// and by the sky's ambient light. It is a custom pipeline (render's
// custom.cone) drawn from the 'skyrig.cone' module; the mesh it is given is
// mesh's plane (-1 to 1 in x and z), whose x and z are the screen's.
//
// THE CLOUDS (the browser's own choices, so they are here and not in the
// package): the low layer a flat plane 1800 m up, its coverage the cloud
// amount, a domain-warped fractal of value noise, thresholded to a soft
// edge, shaded by whether there is more cloud towards the sun than here
// (the sun side's shadow), with a silver lining where it is thin near the sun;
// the high layer (cirrus, 7500 m) the same stretched and thin. Both drift
// with the scene's time. They are blended inside this shader, over the sky,
// so no blending state is wanted. Where the sun is, they take its colour
// from the CPU ('cloudLight': the sun's irradiance through the high air over
// pi, plus the moon's).
//
// Group 0 is render's frame; group 1 the material's: its parameters (the
// package's DaySkyParams, then the clouds'), and render's sampler (unused).
// Compiled by tools/shaders/shaders.py and embedded in skyrig.cone.

import renderframe;
import analyticdaylight;

struct Params {
    DaySkyParams sky;
    float4 clouds;       // x low layer's cover 0..1, y high layer's, z scene time, seconds
    float4 cloudLight;   // rgb: a white cloud's radiance from the sun and the moon, nits
};

[[vk::binding(0, 1)]] ConstantBuffer<Params> params;
[[vk::binding(1, 1)]] SamplerState unused;

struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 ndc : TEXCOORD0;
};

// The plane's x and z are the screen's, at the far plane
[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint id : SV_VulkanInstanceID) {
    VertexOutput output;
    output.ndc = float2(input.position.x, input.position.z);
    output.position = float4(input.position.x, input.position.z, 0.9999995, 1.0);
    return output;
}

float hash21(int2 p) {
    uint x = uint(p.x) * 1597334677u ^ uint(p.y) * 3812015801u;
    x = (x ^ (x >> 16u)) * 2246822519u;
    x = (x ^ (x >> 13u)) * 3266489917u;
    x = x ^ (x >> 16u);
    return float(x >> 8u) * (1.0 / 16777216.0);
}

float vnoise(float2 p) {
    int2 i = int2(floor(p));
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    return lerp(lerp(hash21(i), hash21(i + int2(1, 0)), f.x), lerp(hash21(i + int2(0, 1)), hash21(i + int2(1, 1)), f.x), f.y);
}

float fbm(float2 p, int octaves) {
    float sum = 0.0;
    float amp = 0.5;
    for (int i = 0; i < octaves; i++) {
        sum += amp * vnoise(p);
        p = p * 2.03 + float2(17.1, 9.2);
        amp *= 0.5;
    }
    return sum;
}

// The low layer's density at plane coordinates 'uv': a fractal warped by a
// second, thresholded to a soft edge; 'cover' 0 (none) to 1 (overcast)
float lowDensity(float2 uv, float cover) {
    float c1 = fbm(uv, 5);
    float c2 = fbm(uv * 3.7 + 1.7 * c1, 4);
    float n = 0.65 * c1 + 0.35 * c2;
    float t0 = 0.72 - 0.5 * cover;
    return smoothstep(t0, t0 + 0.16, n);
}

// The high layer's: stretched along the wind and thin
float highDensity(float2 uv, float cover) {
    float n = fbm(float2(uv.x * 0.35, uv.y * 1.4) + float2(31.0, 5.0), 4);
    float t0 = 0.74 - 0.5 * cover;
    return smoothstep(t0, t0 + 0.2, n);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    // The direction this pixel looks in
    float4 farPoint = mul(frame.inverseViewProjection, float4(input.ndc, 1.0, 1.0));
    float3 dir = normalize(farPoint.xyz / farPoint.w - frame.cameraPosition.xyz);
    float pixel = length(fwidth(dir));
    DaySkyParams sky = params.sky;
    float3 color = daySkyRadiance(dir, sky);
    float time = params.clouds.z;

    // Clouds first, so the sun, moon and stars can be put behind them
    float lowAlpha = 0.0;
    float3 lowColor = float3(0.0, 0.0, 0.0);
    float highAlpha = 0.0;
    float3 highColor = float3(0.0, 0.0, 0.0);
    float3 sunDirXZ = float3(sky.sun.x, 0.0, sky.sun.z);
    float2 sunPlane = normalize(sunDirXZ.xz + float2(0.0001, 0.0));
    float3 ambientC = frame.ambient.rgb * 0.8;
    float3 light = params.cloudLight.rgb;
    if (dir.y > 0.005) {
        // The low layer, 1800 m up, drifting east-north-east at about 8 m/s
        float2 plane = (frame.cameraPosition.xz + dir.xz * (1800.0 / dir.y)) / 2500.0 + float2(1.0, 0.3) * (time * 0.0032);
        float dens = lowDensity(plane, params.clouds.x);
        float towardSun = lowDensity(plane + sunPlane * 0.03, params.clouds.x);
        float shadow = saturate((towardSun - dens) * 2.0 + dens * 0.3);
        float cosSun = max(dot(dir, sky.sun.xyz), 0.0);
        float3 lit = light * 0.95 + ambientC;
        float3 dark = ambientC * 0.5;
        lowColor = lerp(lit, dark, shadow) + light * (pow(1.0 - dens, 4.0) * pow(cosSun, 8.0) * 0.8);
        lowAlpha = dens * 0.96 * smoothstep(0.0, 0.14, dir.y);
        // The high layer, 7500 m up, faster
        float2 hp = (frame.cameraPosition.xz + dir.xz * (7500.0 / dir.y)) / 9000.0 + float2(1.0, 0.2) * (time * 0.0045);
        float hd = highDensity(hp, params.clouds.y);
        highColor = light * 1.0 + ambientC * 0.6;
        highAlpha = hd * 0.38 * smoothstep(0.0, 0.18, dir.y);
    }

    // Stars, the sun and the moon, over the sky and behind the clouds
    float cloudAny = saturate(lowAlpha + highAlpha);
    // The stars and the moon are set to read the same however the exposure
    // is: the brightest star at 0.9 of the display's white and the moon's
    // disc at 2.5 (so its seas show), unless the physical radiance is less
    // (the moon at 2500 nits is far brighter than any night's exposure shows
    // detail in; this is the exposure for the ground, not for the moon)
    float exposure = max(frame.exposure.x, 1e-9);
    color += starRadiance(dir, sky, pixel, time, 0.9 / exposure) * (1.0 - cloudAny);
    color += sunDiscRadiance(dir, sky);
    color += moonDiscRadiance(dir, sky) * min(1.0, 2.5 / (max(sky.moonDisc.g, 1.0) * exposure));
    color = lerp(color, highColor, highAlpha);
    color = lerp(color, lowColor, lowAlpha);
    return float4(min(color * frame.exposure.x, 200.0), 1.0);
}
