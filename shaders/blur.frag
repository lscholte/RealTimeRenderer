out vec4 FragColor;

in vec2 fTexCoord;

uniform sampler2D Image;
uniform bool IsHorizontalPass;

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
     vec2 tex_offset = 1.0 / textureSize(Image, 0); // gets size of single texel
     vec3 result = texture(Image, fTexCoord).rgb * weight[0];
     if(IsHorizontalPass)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += texture(Image, fTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(Image, fTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += texture(Image, fTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
             result += texture(Image, fTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
         }
     }
     FragColor = vec4(result, 1.0);
}
