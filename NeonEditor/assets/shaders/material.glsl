struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior;// index of refraction
    float dissolve;// 1 == opaque; 0 == fully transparent
    int illum;
    int textureId;
};

vec3 computeDiffuse(Material mat, vec3 lightDir, vec3 normal)
{
    float dotNL = max(dot(normal, lightDir), 0.0);
    vec3 c = mat.diffuse * dotNL;
    return mat.ambient + c;
}

const float PI = 3.14159265;

vec3 computeSpecular(Material mat, vec3 viewDir, vec3 lightDir, vec3 normal)
{
    const float kShininess = max(mat.shininess, 4.0);
    if (dot(lightDir, normal) < 0) return vec3(0);
    vec3 r = normalize(2 * dot(lightDir, normal) * normal - lightDir);
    float specular = pow(max(dot(r, viewDir), 0.0), kShininess);
    return vec3(mat.specular * specular);
}
