local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "opTile"

kernel.vertexData =
{
	{
		name = "numPixels",
		default = 8,
		min = 0,
		max = 99999,
		index = 0, -- a_UserData.x
	},
	{
		name = "angle",
		default = 0,
		min = 0,
		max = 360,
		index = 1, -- a_UserData.y
	},
	{
		name = "scale",
		default = 2.8,
		min = 0,
		max = 99999,
		index = 2, -- a_UserData.z
	},
}

kernel.vertex =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;
// Some Android devices AREN'T able to use "varying mat3".
varying P_UV vec3 transform0;
varying P_UV vec3 transform1;
varying P_UV vec3 transform2;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV float numPixels = a_UserData.x;
	P_UV float rotation_in_radians = radians( a_UserData.y );

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	////
	//
	// "u_TexelSize.xy" is in pixels.
	// "u_TexelSize.zw" is in content units.
	slot_size = ( u_TexelSize.zw * numPixels );

	// This is used to sample from the middle of the slot.
	sample_uv_offset = ( slot_size * 0.5 );
	//
	////

	//// Rotate around the middle of the texture.
	//
	P_UV mat3 translate0;
	P_UV mat3 rotate;
	P_UV mat3 translate1;

	// Translate the center of the texture to the origin.
	translate0 = mat3( 1.0, 0.0, 0.0,
						0.0, 1.0, 0.0,
						-0.5, -0.5, 1.0 );

	// Rotate.
	{
		P_UV float s = sin( rotation_in_radians );
		P_UV float c = cos( rotation_in_radians );

		rotate = mat3( c, ( - s ), 0.0,
						s, c, 0.0,
						0.0, 0.0, 1.0 );
	}

	// Translate the center of the texture back to its original position.
	translate1 = mat3( 1.0, 0.0, 0.0,
						0.0, 1.0, 0.0,
						0.5, 0.5, 1.0 );

	P_UV mat3 transform = ( translate1 * rotate * translate0 );
	transform0 = transform[ 0 ];
	transform1 = transform[ 1 ];
	transform2 = transform[ 2 ];
	//
	////

	return position;
}
]]

kernel.fragment =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;
// Some Android devices AREN'T able to use "varying mat3".
varying P_UV vec3 transform0;
varying P_UV vec3 transform1;
varying P_UV vec3 transform2;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float scale = v_UserData.z;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	P_UV mat3 transform;
	transform[ 0 ] = transform0;
	transform[ 1 ] = transform1;
	transform[ 2 ] = transform2;

	// Apply the "angle" parameter.
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 tc = ( transform * vec3( texCoord, 1.0 ) ).xy;

	// IDEA: WE COULD ONLY ROTATE THE CENTRAL SAMPLING POINT,
	// AND KEEP THE ADJACENT PIXELS SAMPLED AXIS-ALIGNED!!!!!
	P_UV vec2 center_uv = ( sample_uv_offset + ( floor( tc / slot_size ) * slot_size ) );

	// Apply the "scale" parameter.
	P_UV vec2 uv = mix( center_uv, tc, scale );

	return ( texture2D( u_FillSampler0, uv ) * v_ColorScale );
}
]]

return kernel
