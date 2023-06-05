
/*
    这里对我们的三角形坐标做一个说明，在这里定义的三角形坐标是预定义的，
并非由计算得出，直接是映射后应该得到的归一化坐标，纵横的range都是-1～1。
对应屏幕中心的坐标为（0,0），左上角为（-1,-1），右下角为（1,1）。
    （特别说明，这里的坐标定义y轴的正负方向与OpenGL的定义是相反的）
*/ 
#version 450

/*
    第二步，在这里添加MVP变换阵，引入这个 “统一缓冲区” 对象，可见当前这个对象是在顶点着色器
文件中硬编码进去的（至少当作一个占位符存在）
*/ 
layout(binding = 0) uniform UniformBufferObject {
    vec2 foo;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

/*
    在 vertex shader 中，每个顶点都会调用一次main()函数！这是在GPU中内置并行的部分。
    其中 gl_Position 为内置变量，用作管线中当前组件的输出，gl_VertexIndex也是内置
变量，用于指示当前顶点的索引。
    可以看到，GLSL语言中允许使用这种插入拼接的方式来从低维数组创建高维数组，其中gl_Position
的第三个维度代表其深度坐标，这里我们就将其置为0，不考虑任何三角形的深度（因为目前只渲染一个）。
第四维度代表不对坐标做任何变换直接输出。
*/ 

void main() {
    // gl_Position = vec4(inPosition, 0.0, 1.0);

    // 对当前矩阵应用 MVP 变换阵（其实就是连续乘这几个矩阵就好）
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}