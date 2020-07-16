// camera_vertex
//顶点着色器
// 把顶点坐标给这个变量，确定要画的形状
attribute vec4 vPosition;

// 接收纹理坐标，接收采样器采样图片的坐标
attribute vec4 vCoord;

// 变换矩阵， 需要将原本的vCoord(01,11,00,10)
// 与矩阵相乘，才能够得到surfacetexure(特殊)的正确的采样坐标
uniform mat4 vMatrix;

// 传给片元着色器 像素点
varying vec2 aCoord;

void main(){
    // 内置变量 gl_position,我们把顶点数据赋值给这个变量
    // opengl 就知道它要画什么形状了
    gl_Position = vPosition;
    // 测试和设备无关
    aCoord = (vMatrix*vCoord).xy;
    // aCoord = vec2((vCoord*vMatrix).x,(vCoord*vMatrix).y);
}