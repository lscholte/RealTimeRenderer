layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

out vec2 fTexCoord;

void main() {
    gl_Position = Position;

    fTexCoord = TexCoord;

}
