local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "exposure"

kernel.vertexData =
{
	{
		name = "exposure",
		default = 0,
		min = -10,
		max = 10,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;

    return vec4( texColor.rgb * pow( 2.0, v_UserData.x ), texColor.w );
}
]]

return kernel
