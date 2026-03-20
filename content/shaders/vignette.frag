// Vignette shader - darkens edges of screen
// Strength and radius parameterized per-level
uniform sampler2D scene;
uniform vec2 resolution;
uniform float vignetteStrength; // 0.3 to 0.8
uniform float vignetteRadius;   // 0.7 to 0.4

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    uv.y = 1.0 - uv.y;
    vec4 color = texture2D(scene, uv);

    vec2 center = vec2(0.5);
    float dist = distance(uv, center);
    float vignette = smoothstep(vignetteRadius, vignetteRadius + 0.3, dist);
    color.rgb *= 1.0 - vignette * vignetteStrength;

    gl_FragColor = color;
}
