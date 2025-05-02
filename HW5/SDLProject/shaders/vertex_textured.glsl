attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec2 texCoordVar;
varying vec2 varPosition;

void main()
{
    vec4 world_pos = modelMatrix * position;
    varPosition = world_pos.xy;              

    texCoordVar = texCoord;
    gl_Position = projectionMatrix * viewMatrix * world_pos;
}
