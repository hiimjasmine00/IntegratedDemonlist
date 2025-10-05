#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform int colorMode;
uniform vec4 colors[4];
uniform sampler2D CC_Texture0;

void main() {
    if (colorMode == 0) {
        gl_FragColor = colors[0] * v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
        return;
    }

    float angle = radians(65.0);
    vec2 dir = vec2(cos(angle), sin(angle));

    float d0 = dot(vec2(0.0, 0.0), dir);
    float d1 = dot(vec2(1.0, 0.0), dir);
    float d2 = dot(vec2(0.0, 1.0), dir);
    float d3 = dot(vec2(1.0, 1.0), dir);

    float tMin = min(min(min(d0, d1), d2), d3);
    float tMax = max(max(max(d0, d1), d2), d3);

    float t = dot(v_texCoord, dir);
    t = (t - tMin) / (tMax - tMin);
    t = clamp(t, 0.0, 1.0);

    vec4 col = vec4(1.0);
    if (colorMode == 1) {
        if (t < 0.5) {
            col = mix(colors[0], colors[1], smoothstep(0.0, 0.5, t));
        }
        else {
            col = mix(colors[1], colors[0], smoothstep(0.5, 1.0, t));
        }
    }
    else if (colorMode == 2) {
        if (t < 0.45) {
            col = mix(colors[0], colors[1], smoothstep(0.0, 0.45, t));
        }
        else if (t < 0.5) {
            col = mix(colors[1], colors[2], smoothstep(0.45, 0.5, t));
        }
        else if (t < 0.55) {
            col = mix(colors[2], colors[3], smoothstep(0.5, 0.55, t));
        }
        else {
            col = mix(colors[3], colors[0], smoothstep(0.55, 1.0, t));
        }
    }

    gl_FragColor = col * v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
}
