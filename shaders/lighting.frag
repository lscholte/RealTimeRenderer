in vec4 normalMapCoord;
in vec3 worldVertPos;
in vec2 fTexCoord;

uniform vec3 CameraDir;

uniform vec3 LightPos;
uniform vec3 LightColour;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D Image;

uniform mat4 InverseProjection;
uniform mat4 InverseView;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


vec4 positionFromDepth() {
    float depth = texture(DepthMap, fTexCoord).r * 2 - 1;
    vec2 screenCoords = fTexCoord * 2.0 - 1.0;
    vec4 clipPos = vec4(screenCoords, depth, 1.0);
    vec4 viewPos = InverseProjection * clipPos;

    viewPos /= viewPos.w;

    vec4 pixelPos = InverseView * viewPos;

    return pixelPos;
}

void main(void)
{

    vec4 worldPos = positionFromDepth();

    vec3 N = normalize(2*texture(NormalMap, fTexCoord).xyz - 1);

    vec3 L = normalize(LightPos - worldPos.xyz);
    float dist = length(LightPos - worldPos.xyz);

    float NL = dot(N, L);

    float constant = 0.3;
    float linear = 0.007;
    float exponential = 0.1;

    float attenuation = constant + linear*dist + exponential*dist*dist;

    vec4 newColour;

    newColour.r = LightColour.r * NL;
    newColour.g = LightColour.g * NL;
    newColour.b = LightColour.b * NL;
    newColour.a = 1;

    newColour /= attenuation;

    FragColor = texture(Image, fTexCoord).rgba + newColour;

    //The vec3 used here came from https://learnopengl.com/#!Advanced-Lighting/Bloom
    //but I'm not exactly sure how those numbers were derived. The idea, though, is that
    //this will determine if a colour is very bright, and if so, then it will be added to
    //a separate texture and then blurred later on to create a light bloom effect.
    if(dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0) {
        BrightColor = vec4(FragColor.rgb,1);
    }
}
