#version 330 core

in vec3 FragmentColor;
in vec2 FragmentUV;
in vec3 FragmentWorldP;
in vec3 FragmentWorldN;

out vec4 OutFragmentColor;

uniform vec3 CameraP;

struct point_light{
	vec3 P;
	vec3 Color;
	float Radius;
	float AmbientPercentage;

	float LinearC;
	float QuadraticC;
} Light1;

struct dir_light{
	vec3 Dir;
	vec3 Color;
	float AmbientPercentage;
} DirLight;

struct surface_material{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Emissive;

	vec3 Color;

	float Shine;
};

uniform surface_material Material;

vec3 CalculatePointLight(
	point_light Lit, 
	vec3 FragDiffC, 
	vec3 FragSpecC,
	vec3 ToCamera)
{
	vec3 Result = vec3(0.0f, 0.0f, 0.0f);

	vec3 ToLight = Lit.P - FragmentWorldP;
	float ToLightLen = length(ToLight);
	ToLight = normalize(ToLight);

	float Attenuation = 1.0f / (1.0f + ToLightLen * Lit.LinearC + (ToLightLen * ToLightLen) * Lit.QuadraticC);

	//NOTE(dima): Ambient lighting
	Result += Attenuation * FragDiffC * vec3(Lit.AmbientPercentage);

	//NOTE(dima): Diffuse lighting
	float CosLightNorm = dot(ToLight, FragmentWorldN);
	CosLightNorm = clamp(CosLightNorm, 0.0f, 1.0f);
	Result += Attenuation * CosLightNorm * FragDiffC * Lit.Color;

	//NOTE(dima): Specular lighting
	//vec4 ReflectPower = texture(Material.Specular, FragmentUV);
	vec4 ReflectPower = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float CosViewRefl = dot(ToCamera, reflect(-ToLight, FragmentWorldN));
	CosViewRefl = clamp(CosViewRefl, 0.0f, 1.0f);
	Result += Attenuation * pow(CosViewRefl, Material.Shine) *  Lit.Color * FragSpecC * ReflectPower.xyz;

	return(Result);
}

vec3 CalculateDirLight(
	dir_light Lit, 
	vec3 FragDiffC, 
	vec3 FragSpecC, 
	vec3 ToCamera)
{
	vec3 Result = vec3(0.0f, 0.0f, 0.0f);

	vec3 ToLight = -Lit.Dir;

	//NOTE(dima): ambient
	Result += FragDiffC * vec3(Lit.AmbientPercentage);

	//NOTE(dima): diffuse
	float CosLightNorm = dot(ToLight, FragmentWorldN);
	CosLightNorm = clamp(CosLightNorm, 0.0f, 1.0f);
	Result += CosLightNorm * Lit.Color * FragDiffC;

	//NOTE(dima): specular
	//vec4 ReflectPower = texture(Material.Specular, FragmentUV);
	vec4 ReflectPower = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float CosViewRefl = dot(ToCamera, reflect(-ToLight, FragmentWorldN));
	CosViewRefl = clamp(CosViewRefl, 0.0f, 1.0f);
	Result += pow(CosViewRefl, Material.Shine) *  Lit.Color * FragSpecC * ReflectPower.xyz;

	return(Result);
}

void main(){

	float GlobalAmbientCoef = 0.05f;

	Light1.P = vec3(0.0f, 4.0f, 0.0f);
	Light1.Color = vec3(1.0f, 1.0f, 1.0f);
	Light1.Radius = 50.0f;
	Light1.AmbientPercentage = 0.05f;
	Light1.LinearC = 2.0f / Light1.Radius;
	Light1.QuadraticC = 1.0f / Light1.Radius / Light1.Radius;

	DirLight.Dir = vec3(0.2f, -1.0f, 0.2f);
	DirLight.Dir = normalize(DirLight.Dir);
	DirLight.Color = vec3(1.0f, 1.0f, 1.0f);
	DirLight.AmbientPercentage = 0.05f;

	vec3 ToCamera = CameraP - FragmentWorldP;
	ToCamera = normalize(ToCamera);

	//v3 FragTextureColor = texture() * FramentColor;

	//vec3 FragDiffColor = texture(Material.Diffuse, FragmentUV).xyz;
	//vec3 FragSpecColor = texture(Material.Specular, FragmentUV).xyz;
	//vec3 FragEmisColor = texture(Material.Emissive, FragmentUV).xyz;
	
	vec3 FragDiffColor = Material.Color;
	vec3 FragSpecColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 FragEmisColor = vec3(0.0f, 0.0f, 0.0f);
	
	vec3 TotalColor = vec3(0.0f, 0.0f, 0.0f);

	//TotalColor += CalculateDirLight(DirLight, FragDiffColor, FragSpecColor, ToCamera);
	TotalColor += CalculatePointLight(Light1, FragDiffColor, FragSpecColor, ToCamera);

	OutFragmentColor = vec4(TotalColor, 1.0f);
}