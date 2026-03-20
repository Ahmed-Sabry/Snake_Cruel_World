// Ink bleed / edge wobble shader
// Creates a "wet ink" bleeding effect via sin-wave UV displacement
// Used for levels 7+ (corruption > 0.5)
uniform sampler2D scene;
uniform vec2 resolution;
uniform float time;
uniform float wobbleAmount; // 0.0 to 2.0 pixels

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    // Offset UV by noise to create bleeding effect
    float nx = sin(uv.y * 50.0 + time * 2.0) * wobbleAmount / resolution.x;
    float ny = cos(uv.x * 50.0 + time * 1.5) * wobbleAmount / resolution.y;
    vec4 color = texture2D(scene, uv + vec2(nx, ny));

    gl_FragColor = color;
}
