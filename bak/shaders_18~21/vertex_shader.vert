
/*
    这里对我们的三角形坐标做一个说明，在这里定义的三角形坐标是预定义的，
并非由计算得出，直接是映射后应该得到的归一化坐标，纵横的range都是-1～1。
对应屏幕中心的坐标为（0,0），左上角为（-1,-1），右下角为（1,1）。
    （特别说明，这里的坐标定义y轴的正负方向与OpenGL的定义是相反的）
*/ 
#version 450


// // 这里预定义三角形的坐标，在非文件读入的时候，我们就这样做
// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );


/*
    如果你想让你的三角形的每个顶点有不同的颜色，我们同样可以通过预定义的方式来确定其颜色。
这里我们将三个顶点刚好置为 R/G/B 全色
    其实这里的颜色值应该也可以从文件读入
*/ 
// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

/*
    首先第一步就是注释掉以上的顶点位置/颜色初始化赋值语句，使得我们的顶点不再从文件中硬编码。取而代之的是
我们从顶点缓冲区获取顶点信息，替换为如下语句：
*/ 

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

// 注意当使用 dvec -> 64位双精度类型时，其占位宽度翻倍，变量间隔应至少为2,如下

// layout(location = 0) in dvec3 inPosition;
// layout(location = 2) in vec3 inColor;

/*
    在 vertex shader 中，每个顶点都会调用一次main()函数！这是在GPU中内置并行的部分。
    其中 gl_Position 为内置变量，用作管线中当前组件的输出，gl_VertexIndex也是内置
变量，用于指示当前顶点的索引。
    可以看到，GLSL语言中允许使用这种插入拼接的方式来从低维数组创建高维数组，其中gl_Position
的第三个维度代表其深度坐标，这里我们就将其置为0，不考虑任何三角形的深度（因为目前只渲染一个）。
第四维度代表不对坐标做任何变换直接输出。
*/ 

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}