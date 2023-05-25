#version 420 core

out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoords;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;

// uniform vec3 lightPosition;
// uniform vec2 lightColor;
// uniform vec3 camPos;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, vTexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(vWorldPos);
    vec3 Q2  = dFdy(vWorldPos);
    vec2 st1 = dFdx(vTexCoords);
    vec2 st2 = dFdy(vTexCoords);

    vec3 N   = normalize(vNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    vec3 albedo = pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2));
    // vec3 normal = texture(normalMap, vTexCoords).rgb;
    float metallic = texture(metallicMap, vTexCoords).r;
    float roughness = texture(roughnessMap, vTexCoords).r;
    vec3 n;
    n = getNormalFromMap();
    FragColor = vec4(vec3(albedo), 1.0);
}