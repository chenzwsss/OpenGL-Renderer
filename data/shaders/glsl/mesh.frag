#version 420 core

out vec4 outFragColor;

in FragData {
    vec3 vWorldPos;
    vec3 vNormal;
    vec2 vTexCoords;
    // Noperspective so the interpolation is in screen-space
    noperspective vec3 wireframeDist;
} fragData;

uniform sampler2D albedoMap;

uniform int render_wireframe;

void main() {

    vec2 uv = fragData.vTexCoords;

    vec4 color = texture(albedoMap, uv);//pow(texture(albedoMap, uv).rgb, vec2(2.2));

    // Wireframe
    if (render_wireframe > 0) {
        vec3 d = fwidth(fragData.wireframeDist);
        vec3 a3 = smoothstep(vec3(0.0), d * 1.5, fragData.wireframeDist);
        float edgeFactor = min(min(a3.x, a3.y), a3.z);
        color.rgb = mix(vec3(1.0), color.rgb, vec3(edgeFactor));
    }

    outFragColor = vec4(color);
}