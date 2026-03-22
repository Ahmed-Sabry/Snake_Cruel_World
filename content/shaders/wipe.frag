uniform sampler2D texture;
uniform float progress;    // 0.0 = no wipe, 1.0 = fully wiped
uniform vec2 direction;    // normalized wipe direction
uniform vec4 color;        // wipe color

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);

    // Compute wipe position along the direction axis, reversing each axis independently
    float xContrib = (direction.x < 0.0) ? (1.0 - uv.x) * abs(direction.x) : uv.x * abs(direction.x);
    float yContrib = (direction.y < 0.0) ? (1.0 - uv.y) * abs(direction.y) : uv.y * abs(direction.y);
    float wipePos = xContrib + yContrib;

    // Soft edge width for ink-like feathering
    float edgeWidth = 0.05;
    float edge = smoothstep(progress - edgeWidth, progress + edgeWidth, wipePos);

    // Mix between scene and wipe color
    gl_FragColor = mix(color, texColor, edge);
}
