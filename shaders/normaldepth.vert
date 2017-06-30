layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

layout(location = SCENE_NORMAL_ATTRIB_LOCATION)
in vec3 Normal;

uniform mat4 ModelWorld;
uniform mat4 ModelViewProjection;
uniform mat3 Normal_ModelWorld;

out vec3 worldVertPos;
out vec3 modelNormal;
out vec3 worldNormal;
out vec2 fTexCoord;

void main() {
    gl_Position = ModelViewProjection * Position;

    modelNormal = Normal;
    fTexCoord = TexCoord;
    worldVertPos = (ModelWorld * Position).xyz;
    worldNormal = Normal_ModelWorld * Normal;
}
