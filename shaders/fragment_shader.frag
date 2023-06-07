
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
layout(location = 1) in vec2 fragTexCoord; // 第七步，承接来自Vertex buffer的UV坐标

// 第八步，我们将添加对纹理的采样（从 uniform buffer 中）
layout(binding = 1) uniform sampler2D texSampler;

// 如果vertex shader中给出了输出的顶点颜色，我们需要定义一个变量进行承接
layout(location = 0) out vec4 outColor;

void main() {
    // outColor = vec4(fragColor, 1.0);
    // 将以上修改如下，Fragment Shader将自动根据顶点的UV坐标对颜色进行插值
    // outColor = vec4(fragTexCoord, 0.0, 1.0); // 对应第七步修改（此时还是常数展示）

    // 对应第八步修改如下
    // outColor = texture(texSampler, fragTexCoord);
    /*
        第九步
        因为我们设置了重复模式，所以索引超出的情况可以看到图片纹素重复的效果，如下修改：
    */ 
    // outColor = texture(texSampler, fragTexCoord * 2.0);
    /*
        第十步，
        如果你想看到原定义颜色与纹理采样叠加的效果，可以进行如下修改
    */ 
    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}
