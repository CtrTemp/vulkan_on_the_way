
// 第二步编写 FragmentShader
// Fragment Shader 的语法与C语言基本一致

/*
    Fragment Shader 在管线中是在光栅化的下一阶段，光栅化将三角形片元的顶点
阻止生成三角形，并离散化为覆盖屏幕中的像素点。我们称每个被三角形覆盖的像素区域
称为一个“fragment”（片段）

    而Fragment Shader的main()函数作用于每个片段（其实是以每个片段作为最小
单元，并在每个片段中的每个顶点进行具体输入输出操作）

*/ 

#version 450


/*
    不同于vertex shader，fragment shader中没有内置的输入输出变量，需要我们
手动定义指示。
    layout关键字中的location字段表示其在“帧缓冲区”中的索引位置（应该就是表示
的是当前是第几个三角形/第几个fragment）；
    而后out关键字表示输出；
    vec4表示输出的颜色为 RGBA 四通道值
*/ 
layout(location = 0) in vec3 fragColor;


// 如果vertex shader中给出了输出的顶点颜色，我们需要定义一个变量进行承接
layout(location = 0) out vec4 outColor;

void main() {
    // 如果是为整个三角形都输出红色，那么将写做如下：
    // outColor = vec4(1.0, 0.0, 0.0, 1.0);
    // 以下的写法将承接vertex shader中传入的每个顶点的颜色值，而非将色值写死
    outColor = vec4(fragColor, 1.0);
    // 非常巧妙的一点在于，fragment shader将自动地为顶点中间的像素进行平滑颜色插值（双线性插值）
}