uniform sampler2D texture;
uniform float progress;    // 0.0 = fully open, 1.0 = fully closed
uniform vec2 resolution;
uniform vec4 color;        // iris color (typically black)

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);

    // Center of the iris
    vec2 center = vec2(0.5, 0.5);

    // Aspect-corrected distance from center
    vec2 aspect = vec2(resolution.x / resolution.y, 1.0);
    float dist = length((uv - center) * aspect);

    // Maximum radius (corner distance)
    float maxRadius = length(vec2(0.5, 0.5) * aspect);

    // Iris radius shrinks as progress increases
    float radius = maxRadius * (1.0 - progress);

    // Ink-style wobbly edge
    float angle = atan(uv.y - center.y, uv.x - center.x);
    float wobble = sin(angle * 8.0) * 0.015 + sin(angle * 13.0) * 0.01;
    radius += wobble * maxRadius * progress;

    // Soft edge
    float edgeWidth = 0.02 * maxRadius;
    float mask = smoothstep(radius - edgeWidth, radius + edgeWidth, dist);

    gl_FragColor = mix(texColor, color, mask);
}
