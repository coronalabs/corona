local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "checkerboard"

kernel.uniformData =
{
	{
		name = "color1",
		default = { 1, 0, 0, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "color2",
		default = { 0, 0, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 1, -- u_UserData1
	},
	{
		name = "xStep",
		default = 3,
		min = 0,
		max = 99999,
		type="scalar",
		index = 2, -- u_UserData2
	},
	{
		name = "yStep",
		default = 3,
		min = 0,
		max = 99999,
		type="scalar",
		index = 3, -- u_UserData3
	},
}

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // color1
uniform P_COLOR vec4 u_UserData1; // color2
uniform P_UV float u_UserData2; // xStep
uniform P_UV float u_UserData3; // yStep

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 color1 = u_UserData0;
	P_COLOR vec4 color2 = u_UserData1;
	P_UV float xStep = u_UserData2;
	P_UV float yStep = u_UserData3;

	// "xStep" is the number of time the pattern is repeated by texCoord.x.
	// "yStep" is the number of time the pattern is repeated by texCoord.y.
	//
	// If "xStep" is 1.0, then the pattern will follow the natural progression
	// of texCoord.x, along the X axis, only once.

	P_UV float total = ( floor( texCoord.x * xStep ) +
							floor( texCoord.y * yStep ) );

	bool is_even = ( mod( total, 2.0 ) < 0.001 );

	return ( ( is_even ? color1 : color2 ) * v_ColorScale );
}
]]

return kernel
