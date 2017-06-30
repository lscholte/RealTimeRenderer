in vec3 worldVertPos;
in vec3 modelNormal;
in vec3 worldNormal;
in vec2 fTexCoord;

uniform int HasBumpMap;
uniform sampler2D BumpMap;

uniform vec3 CameraPos;

out vec4 FragColor;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
    vec3 T = (dp1 * duv2.y   - dp2 * duv1.y)*r;
    vec3 B = (dp2 * duv1.x   - dp1 * duv2.x)*r;

    T = normalize(T);
    B = normalize(B);
    N = normalize(N);

    return mat3(T, B, N);
}

void main() {

    vec3 worldCameraPos = CameraPos;

    vec3 N = normalize(worldNormal);

    if (HasBumpMap != 0) {
        vec3 V = normalize(worldCameraPos - worldVertPos);
        mat3 TBN = cotangent_frame(N, V, fTexCoord);
        N = normalize(2*texture(BumpMap, fTexCoord).xyz-1);
        N = normalize(TBN * N);
    }

    FragColor = vec4(0.5*(N+1), 1);
}
