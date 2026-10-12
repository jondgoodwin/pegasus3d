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
// the high layer (cirrus, 7500 m) the same stretched and thin. Both are
// carried east by the wind and their shapes morph, as closed forms in the
// scene's time (below). THE WEATHER (parts/weather.cone, packed by
// skyrig.cone) sets their cover, density and light, how grey and how bright
// the sky is left, the sun's and the moon's showing, and a haze over the
// horizon. They are blended inside this shader, over the sky,
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
    float4 weatherA;     // x the clouds' density, y the light they are lit with, z how grey the sky is, w how bright it is left
    float4 weatherB;     // x the horizon haze's strength 0..1, y the sun's disc, z the stars and moon (how much shows through)
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

// THE WIND (weather.cone's WIND_LOW and WIND_HIGH, the same numbers): the
// pattern is carried along +x, towards the east, at 7 m/s at the low layer's
// 1800 m (its plane's unit is 2500 m) and 11 m/s at the cirrus's 7500 m (a
// unit of 9000 m). A closed form in the scene's time.
static const float LOW_DRIFT = 7.0 / 2500.0;
static const float HIGH_DRIFT = 11.0 / 9000.0;

// THE SHAPE MORPH: each octave of the fractal is also carried round a small
// circle of its own (half a cell across, slower for the coarse octaves, a
// period of nine minutes down to two), so the clouds grow and thin and break
// up as they go and are not a rigid picture sliding by. Also a closed form in
// the scene's time.
float fbm(float2 p, int octaves, float time) {
    float sum = 0.0;
    float amp = 0.5;
    for (int i = 0; i < octaves; i++) {
        float rate = 0.011 * (1.0 + 0.55 * float(i));
        float2 shift = 0.5 * float2(sin(time * rate + 1.7 * float(i)), cos(time * rate * 0.83 + 2.3 * float(i)));
        sum += amp * vnoise(p + shift);
        p = p * 2.03 + float2(17.1, 9.2);
        amp *= 0.5;
    }
    return sum;
}

// The low layer's density at plane coordinates 'uv': a fractal warped by a
// second, thresholded to a soft edge; 'cover' 0 (none) to 1 (overcast)
float lowDensity(float2 uv, float cover, float time) {
    float c1 = fbm(uv, 5, time);
    float c2 = fbm(uv * 3.7 + 1.7 * c1, 4, time);
    float n = 0.65 * c1 + 0.35 * c2;
    float t0 = 0.72 - 0.5 * cover;
    return smoothstep(t0, t0 + 0.16, n);
}

// The high layer's: stretched along the wind and thin
float highDensity(float2 uv, float cover, float time) {
    float n = fbm(float2(uv.x * 0.35, uv.y * 1.4) + float2(31.0, 5.0), 4, time);
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
    float time = params.clouds.z;
    float density = params.weatherA.x;
    float lightScale = params.weatherA.y;
    float grey = params.weatherA.z;
    float dim = params.weatherA.w;
    // The weather drains the blue sky towards a cool grey and leaves it dimmer;
    // in fair weather (grey 0, dim 1) this changes nothing
    float3 color = daySkyRadiance(dir, sky);
    color = lerp(color, dot(color, float3(0.2126, 0.7152, 0.0722)) * float3(0.97, 0.99, 1.03), grey) * dim;
    // The haze the horizon wears: the sky's colour just above the horizon,
    // greyed the same way, for the ground fog to be drawn in
    float3 hazeC = daySkyRadiance(normalize(float3(dir.x, 0.03, dir.z)), sky);
    hazeC = lerp(hazeC, dot(hazeC, float3(0.2126, 0.7152, 0.0722)) * float3(0.97, 0.99, 1.03), grey) * dim;
    float hazeMix = params.weatherB.x * exp(-max(dir.y, 0.0) * 5.0);

    // Clouds first, so the sun, moon and stars can be put behind them
    float lowAlpha = 0.0;
    float3 lowColor = float3(0.0, 0.0, 0.0);
    float highAlpha = 0.0;
    float3 highColor = float3(0.0, 0.0, 0.0);
    float3 sunDirXZ = float3(sky.sun.x, 0.0, sky.sun.z);
    float2 sunPlane = normalize(sunDirXZ.xz + float2(0.0001, 0.0));
    float3 ambientC = frame.ambient.rgb * 0.8;
    float3 light = params.cloudLight.rgb * lightScale;
    if (dir.y > 0.005) {
        // The low layer, 1800 m up, carried east (+x) by the wind at 7 m/s
        float2 plane = (frame.cameraPosition.xz + dir.xz * (1800.0 / dir.y)) / 2500.0 - float2(1.0, 0.0) * (time * LOW_DRIFT);
        float dens = lowDensity(plane, params.clouds.x, time);
        float towardSun = lowDensity(plane + sunPlane * 0.03, params.clouds.x, time);
        // (under a grey sky the light is even and the clouds are not shaded)
        float shadow = saturate((towardSun - dens) * 2.0 + dens * 0.3) * (1.0 - 0.7 * grey);
        float cosSun = max(dot(dir, sky.sun.xyz), 0.0);
        float3 lit = light * 0.95 + ambientC;
        float3 dark = ambientC * 0.5;
        lowColor = lerp(lit, dark, shadow) + light * (pow(1.0 - dens, 4.0) * pow(cosSun, 8.0) * 0.8);
        lowAlpha = saturate(dens * 0.96 * density) * smoothstep(0.0, 0.14, dir.y);
        // The high layer, 7500 m up, carried faster, 11 m/s
        float2 hp = (frame.cameraPosition.xz + dir.xz * (7500.0 / dir.y)) / 9000.0 - float2(1.0, 0.0) * (time * HIGH_DRIFT);
        float hd = highDensity(hp, params.clouds.y, time);
        highColor = light * 1.0 + ambientC * 0.6;
        highAlpha = saturate(hd * 0.38 * density) * smoothstep(0.0, 0.18, dir.y);
    }

    // Stars, the sun and the moon, over the sky and behind the clouds
    float cloudAny = saturate(lowAlpha + highAlpha);
    // The stars and the moon are set to read the same however the exposure
    // is: the brightest star at 0.9 of the display's white and the moon's
    // disc at 2.5 (so its seas show), unless the physical radiance is less
    // (the moon at 2500 nits is far brighter than any night's exposure shows
    // detail in; this is the exposure for the ground, not for the moon)
    float exposure = max(frame.exposure.x, 1e-9);
    color += starRadiance(dir, sky, pixel, time, 0.9 / exposure) * (1.0 - cloudAny) * params.weatherB.z;
    color += sunDiscRadiance(dir, sky) * params.weatherB.y;
    color += moonDiscRadiance(dir, sky) * min(1.0, 2.5 / (max(sky.moonDisc.g, 1.0) * exposure)) * params.weatherB.z;
    color = lerp(color, highColor, highAlpha);
    color = lerp(color, lowColor, lowAlpha);
    // The horizon haze, over all of it
    color = lerp(color, hazeC, hazeMix);
    return float4(min(color * frame.exposure.x, 200.0), 1.0);
}
