#version 330 core

// Transformation matrices.
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;
// Camera.
uniform vec3 cameraPos;
// Material properties.
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform sampler2D mapKd;
uniform int useMapKd;
// Light data.
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightIntensity;
uniform float spotLightCutoffStart;
uniform float spotLightTotalWidth;

in vec3 iPosition;
in vec3 iNormal;
in vec2 iTexCoord;

out vec4 FragColor;


vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 lightDir)
{
    return Kd * I * max(0, dot(N, lightDir));
}

vec3 Specular(vec3 Ks, vec3 I, vec3 N, vec3 lightDir, vec3 viewDir, float Ns)
{
    vec3 vH = (lightDir + viewDir) / length(lightDir + viewDir);
    return Ks * I * pow(max(0, dot(N, vH)), Ns);
}

void main()
{
    vec3 fsNormal = normalize(iNormal);
    vec3 fsKd = useMapKd * texture2D(mapKd, iTexCoord).rgb + (1 - useMapKd) * Kd;

    // -------------------------------------------------------------
    // Ambient light.
    vec3 ambient = Ka * ambientLight;
    // -------------------------------------------------------------
    // Directional light.
    vec3 fsLightDir = normalize((viewMatrix * vec4(-dirLightDir, 0.0)).xyz);
    // Diffuse.
    vec3 diffuse = Diffuse(fsKd, dirLightRadiance, fsNormal, fsLightDir);
    // Specular.
    vec3 fsViewDir = normalize((viewMatrix * vec4(cameraPos, 0.0)).xyz - iPosition);
    vec3 specular = Specular(Ks, dirLightRadiance, fsNormal, fsLightDir, fsViewDir, Ns);
    vec3 dirLight = diffuse + specular;
    // -------------------------------------------------------------
    // Point light.
    vec4 tmpPos = viewMatrix * vec4(pointLightPos, 1.0);
    vec3 fsLightPos = tmpPos.xyz / tmpPos.w;
    fsLightDir = normalize(fsLightPos - iPosition);
    float distSurfaceToLight = distance(fsLightPos, iPosition);
    float attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    vec3 radiance = pointLightIntensity * attenuation;
    // Diffuse.
    diffuse = Diffuse(fsKd, radiance, fsNormal, fsLightDir);
    // Specular.
    specular = Specular(Ks, radiance, fsNormal, fsLightDir, fsViewDir, Ns);
    vec3 pointLight = diffuse + specular;
    // -------------------------------------------------------------
    // Spot light
    tmpPos = viewMatrix * vec4(spotLightPos, 1.0);
    fsLightPos = tmpPos.xyz / tmpPos.w;
    fsLightDir = normalize(fsLightPos - iPosition);
    distSurfaceToLight = distance(fsLightPos, iPosition);
    attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    radiance = spotLightIntensity * attenuation;
    // Diffuse.
    diffuse = Diffuse(fsKd, radiance, fsNormal, fsLightDir);
    // Specular.
    specular = Specular(Ks, radiance, fsNormal, fsLightDir, fsViewDir, Ns);
    vec3 spotLight = diffuse + specular;
    vec3 fsSpotLightDir = normalize((viewMatrix * vec4(-spotLightDir, 0.0)).xyz);
    float degree = degrees(acos(dot(fsLightDir, fsSpotLightDir)));
    spotLight *= (spotLightTotalWidth - min(max(degree, spotLightCutoffStart), spotLightTotalWidth)) / 
                 (spotLightTotalWidth - spotLightCutoffStart);
    // -------------------------------------------------------------
    
    vec3 lightingColor = ambient + dirLight + pointLight + spotLight;
    FragColor = vec4(lightingColor, 1.0);
}
