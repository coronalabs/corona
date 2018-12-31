local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "iris"

-- IMPORTANT: If you move the position while keeping the unitOpenness
-- constant, the actual radius in pixel WON'T stay constant because
-- the diagonal distance from the center to the most distant corner
-- of the DisplayObject ISN'T constant.
kernel.uniformData =
{
	{
		name = "center",
		default = { 0.5, 0.5 },
		min = { 0, 0 },
		max = { 1, 1 },
		type="vec2",
		index = 0, -- u_UserData0
	},
	{
		name = "aperture",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 1, -- u_UserData1
	},
	{
		name = "aspectRatio",
		default = 1,
		min = 0,
		max = 99999,
		type="scalar",
		index = 2, -- u_UserData2
	},
	{
		name = "smoothness",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 3, -- u_UserData3
	},
}

kernel.vertex =
[[
uniform P_UV vec2 u_UserData0; // center
uniform P_UV float u_UserData1; // unitOpenness
uniform P_UV float u_UserData2; // aspectRatio
uniform P_UV float u_UserData3; // unitSmoothness

varying P_UV float feathering_lower_edge;
varying P_UV float feathering_upper_edge;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV vec2 center = u_UserData0;

	// We need the max() because we want to determine the
	// maximum distance to cover.
	P_UV float xMax = max( center.x,
							( 1.0 - center.x ) );
	P_UV float yMax = max( center.y,
							( 1.0 - center.y ) );

	// unitOpenness is [ 0.0 .. 1.0 ].
	//
	//		0: Closed iris (complete coverage / no radius).
	//		1: Open iris (no coverage / full radius).
	P_UV float unitOpenness = u_UserData1;

	// aspectRatio = ( object.width / object.height )
	P_UV float aspectRatio = u_UserData2;

	// Diagonal length.
	//
	// maximum_distance_to_cover = sqrt( a^2 + b^2 );
	P_UV float a = ( xMax * aspectRatio );
	a = ( a * a );

	P_UV float b = yMax;
	b = ( b * b );

	P_UV float unitSmoothness = u_UserData3;

	// "0.5" : This is an arbitrary constant. This must be a constant so
	// that the thickness of the smooth border is constant.
	P_UV float feathering_edge_thickness = ( 0.5 * unitSmoothness );

	// Add the smooth border thickness so we can move it completely out of view
	// when necessary.
	P_UV float maximum_distance_to_cover = ( sqrt( a + b ) + feathering_edge_thickness );

	// This is the "solid" edge.
	feathering_upper_edge = ( unitOpenness * maximum_distance_to_cover );

	// This is the "transparent" edge.
	feathering_lower_edge = ( feathering_upper_edge - feathering_edge_thickness );

	return position;
}
]]

kernel.fragment =
[[
uniform P_UV vec2 u_UserData0; // center
uniform P_UV float u_UserData1; // unitOpenness
uniform P_UV float u_UserData2; // aspectRatio
uniform P_UV float u_UserData3; // unitSmoothness

varying P_UV float feathering_lower_edge;
varying P_UV float feathering_upper_edge;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// aspectRatio = ( object.width / object.height )
	P_UV float aspectRatio = u_UserData2;

	P_UV vec2 center = vec2( ( u_UserData0.x * aspectRatio ),
								u_UserData0.y );

	// Current fragment position in texture-space.
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 pos = vec2( ( texCoord.x * aspectRatio ),
								texCoord.y );

	// Distance from the center to the current fragment.
	P_UV float dist = distance( pos,
								center );

	// Get the colors to modulate.
	// We want to use texCoord instead of "pos" here because
	// we want to sample the texture at its unmodified coordinates.
	// "pos" is only useful to determine the transparency.
	P_COLOR vec4 color = ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );

	#if 0 // For debugging ONLY.

		// Blend with a plain color only.
		// Diregard the texture color sampled above.
		color = v_ColorScale;

	#elif 0 // For debugging ONLY.

		// Use debug colors only.
		if( dist <= feathering_lower_edge )
		{
			return vec4( 1.0, 0.0, 0.0, 1.0 );
		}
		else if( dist >= feathering_upper_edge )
		{
			return vec4( 0.0, 1.0, 0.0, 1.0 );
		}
		else // if( ( dist > feathering_lower_edge ) && ( dist < feathering_upper_edge ) )
		{
			return vec4( 0.0, 0.0, 1.0, 1.0 );
		}

	#endif

	return ( color * smoothstep( feathering_lower_edge,
									feathering_upper_edge,
									dist ) );
}
]]

return kernel
