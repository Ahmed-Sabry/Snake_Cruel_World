// Blackout shader with radial gradient
// Creates a candlelight/match illumination effect around the snake head
// Used for Level 2 (Lights Out) - can replace flat overlay approach
uniform sampler2D scene;
uniform vec2 resolution;
uniform vec2 headPos;     // Snake head position in pixels
uniform float glowRadius; // Radius of visible area in pixels
uniform float time;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    uv.y = 1.0 - uv.y;
    vec4 color = texture2D(scene, uv);

    // Distance from head in pixels
    vec2 pixelPos = gl_FragCoord.xy;
    pixelPos.y = resolution.y - pixelPos.y;
    float dist = length(pixelPos - headPos);

    // Noisy edge for flickering candlelight feel
    float noise = sin(pixelPos.x * 0.1 + time * 3.0) * cos(pixelPos.y * 0.1 + time * 2.0) * 8.0;
    float adjustedDist = dist + noise;

    // Smooth falloff
    float glow = 1.0 - smoothstep(glowRadius * 0.5, glowRadius, adjustedDist);

    // Warm orange tint at center
    vec3 warmTint = mix(color.rgb, color.rgb * vec3(1.1, 0.9, 0.7), glow * 0.3);

    // Darken outside glow
    float darkness = 1.0 - glow;
    vec3 result = warmTint * (1.0 - darkness * 0.95);

    gl_FragColor = vec4(result, color.a);
}
