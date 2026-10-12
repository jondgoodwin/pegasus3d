// The HUD's primitives: the draw list of uilayout (rectangles, rounded
// rectangles, borders, lines, arcs and the minimap's image) drawn into the
// frame after tone mapping, as one instanced quad each; the text over them is
// msdftext's. Compiled by tools/shaders/shaders.py and embedded in hud.cone.
//
// Every shape is its signed distance in screen pixels, antialiased by one
// pixel of coverage (the way uilayout's software rasteriser paints them, and
// the shape functions are the usual ones: Inigo Quilez's rounded box and
// segment distances).
//
//   box    x0 y0 x1 y1  the quad, a pixel or two beyond the shape
//   shape  rect, border, image: centre x, centre y, half width, half height
//          line: the two end points; arc: centre x, centre y, radius, start angle
//   color  straight alpha, sRGB (what uilayout's 0xRRGGBBAA says)
//   info   x kind (0 fill, 1 border, 2 line, 3 arc, 4 image), y corner radius,
//          z thickness, w the arc's end angle
//   uv     the image's texture window u0 v0 u1 v1 over the shape's box
//   clip   x0 y0 x1 y1 in pixels: nothing is drawn outside it
//
// The output is premultiplied (the pipeline blends ONE, ONE_MINUS_SRC_ALPHA);
// into an sRGB target it is converted to linear first, so that the colours
// come out as written.

struct Params {
    float4 view;                   // x, y: the target's size in pixels; z: 1 if the target is sRGB
};

[[vk::binding(0, 0)]] ConstantBuffer<Params> params;
[[vk::binding(1, 0)]] Texture2D mapTexture;
[[vk::binding(2, 0)]] SamplerState mapSampler;

struct Prim {
    float4 box : POSITION;
    float4 shape : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 info : TEXCOORD2;
    float4 uv : TEXCOORD3;
    float4 clip : TEXCOORD4;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 pixel : TEXCOORD0;
    nointerpolation float4 shape : TEXCOORD1;
    nointerpolation float4 color : TEXCOORD2;
    nointerpolation float4 info : TEXCOORD3;
    nointerpolation float4 uv : TEXCOORD4;
    nointerpolation float4 clip : TEXCOORD5;
    nointerpolation float4 box : TEXCOORD6;
};

float2 quadCorner(uint v) {
    float x = (v == 1u || v == 2u || v == 4u) ? 1.0 : 0.0;
    float y = (v == 2u || v == 4u || v == 5u) ? 1.0 : 0.0;
    return float2(x, y);
}

[shader("vertex")]
VertexOutput vertexMain(Prim prim, uint vid : SV_VulkanVertexID) {
    float2 c = quadCorner(vid);
    float2 p = float2(lerp(prim.box.x, prim.box.z, c.x), lerp(prim.box.y, prim.box.w, c.y));
    VertexOutput o;
    o.position = float4(p.x / params.view.x * 2.0 - 1.0, 1.0 - p.y / params.view.y * 2.0, 0.0, 1.0);
    o.pixel = p;
    o.shape = prim.shape;
    o.color = prim.color;
    o.info = prim.info;
    o.uv = prim.uv;
    o.clip = prim.clip;
    o.box = prim.box;
    return o;
}

// The distance from p to a rounded box (negative inside)
float roundedBox(float2 p, float2 centre, float2 half, float radius) {
    float r = min(radius, min(half.x, half.y));
    float2 q = abs(p - centre) - (half - float2(r, r));
    return length(max(q, float2(0.0, 0.0))) + min(max(q.x, q.y), 0.0) - r;
}

float3 srgbToLinear(float3 c) {
    float3 lo = c / 12.92;
    float3 hi = pow((c + float3(0.055, 0.055, 0.055)) / 1.055, float3(2.4, 2.4, 2.4));
    return float3(c.x <= 0.04045 ? lo.x : hi.x, c.y <= 0.04045 ? lo.y : hi.y, c.z <= 0.04045 ? lo.z : hi.z);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput i) : SV_Target {
    float2 p = i.pixel;
    if (p.x < i.clip.x || p.y < i.clip.y || p.x >= i.clip.z || p.y >= i.clip.w) { discard; }
    float kind = i.info.x;
    float radius = i.info.y;
    float thick = i.info.z;
    float cover = 0.0;
    float4 col = i.color;
    if (kind < 0.5) {
        float d = roundedBox(p, i.shape.xy, i.shape.zw, radius);
        cover = clamp(0.5 - d, 0.0, 1.0);
    } else if (kind < 1.5) {
        float d = roundedBox(p, i.shape.xy, i.shape.zw, radius);
        cover = clamp(0.5 - d, 0.0, 1.0) * clamp(0.5 + d + thick, 0.0, 1.0);
    } else if (kind < 2.5) {
        float2 a = i.shape.xy;
        float2 e = i.shape.zw;
        float2 pa = p - a;
        float2 ba = e - a;
        float h = clamp(dot(pa, ba) / max(dot(ba, ba), 0.0001), 0.0, 1.0);
        float d = length(pa - ba * h) - thick * 0.5;
        cover = clamp(0.5 - d, 0.0, 1.0);
    } else if (kind < 3.5) {
        float2 q = p - i.shape.xy;
        float dist = length(q);
        float tau = 6.2831855;
        float a0 = i.shape.w;
        float sweep = i.info.w - a0;
        bool inside = true;
        if (sweep < tau - 0.001) {
            float a = atan2(q.y, q.x) - a0;
            a = a - tau * floor(a / tau);
            inside = a <= sweep;
        }
        if (inside) {
            if (thick <= 0.0) { cover = clamp(i.shape.z + 0.5 - dist, 0.0, 1.0); }
            else { cover = clamp(thick * 0.5 + 0.5 - abs(dist - i.shape.z), 0.0, 1.0); }
        }
    } else {
        float d = roundedBox(p, i.shape.xy, i.shape.zw, radius);
        cover = clamp(0.5 - d, 0.0, 1.0);
        float2 f = (p - i.box.xy) / max(i.box.zw - i.box.xy, float2(1.0, 1.0));
        // (the box is the shape's, grown by a pixel and a half; the uv window is the shape's)
        float2 size = i.shape.zw * 2.0;
        float2 top = i.shape.xy - i.shape.zw;
        float2 g = (p - top) / max(size, float2(1.0, 1.0));
        float2 uv = lerp(i.uv.xy, i.uv.zw, g);
        float4 t = mapTexture.SampleLevel(mapSampler, uv, 0.0);
        col = float4(t.rgb * i.color.rgb, i.color.a);
    }
    float a = col.a * cover;
    float3 rgb = col.rgb;
    if (params.view.z > 0.5) { rgb = srgbToLinear(rgb); }
    return float4(rgb * a, a);
}
