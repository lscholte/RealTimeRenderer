uniform vec3 CameraPos;
uniform vec3 LightPos;

uniform sampler2DShadow ShadowMap;
uniform sampler2D NormalMap;
uniform sampler2D LightMap;
uniform sampler2D BloomMap;

uniform vec3 Ambient;
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float Shininess;

uniform int HasDiffuseMap;
uniform sampler2D DiffuseMap;

uniform int HasSpecularMap;
uniform sampler2D SpecularMap;

uniform int HasAlphaMask;
uniform sampler2D AlphaMask;

uniform mat3 Normal_ModelWorld;

out vec3 FragColor;

in vec3 fragment_color;
in vec3 worldNormal;
in vec3 worldVertPos;
in vec2 fTexCoord;

in vec4 shadowMapCoord;
in vec4 normalMapCoord;

void main()
{

    if (HasAlphaMask != 0) {
        if(texture(AlphaMask, fTexCoord).r < 0.9) {
            discard;
        }
    }

    vec3 worldLightPos = LightPos;
    vec3 worldCameraPos = CameraPos;

    vec3 N = normalize(2*textureProj(NormalMap, normalMapCoord).rgb - 1);
    vec3 L = normalize(worldLightPos - worldVertPos);
    vec3 V = normalize(worldCameraPos - worldVertPos);

    float lambertian = max(dot(L, N), 0.0);
    float specular = 0.0;

    float visibility = textureProj(ShadowMap, shadowMapCoord);

    if(lambertian > 0.0) {

      vec3 H = normalize(L + V);
      float specAngle = max(dot(H, N), 0.0);
      specular = pow(specAngle, Shininess);

    }

    vec3 diffuseMap;
    if (HasDiffuseMap != 0) {
        diffuseMap = texture(DiffuseMap, fTexCoord).rgb;
    }
    else {
        diffuseMap = vec3(1,1,1);
    }

    vec3 specularMap;
    if (HasDiffuseMap != 0) {
        specularMap = texture(SpecularMap, fTexCoord).rgb;
    }
    else {
        specularMap = vec3(1,1,1);
    }


    vec3 lightMap = textureProj(LightMap, normalMapCoord).rgb;
    vec3 bloomMap = textureProj(BloomMap, normalMapCoord).rgb;

    vec3 colorLinear = Ambient * 0.01  +
                       lambertian * diffuseMap * visibility +
                       Specular * specular * specularMap * visibility +
                       lightMap * diffuseMap +
                       bloomMap;


    FragColor = colorLinear;
}
