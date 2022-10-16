precision mediump float;
out float depth;
void main()
{
    depth = gl_FragCoord.z;
}
