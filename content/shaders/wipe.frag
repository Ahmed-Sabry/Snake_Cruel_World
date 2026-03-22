uniform sampler2D texture;
uniform float progress;    // 0.0 = no wipe, 1.0 = fully wiped
uniform vec2 direction;    // normalized wipe direction
uniform vec4 color;        // wipe color

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);

    // Compute wipe position along the direction axis
    // Map UV from [0,1] to [-1,1] for directional calculation
    float wipePos = dot(uv, abs(direction));

    // Adjust for direction sign (left/up wipes go in reverse)
    if (direction.x < 0.0 || direction.y < 0.0)
        wipePos = 1.0 - wipePos;

    // Soft edge width for ink-like feathering
    float edgeWidth = 0.05;
    float edge = smoothstep(progress - edgeWidth, progress + edgeWidth, wipePos);

    // Mix between scene and wipe color
    gl_FragColor = mix(color, texColor, edge);
}
