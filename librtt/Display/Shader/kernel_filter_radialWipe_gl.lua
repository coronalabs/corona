local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "radialWipe"

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
		name = "progress",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 1, -- u_UserData1
	},
	{
		name = "axisOrientation",
		default = 0,
		min = 0,
		max = 1,
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
uniform P_UV float u_UserData2; // unitOpennessOffset
uniform P_UV float u_UserData3; // unitSmoothness

// Some Android devices AREN'T able to use "varying mat2".
varying P_UV vec2 opennessOffsetMatrix0; // 2D rotation matrix (See unitOpennessOffset).
varying P_UV vec2 opennessOffsetMatrix1; // 2D rotation matrix (See unitOpennessOffset).

varying P_UV float feathering_lower_edge_in_radians;
varying P_UV float feathering_upper_edge_in_radians;

const P_UV float kPI = 3.14159265359;
const P_UV float kTWO_PI = ( 2.0 * kPI );

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV vec2 center = u_UserData0;

	// unitOpenness is [ 0.0 .. 1.0 ].
	//
	//		0: Closed iris (complete coverage / no radius).
	//		1: Open iris (no coverage / full radius).
	P_UV float unitOpenness = u_UserData1;

	// This is used to "offset" (rotate) the
	// starting reference of unitOpenness.
	P_UV float unitOpennessOffset = u_UserData2;

	P_UV float rotation_offset_in_radians = ( unitOpennessOffset * kTWO_PI );

	{
		P_UV float s = sin( rotation_offset_in_radians );
		P_UV float c = cos( rotation_offset_in_radians );

		opennessOffsetMatrix0 = vec2( c, ( - s ) );
		opennessOffsetMatrix1 = vec2( s, c );
	}

	P_UV float unitSmoothness = u_UserData3;

	// kPI : This is an arbitrary constant. This must be a constant so
	// that the thickness of the smooth border is constant.
	P_UV float feathering_edge_thickness_in_radians = ( kPI * unitSmoothness );

	// Add the smooth border thickness so we can move it completely out of view
	// when necessary.
	P_UV float maximum_distance_to_cover_in_radians = ( kTWO_PI + feathering_edge_thickness_in_radians );

	// This is the "transparent" (leading) edge.
	feathering_upper_edge_in_radians = ( unitOpenness * maximum_distance_to_cover_in_radians );

	// This is the "opaque" (trailing) edge.
	feathering_lower_edge_in_radians = ( feathering_upper_edge_in_radians - feathering_edge_thickness_in_radians );

	return position;
}
]]

kernel.fragment =
[[
uniform P_UV vec2 u_UserData0; // center
uniform P_UV float u_UserData1; // unitOpenness
uniform P_UV float u_UserData2; // unitOpennessOffset
uniform P_UV float u_UserData3; // unitSmoothness

// Some Android devices AREN'T able to use "varying mat2".
varying P_UV vec2 opennessOffsetMatrix0; // 2D rotation matrix (See unitOpennessOffset).
varying P_UV vec2 opennessOffsetMatrix1; // 2D rotation matrix (See unitOpennessOffset).

varying P_UV float feathering_lower_edge_in_radians;
varying P_UV float feathering_upper_edge_in_radians;

const P_UV float kPI = 3.14159265359;
const P_UV float kTWO_PI = ( 2.0 * kPI );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV vec2 center = u_UserData0;

	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!

		// Rotate by unitOpennessOffset.
		P_UV mat2 opennessOffsetMatrix;
		opennessOffsetMatrix[ 0 ] = opennessOffsetMatrix0;
		opennessOffsetMatrix[ 1 ] = opennessOffsetMatrix1;

		P_UV vec2 rotated_texCoord = ( opennessOffsetMatrix * texCoord );
		P_UV vec2 rotated_center = ( opennessOffsetMatrix * center );

		// Vector to the current fragment. This is used to find the angle between
		// the trigonometric circle's origin and this vector. The angle is then
		// used to select a gradient color.
		P_UV vec2 V = ( rotated_texCoord - rotated_center );

		// Get the rotation of "V" from the origin.
		//
		//		tan( theta ) = ( opposite / adjacent ).
		//		tan( theta ) = ( V.y / V.x ).
		//
		// Therefore:
		//
		//		theta = atan( tan( theta ) ).
		//		theta = atan( V.y / V.x ).
		//
		// "theta" is the angle, in radians, between "V" and the rotation origin.
		//
		// Reference:
		// http://en.wikipedia.org/wiki/Polar_coordinate_system#Converting_between_polar_and_Cartesian_coordinates

		P_UV float V_rotation_in_radians = ( atan( V.y, V.x ) + kPI );

	// Get the colors to modulate.
	// We want to use texCoord instead of rotated_texCoord here because
	// we want to sample the texture at its unmodified coordinates.
	// rotated_texCoord is only useful to determine the transparency.
	P_COLOR vec4 color = ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );

	#if 0 // For debugging ONLY.

		// Blend with a plain color only.
		// Diregard the texture color sampled above.
		color = v_ColorScale;

	#elif 0 // For debugging ONLY.

		// Use debug colors only.
		if( V_rotation_in_radians <= feathering_lower_edge_in_radians )
		{
			return vec4( 1.0, 0.0, 0.0, 1.0 );
		}
		else if( V_rotation_in_radians >= feathering_upper_edge_in_radians )
		{
			return vec4( 0.0, 1.0, 0.0, 1.0 );
		}
		else // if( ( V_rotation_in_radians > feathering_lower_edge_in_radians ) && ( V_rotation_in_radians < feathering_upper_edge_in_radians ) )
		{
			return vec4( 0.0, 0.0, 1.0, 1.0 );
		}

	#endif

	// Set the transparency based on the angle of rotation from the origin.
	// This multiplies-in the alpha.
	return ( color * smoothstep( feathering_lower_edge_in_radians,
									feathering_upper_edge_in_radians,
									V_rotation_in_radians ) );
}
]]

return kernel
