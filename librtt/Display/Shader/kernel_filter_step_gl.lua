local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "step"

kernel.uniformData =
{
	{
		name = "lowerThreshold",
		default = 0.25,
		min = 0,
		max = 1,
		type="scalar",
		index = 0, -- u_UserData0
	},
	{
		name = "lowerColor",
		default = { 0.10, 0.5, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 1, -- u_UserData1
	},
	{
		name = "higherThreshold",
		default = 0.5,
		min = 0,
		max = 1,
		type="scalar",
		index = 2, -- u_UserData2
	},
	{
		name = "higherColor",
		default = { 0, 0, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 3, -- u_UserData3
	},
}

--[[
How to interpret the above intervals:

	[ 0 .. lowerThreshold .. higherThreshold .. 1 ]

		interval_a : no color    : [               0 .. lowerThreshold  ] : 0.00 .. 0.25
		interval_b : lowerColor  : [  lowerThreshold .. higherThreshold ] : 0.25 .. 0.50
		interval_c : higherColor : [ higherThreshold .. 1               ] : 0.50 .. 1.00

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	// The branching version of this shader is:

		P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

		if( texColor.x >= u_UserData2 )
		{
			// Higher color.
			return ( u_UserData3 * v_ColorScale );
		}
		else if( texColor.x >= u_UserData0 )
		{
			// Lower color.
			return ( u_UserData1 * v_ColorScale );
		}
		else
		{
			// No color.
			return vec4( 0.0 );
		}

	// Our final code uses the branch-less version of the above code.
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
--]]

kernel.fragment =
[[
uniform P_COLOR float u_UserData0; // lowerThreshold
uniform P_COLOR vec4 u_UserData1; // lowerColor
uniform P_COLOR float u_UserData2; // higherThreshold
uniform P_COLOR vec4 u_UserData3; // higherColor

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 texColorResult;
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );
	P_COLOR vec4 colorDelta = ( u_UserData3 - u_UserData1 ); // higherColor - lowerColor.

	// This sets the color to lowerColor.
	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	texColorResult = ( u_UserData1 * step( u_UserData0, texColor.x ) );

	// This sets the color to higherColor.
	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	texColorResult += ( colorDelta * step( u_UserData2, texColor.x ) );

	return ( texColorResult * v_ColorScale );
}
]]

return kernel
