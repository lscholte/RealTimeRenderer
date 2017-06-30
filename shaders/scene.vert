layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

layout(location = SCENE_NORMAL_ATTRIB_LOCATION)
in vec3 Normal;

in vec3 vertex_color;

out vec3 fragment_color;
out vec3 worldNormal;
out vec3 modelVertPos;
out vec3 worldVertPos;
out vec2 fTexCoord;

uniform mat4 ModelWorld;
uniform mat4 ModelViewProjection;
uniform mat3 Normal_ModelWorld;

uniform mat4 LightMatrix;
uniform mat4 NormalMatrix;
out vec4 shadowMapCoord;
out vec4 normalMapCoord;

void main()
{
    gl_Position = ModelViewProjection * Position;
    
    worldVertPos = (ModelWorld * Position).xyz;

    worldNormal = Normal_ModelWorld * Normal;

    fTexCoord = TexCoord;

    fragment_color = vertex_color;

    shadowMapCoord = LightMatrix * Position;
    normalMapCoord = NormalMatrix * Position;


}
