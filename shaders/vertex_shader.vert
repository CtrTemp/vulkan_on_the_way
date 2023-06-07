
/*
    这里对我们的三角形坐标做一个说明，在这里定义的三角形坐标是预定义的，
并非由计算得出，直接是映射后应该得到的归一化坐标，纵横的range都是-1～1。
对应屏幕中心的坐标为（0,0），左上角为（-1,-1），右下角为（1,1）。
    （特别说明，这里的坐标定义y轴的正负方向与OpenGL的定义是相反的）
*/ 
#version 450


layout(binding = 0) uniform UniformBufferObject {
    // vec2 foo;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;



// 这里对应的就是C++中写的 verteies
// 第三步，将Vertex shader中的vec2也对应改为vec3
// layout(location = 0) in vec2 inPosition;    // 实际坐标
layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inColor;       // 实际顶点颜色
layout(location = 2) in vec2 inTexCoord;    // 顶点对应UV坐标（这是新添加的）

layout(location = 0) out vec3 fragColor;    // 着色器输出颜色
layout(location = 1) out vec2 fragTexCoord; // 片段对应的纹理坐标（这是新添加的）



/*
    在 vertex shader 中，每个顶点都会调用一次main()函数！这是在GPU中内置并行的部分。
    其中 gl_Position 为内置变量，用作管线中当前组件的输出，gl_VertexIndex也是内置
变量，用于指示当前顶点的索引。
    可以看到，GLSL语言中允许使用这种插入拼接的方式来从低维数组创建高维数组，其中gl_Position
的第三个维度代表其深度坐标，这里我们就将其置为0，不考虑任何三角形的深度（因为目前只渲染一个）。
第四维度代表不对坐标做任何变换直接输出。
*/ 

void main() {
    // gl_Position 是默认变量，输出到vertex shader，以下这里应用了mvp变换
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord; // 同样，我们将UV值也传给后面的fragment shader
}