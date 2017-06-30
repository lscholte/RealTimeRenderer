in vec3 worldVertPos;
in vec2 fTexCoord;

uniform vec3 CameraPos;
uniform float FocusDistance;

uniform sampler2D DepthMap;
uniform sampler2D Image;

uniform mat4 InverseProjection;
uniform mat4 InverseView;

out vec4 FragColor;

uniform bool IsHorizontalPass;

vec4 positionFromDepth() {
    float depth = texture(DepthMap, fTexCoord).r * 2 - 1;
    vec2 screenCoords = fTexCoord * 2.0 - 1.0;
    vec4 clipPos = vec4(screenCoords, depth, 1.0);
    vec4 viewPos = InverseProjection * clipPos;

    viewPos /= viewPos.w;

    vec4 pixelPos = InverseView * viewPos;

    return pixelPos;
}

vec4 performGuassianBlur(float sigma) {

    float weight[5] = float[] (1/sqrt(2*3.14*sigma*sigma)*exp(-(0)/(2*sigma*sigma)),
                                1/sqrt(2*3.14*sigma*sigma)*exp(-(1)/(2*sigma*sigma)),
                                           1/sqrt(2*3.14*sigma*sigma)*exp(-(4)/(2*sigma*sigma)),
                                           1/sqrt(2*3.14*sigma*sigma)*exp(-(9)/(2*sigma*sigma)),
                                           1/sqrt(2*3.14*sigma*sigma)*exp(-(16)/(2*sigma*sigma)));
    float normalizeFactor = weight[0] + 2*weight[1] + 2*weight[2] + 2*weight[3] + 2*weight[4];

    vec2 tex_offset = 1.0 / textureSize(Image, 0); // gets size of single texel
    vec3 result = texture(Image, fTexCoord).rgb * weight[0] * (1/normalizeFactor);
    if(IsHorizontalPass)
    {
        for(int i = 1; i < 5; ++i)
        {
           result += texture(Image, fTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * (1/normalizeFactor);
           result += texture(Image, fTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * (1/normalizeFactor);
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(Image, fTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i] * (1/normalizeFactor);
            result += texture(Image, fTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i] * (1/normalizeFactor);
        }
    }
    return vec4(result, 1.0);
}

void main(void)
{
    vec4 worldPos = positionFromDepth();
    float distFromCamera = length(CameraPos - worldPos.xyz);
    float sigma = abs(FocusDistance - distFromCamera)/10; //TODO: Maybe make this 10 value that I can change via slider
    FragColor = performGuassianBlur(sigma);
}
