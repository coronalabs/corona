local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "linearGradient"

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
		name = "position1",
		default = { 0, 0 },
		min = { 0, 0 },
		max = { 1, 1 },
		type="vec2",
		index = 1, -- u_UserData1
	},
	{
		name = "color2",
		default = { 0, 0, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 2, -- u_UserData2
	},
	{
		name = "position2",
		default = { 1, 1 },
		min = { 0, 0 },
		max = { 1, 1 },
		type="vec2",
		index = 3, -- u_UserData3
	},
}

kernel.vertex =
[[
uniform P_COLOR vec4 u_UserData0; // fromColor
uniform P_UV vec2 u_UserData1; // fromPos
uniform P_COLOR vec4 u_UserData2; // toColor
uniform P_UV vec2 u_UserData3; // toPos

varying P_UV float len_V;
varying P_NORMAL vec2 N;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV vec2 fromPos = u_UserData1;
	P_UV vec2 toPos = u_UserData3;

	// IMPORTANT: Positions MUST be in texture-space (same as the UVs).
	P_UV vec2 V = ( toPos - fromPos );
	len_V = length( V );
	N = normalize( V );

	return position;
}
]]

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // fromColor
uniform P_UV vec2 u_UserData1; // fromPos
uniform P_COLOR vec4 u_UserData2; // toColor
uniform P_UV vec2 u_UserData3; // toPos

varying P_UV float len_V;
varying P_NORMAL vec2 N;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 fromColor = u_UserData0;
	P_UV vec2 fromPos = u_UserData1;
	P_COLOR vec4 toColor = u_UserData2;
	P_UV vec2 toPos = u_UserData3;

	// "W" : A vector from the "fromPos" to the current fragment.
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 W = ( texCoord - fromPos );

	// "d" : The progress of "W" along "N".
	P_UV float d = dot( W, N );

	// Keep "d" within reasonable bounds.
	d = clamp( d, 0.0, len_V );

	// "progress" : The unitized progress of "d" along "V".
	P_UV float progress = ( d / len_V );

	#if 0 // For debugging ONLY.

		// Use debug colors only.
		if( progress <= 0.0 )
		{
			return vec4( 1.0, 0.0, 0.0, 1.0 );
		}
		else if( progress >= 1.0 )
		{
			return vec4( 0.0, 1.0, 0.0, 1.0 );
		}
		else
		{
			return vec4( 0.0, 0.0, 1.0, 1.0 );
		}

	#endif

	// Linear interpolation between colors.
	// This multiplies-in the alpha.
	return ( mix( fromColor,
					toColor,
					progress ) * v_ColorScale );
}
]]

return kernel
