local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "posterize"

kernel.vertexData =
{
	{
		name = "colorsPerChannel",
		default = 4,
		min = 2,
		max = 99999,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	mediump float color_count = v_UserData.x;

	mediump vec4 color = texture2D( u_FillSampler0, texCoord );

	// This quantification can be visualized in Grapher.app using:
	//
	//		y = ( floor( 8.0 * x ) / 8.0 )
	//
	// Where 8 is the color_count.
	P_COLOR vec4 posterized_color = ( floor( color_count * color ) / color_count );

	return ( posterized_color * v_ColorScale );
}
]]

return kernel
