#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 SunLightSpace;
    vec4 SpotLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowSunMap;
uniform sampler2D shadowLightMap;
uniform vec3 viewPos;
uniform vec3 modelPos;
uniform float renderFadeStart;
uniform float renderFadeEnd;
uniform int select;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};
uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse  * diff;
    vec3 specular = light.specular * spec;
    return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (1.0 - shadow) * (ambient + diffuse + specular) * color;
}

float ShadowCalculation(sampler2D shadowMap, vec4 fragPosLightSpace, vec3 direction)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(-direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0){
        shadow = 0.0;
    }
    return shadow;
}

void main()
{
    float distance = distance(fs_in.FragPos, modelPos);
    if(distance < renderFadeEnd){
        vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
        vec3 normal = normalize(fs_in.Normal);
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        // Calculate shadow
        float shadowSun = ShadowCalculation(shadowSunMap, fs_in.SunLightSpace, dirLight.direction); 
        float shadowSpot = ShadowCalculation(shadowLightMap, fs_in.SpotLightSpace, spotLight.direction); 
        // Calculate light
        vec3 result = CalcDirLight(dirLight, normal, viewDir, color, shadowSun);  
        // Calculate spotlight
        result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir, color, shadowSpot); 
        // Calculate Fading
        if(distance > renderFadeStart){
            result *= max(renderFadeEnd - distance, 0.0) / (renderFadeEnd - renderFadeStart);
        }
        if(select == 1){
            result += vec3(0.25,0.25,0.25);
        }
        FragColor = vec4(result, 1.0);
    }else{
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}