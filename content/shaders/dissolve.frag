uniform sampler2D texture;
uniform float progress;    // 0.0 = no dissolve, 1.0 = fully dissolved
uniform vec2 resolution;
uniform vec4 color;        // dissolve-to color

// Simple hash for procedural noise (no texture needed)
float hash(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Value noise
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f); // smoothstep

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);

    // Multi-octave noise for ink-splatter feel
    vec2 noiseCoord = uv * 12.0;
    float n = noise(noiseCoord) * 0.6
            + noise(noiseCoord * 2.0) * 0.3
            + noise(noiseCoord * 4.0) * 0.1;

    // Edge softness
    float edgeWidth = 0.08;
    float threshold = progress * (1.0 + edgeWidth * 2.0) - edgeWidth;
    float mask = smoothstep(threshold - edgeWidth, threshold + edgeWidth, n);

    gl_FragColor = mix(color, texColor, mask);
}
