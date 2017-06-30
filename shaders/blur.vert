layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

out vec3 worldVertPos;
out vec2 fTexCoord;
out vec4 normalMapCoord;

void main() {
    gl_Position = Position;

    fTexCoord = TexCoord;
}
