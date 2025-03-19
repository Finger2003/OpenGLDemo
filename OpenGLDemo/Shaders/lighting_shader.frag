#version 460 core
out vec4 FragColor;


struct DirLight {
    vec3 direction;
    vec3 color;
	
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 color;
    
//    float constant;
//    float linear;
//    float quadratic;
//	
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float edgeCoeff;
    vec3 color;
//    float cutOff;
//    float outerCutOff;
//  
//    float constant;
//    float linear;
//    float quadratic;
//  
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;       
};


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform bool useBlinn;
uniform bool isDay;
uniform float fogIntensity;
uniform vec3 fogColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_ambient1;
uniform sampler2D texture_emissive1;
uniform sampler2D texture_shininess1;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float CalcFogFactor(vec3 worldPos);

float CalcShininessExponent(vec3 color);
float CalcBlinnShininessExponent(vec3 color);
float CalcDiff(vec3 normal, vec3 lightDir);
float CalcSpec(vec3 normal, vec3 lightDir, vec3 viewDir);
vec3 CalcDiffVec(vec3 normal, vec3 lightDir);
vec3 CalcSpecVec(vec3 normal, vec3 lightDir, vec3 viewDir);
float CalcAttenuation(vec3 lightPos, vec3 fragPos);

const float att_constant = 1.0;
const float att_linear = 0.09;
const float att_quadratic = 0.032;


void main()
{

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // ambient
    vec3 result = texture(texture_ambient1, TexCoords).rgb + texture(texture_emissive1, TexCoords).rgb;

    // directional light
    if(isDay)
        result += CalcDirLight(dirLight, norm, viewDir);

    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    // spot light
	for(int i = 0; i < NR_SPOT_LIGHTS; i++)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);


    float fogFactor = CalcFogFactor(FragPos);
    result = mix(fogColor, result, fogFactor);

    result = clamp(result, 0.0, 1.0);
	FragColor = vec4(result, 1.0);
}




vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    vec3 diffuse = CalcDiffVec(normal, lightDir);
    vec3 specular = CalcSpecVec(normal, lightDir, viewDir);

    return (diffuse + specular) * light.color;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float attenuation = CalcAttenuation(light.position, fragPos);

    vec3 diffuse = CalcDiffVec(normal, lightDir);
    vec3 specular = CalcSpecVec(normal, lightDir, viewDir);

    return (diffuse + specular) * attenuation * light.color;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);


	// spotlight intensity
    float intensity = pow(max(dot(lightDir, normalize(-light.direction)),0), light.edgeCoeff);

	float attenuation = CalcAttenuation(light.position, fragPos);

	vec3 diffuse = CalcDiffVec(normal, lightDir);
	vec3 specular = CalcSpecVec(normal, lightDir, viewDir);

	return (diffuse + specular) * attenuation * intensity * light.color;    
}

float CalcShininessExponent(vec3 color)
{
	float shininess = (color.r + color.g + color.b) * 20.0;
    return shininess;
}

float CalcBlinnShininessExponent(vec3 color)
{
    return 4.0 * CalcShininessExponent(color);
}

float CalcFogFactor(vec3 worldPos)
{
    if(fogIntensity == 0.0)
    	return 1;

    float gradient = ((fogIntensity - 50) * fogIntensity + 60);
    float dist = distance(worldPos, viewPos);

    float fog = exp(-pow(dist / gradient, 4.0));
    return clamp(fog, 0.0, 1.0);
}


float CalcDiff(vec3 normal, vec3 lightDir)
{
    float diff = max(dot(normal, lightDir), 0.0);
    return diff;
}

float CalcSpec(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    float spec = 0.0f;
    vec3 shininess_texture = texture(texture_shininess1, TexCoords).rgb;
    vec3 v1, v2;
    float shininess;

    if(useBlinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        shininess = CalcBlinnShininessExponent(shininess_texture);
        v1 = normal;
        v2 = halfwayDir;
    }
    else
	{
		vec3 reflectDir = reflect(-lightDir, normal);
        shininess = CalcShininessExponent(shininess_texture);
        v1 = viewDir;
        v2 = reflectDir;
	}

    return pow(max(dot(v1, v2), 0.0), shininess);
}

vec3 CalcDiffVec(vec3 normal, vec3 lightDir)
{
    float diff = CalcDiff(normal, lightDir);
    return diff * texture(texture_diffuse1, TexCoords).rgb;
}

vec3 CalcSpecVec(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    float spec = CalcSpec(normal, lightDir, viewDir);
    return spec * texture(texture_specular1, TexCoords).rgb;
}

float CalcAttenuation(vec3 lightPos, vec3 fragPos)
{
    float dist = distance(lightPos, fragPos);
    float attenuation = 1.0 / (((att_quadratic * dist) + att_linear) * dist + att_constant);
    return attenuation;
}