// Chromatic aberration shader
// Splits RGB channels along radial direction from center
// Used for Level 9 (Amnesia) and Level 10 Phase 4
uniform sampler2D scene;
uniform vec2 resolution;
uniform float amount; // 1.0 to 3.0 pixels

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    vec2 dir = uv - vec2(0.5);
    float dist = length(dir);
    vec2 offset = dir * dist * amount / resolution;

    float r = texture2D(scene, uv + offset).r;
    float g = texture2D(scene, uv).g;
    float b = texture2D(scene, uv - offset).b;
    float a = texture2D(scene, uv).a;

    gl_FragColor = vec4(r, g, b, a);
}
