local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "bulge"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 1,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float intensity = v_UserData.x;

	// Convert from Cartesian coordiates to polar coordinates.
	//
	// Reference:
	// http://en.wikipedia.org/wiki/Polar_coordinate_system#Converting_between_polar_and_Cartesian_coordinates

		// This is the same as:
		// P_UV vec2 V_from_center_to_fragment = ( texCoord - vec2( 0.5, 0.5 ) );
		// P_UV float radius = length( V_from_center_to_fragment );
		P_UV float radius = sqrt( ( texCoord.x - 0.5 ) * ( texCoord.x - 0.5 ) + ( texCoord.y - 0.5 ) * ( texCoord.y - 0.5 ) );

		// This is the same as:
		// P_UV float angle = atan( V_from_center_to_fragment );
		P_UV float angle = atan( texCoord.x - 0.5, texCoord.y - 0.5 );

	// Tweak.
	P_UV float length = pow( radius, intensity );

	// Convert from polar coordinates to Cartesian coordiates.
	P_UV float u = length * cos( angle ) + 0.5;
	P_UV float v = length * sin( angle ) + 0.5;

	return texture2D( u_FillSampler0, vec2( v, u ) ) * v_ColorScale;
}
]]

return kernel
