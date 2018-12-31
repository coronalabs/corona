local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "color"

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    return v_ColorScale;
}
]]

return kernel
