local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "radialGradient"

kernel.uniformData =
{
	{
		name = "aspectRatio",
		default = 1,
		min = 0,
		max = 99999,
		type="scalar",
		index = 0, -- u_UserData0
	},
	{
		-- x is center.x.
		-- y is center.y.
		-- z is innerRadius.
		-- w is outerRadius.
		name = "center_and_radiuses",
		default = { 0.5, 0.5, 0.125, 0.25 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 1, -- u_UserData1
	},
	{
		name = "color1",
		default = { 1, 0, 0, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 2, -- u_UserData2
	},
	{
		name = "color2",
		default = { 0, 0, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 3, -- u_UserData3
	},
}

kernel.vertex =
[[
uniform P_UV float u_UserData0; // aspectRatio
uniform P_UV vec4 u_UserData1; // center_and_radiuses
uniform P_UV vec4 u_UserData2; // color1
uniform P_UV vec4 u_UserData3; // color2

varying P_UV float one_over_radius_range;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// aspectRatio = ( object.width / object.height )
	P_UV float aspectRatio = u_UserData0;

	// u_UserData1 = { center.x, center.y, innerRadius, outerRadius }
	P_UV vec2 center = vec2( ( u_UserData1.x * aspectRatio ),
								u_UserData1.y );
	P_UV float innerRadius = u_UserData1.z;
	P_UV float outerRadius = u_UserData1.w;

	P_UV vec4 color1 = u_UserData2;
	P_UV vec4 color2 = u_UserData3;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	one_over_radius_range = ( 1.0 / ( outerRadius - innerRadius ) );

	return position;
}
]]

kernel.fragment =
[[
uniform P_UV float u_UserData0; // aspectRatio
uniform P_UV vec4 u_UserData1; // center_and_radiuses
uniform P_UV vec4 u_UserData2; // color1
uniform P_UV vec4 u_UserData3; // color2

varying P_UV float one_over_radius_range;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// aspectRatio = ( object.width / object.height )
	P_UV float aspectRatio = u_UserData0;

	// u_UserData1 = { center.x, center.y, innerRadius, outerRadius }
	P_UV vec2 center = vec2( ( u_UserData1.x * aspectRatio ),
								u_UserData1.y );
	P_UV float innerRadius = u_UserData1.z;
	P_UV float outerRadius = u_UserData1.w;

	P_UV vec4 color1 = u_UserData2;
	P_UV vec4 color2 = u_UserData3;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// Current fragment position in texture-space.
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 pos = vec2( ( texCoord.x * aspectRatio ),
							texCoord.y );

	// Distance from the center to the current fragment.
	P_UV float dist0 = distance( pos,
									center );

	// Normalize the distance over the range of the radiuses.
	P_UV float dist1 = ( ( dist0 - innerRadius ) * one_over_radius_range );

	#if 0 // For debugging ONLY.

		// Use debug colors only.
		if( dist0 <= innerRadius )
		{
			return vec4( 1.0, 0.0, 0.0, 1.0 );
		}
		else if( dist0 >= outerRadius )
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
	return ( mix( color1,
					color2,
					dist1 ) * v_ColorScale );
}
]]

return kernel
