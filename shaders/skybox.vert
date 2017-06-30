layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

out vec4 TexCoords;

uniform mat4 Projection;
uniform mat4 View;

uniform mat4 ModelViewProjection;


void main()
{
    gl_Position = ModelViewProjection * Position;

    //Guarantee that skybox is the farthest object. Without the tiny offset, it is really buggy
    gl_Position.z = gl_Position.w - 0.00001;
    TexCoords = Position;
}
