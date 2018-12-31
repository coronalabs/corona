local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "straighten"

kernel.vertexData =
{
	{
		name = "width",
		default = 1,
		min = 1,
		max = 99999,
		index = 0, -- a_UserData.x
	},
	{
		name = "height",
		default = 1,
		min = 1,
		max = 99999,
		index = 1, -- a_UserData.y
	},
	{
		name = "angle",
		default = 0,
		min = 0,
		max = 360,
		index = 2, -- a_UserData.z
	},
}

kernel.vertex =
[[
P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV float width = a_UserData.x;
	P_UV float height = a_UserData.y;
	P_UV float rotation_in_radians = radians( a_UserData.z );

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	P_UV float aspectRatio = ( width / height );

	//// Rotate and scale around the middle of the texture.
	//
	P_UV mat3 aspect_ratio_scale1 = mat3( 1.0, 0.0, 0.0,
											0.0, aspectRatio, 0.0,
											0.0, 0.0, 1.0 );

	// Translate the center of the texture to the origin.
	P_UV mat3 translate1 = mat3( 1.0, 0.0, 0.0,
									0.0, 1.0, 0.0,
									0.5, ( 0.5 * ( 1.0 / aspectRatio ) ), 1.0 );

	// Rotate.
	P_UV mat3 rotate;
	{
		P_UV float s = sin( rotation_in_radians );
		P_UV float c = cos( rotation_in_radians );

		rotate = mat3( c, ( - s ), 0.0,
						s, c, 0.0,
						0.0, 0.0, 1.0 );
	}

	// Scale.
	P_UV mat3 scale;
	{
		P_UV float max_aspectRatio = max( ( height / width ),
											( width / height ) );

		P_UV float scale_factor = ( abs( cos( rotation_in_radians ) ) +
										( max_aspectRatio *
											abs( sin( rotation_in_radians ) ) ) );

		scale_factor = ( 1.0 / scale_factor );

		scale = mat3( scale_factor, 0.0, 0.0,
						0.0, scale_factor, 0.0,
						0.0, 0.0, 1.0 );
	}

	P_UV mat3 aspect_ratio_scale0 = mat3( 1.0, 0.0, 0.0,
											0.0, ( 1.0 / aspectRatio ), 0.0,
											0.0, 0.0, 1.0 );

	// Translate the center of the texture back to its original position.
	P_UV mat3 translate0 = mat3( 1.0, 0.0, 0.0,
									0.0, 1.0, 0.0,
									-0.5, -0.5, 1.0 );

	P_UV mat3 transform = ( aspect_ratio_scale1 *
								translate1 *
								rotate *
								scale *
								aspect_ratio_scale0 *
								translate0 );
	//
	////

	// Override the default v_TexCoord.
	// IMPORTANT: "varying" values, like v_TexCoord, should NEVER be used
	// as temporary variables. They can ONLY be safely used as OUTPUT values.
	// ie: Write-only values.
	v_TexCoord.xy = ( transform * vec3( a_TexCoord.xy, 1.0 ) ).xy;

	return position;
}
]]

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{

	return ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );
}
]]

return kernel
