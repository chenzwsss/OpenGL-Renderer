#version 420 core

out vec4 FragColor;

layout (std140, binding = 1) uniform MaterialParams {
    float metallic_factor;
    float roughness_factor;
    int metallic_texture_set;
    int normal_texture_set;
    int roughness_texture_set;
};

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoords;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;

uniform vec4 lightDir;
uniform vec3 lightColor;
uniform vec3 camPos;

const float M_PI = 3.14159265359;

#include "shaders/pbr_functions.glsl"

void main() {

    vec3 albedo = pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2)); // sRGB need to gamma correction

    vec3 n;
    if (normal_texture_set > 0) {
        n = getNormalFromMap();   
    } else {
        n = vNormal;
    }

    float metallic;
    if (metallic_texture_set > 0) {
        metallic = texture(metallicMap, vTexCoords).r * metallic_factor;
    } else {
        metallic = metallic_factor;
    }

    float roughness;
    if (roughness_texture_set > 0) {
        roughness = texture(roughnessMap, vTexCoords).r * roughness_factor;
    } else {
        roughness = roughness_factor;
    }

    vec3 v = normalize(camPos - vWorldPos);
    vec3 l = normalize(lightDir.xyz);
    vec3 h = normalize(v + l);

    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 radiance = lightColor;

    float D = DistributionGGX(n, h, roughness);
    float G = GeometrySmith(n, v, l, roughness);
    vec3 F = fresnelSchlick(max(dot(h, v), 0.0), f0);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 ks = F;
    vec3 kd = vec3(1.0) - ks;
    kd *= 1.0 - metallic;

    float ndotl = max(dot(n, l), 0.0);

    Lo = (kd * albedo / M_PI + specular) * radiance * ndotl;

    // IBL
    vec3 f_a = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);

    ks = f_a;
    kd = 1.0 - ks;
    kd *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, n).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 r = reflect(-v, n); 
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, r,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(n, v), 0.0), roughness)).rg;
    specular = prefilteredColor * (f_a * brdf.x + brdf.y);

    // vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 ambient = kd * diffuse + specular;

    vec3 color = Lo + ambient;

    FragColor = vec4(pow(vec3(color), vec3(1.0 / 2.2)), 1.0);
}