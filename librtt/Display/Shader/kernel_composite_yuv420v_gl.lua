local kernel = {}

kernel.language = "glsl"
kernel.category = "composite"
kernel.name = "yuv420v"

kernel.fragment =
[[
// Using BT.709 which is the standard for HDTV
const P_COLOR mat3 kColorMap = mat3(
          1,       1,      1,
          0, -.18732, 1.8556,
    1.57481, -.46813,      0);

// BT.601, which is the standard for SDTV is provided as a reference
/*
const P_COLOR mat3 kColorMap = mat3(
        1,       1,     1,
        0, -.34413, 1.772,
    1.402, -.71414,     0);
*/
    

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec3 yuv;
    
    yuv.x = texture2D(u_FillSampler0, texCoord).r;
    yuv.yz = texture2D(u_FillSampler1, texCoord).rg - vec2(0.5, 0.5);
    
    P_COLOR vec3 rgb = kColorMap * yuv;
    
    return vec4(rgb, 1) * v_ColorScale;
}
]]

return kernel
