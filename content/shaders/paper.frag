// Paper texture overlay shader
// Multiply-blends tileable paper noise onto the scene for fiber texture
uniform sampler2D scene;
uniform sampler2D paperTex;
uniform float paperOpacity; // 0.05 to 0.15
uniform vec2 resolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    uv.y = 1.0 - uv.y; // Flip Y for SFML coordinate system
    vec4 sceneColor = texture2D(scene, uv);

    // Tile paper texture at 256px intervals
    vec2 paperUV = gl_FragCoord.xy / 256.0;
    vec4 paper = texture2D(paperTex, paperUV);

    // Multiply blend: slightly darken scene where paper has fibers
    vec3 result = sceneColor.rgb * mix(vec3(1.0), paper.rgb, paperOpacity);
    gl_FragColor = vec4(result, sceneColor.a);
}
