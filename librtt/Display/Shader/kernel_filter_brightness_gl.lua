local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "brightness"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 0,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// texColor has pre-multiplied alpha.
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	// pre-multiply brigtness as well
	P_COLOR float brightness = v_UserData.x * texColor.a;

	// Add the brightness.
	texColor.rgb += brightness;

	return ( texColor * v_ColorScale );
}
]]

return kernel
