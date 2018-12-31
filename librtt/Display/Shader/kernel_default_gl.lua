local kernel = {}

kernel.language = "glsl"

kernel.category = "default"

kernel.name = "default"

kernel.vertex =
[[
P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	return position;
}
]]

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    return texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
}
]]

return kernel
