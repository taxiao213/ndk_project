attribute vec4 av_Position;//顶点坐标
attribute vec2 af_Position;//纹理坐标
varying vec2 v_texPosition;//用于把纹理坐标传到fragment里面
uniform mat4 u_Matrix;
void main() {
    v_texPosition = af_Position;
    gl_Position = av_Position*u_Matrix;
}
