#version 450
layout(location=0)out vec4 depth;
void main()
{
    depth = gl_FragCoord;
}
