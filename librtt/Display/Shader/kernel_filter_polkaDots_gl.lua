local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "polkaDots"

kernel.vertexData =
{
	{
		name = "numPixels",
		default = 4,
		min = 4,
		max = 99999,
		index = 0, -- a_UserData.x
	},
	{
		name = "dotRadius",
		default = 1,
		min = 0,
		max = 1,
		index = 1, -- a_UserData.y
	},
	{
		name = "aspectRatio",
		default = 1,
		min = 0,
		max = 99999,
		index = 2, -- v_UserData.z
	},
}

kernel.vertex =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;
varying P_UV float minimum_full_radius;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV float numPixels = a_UserData.x;
	P_UV float dotRadius = a_UserData.y;

	// "u_TexelSize.xy" is in pixels.
	// "u_TexelSize.zw" is in content units.
	slot_size = ( u_TexelSize.zw * numPixels );

	// This is used to sample from the middle of the slot.
	sample_uv_offset = ( slot_size * 0.5 );

	minimum_full_radius = ( ( min( slot_size.x, slot_size.y ) * 0.5 ) * dotRadius );

	return position;
}
]]

kernel.fragment =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;
varying P_UV float minimum_full_radius;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float aspectRatio = v_UserData.z;

	// aspectRatio = ( object.width / object.height )
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 tc = vec2( ( texCoord.x * aspectRatio ),
							texCoord.y );

	P_UV vec2 uv = ( sample_uv_offset + ( floor( tc / slot_size ) * slot_size ) );

	// Distance from the current pixel to the sampling point
	// (center of the circle).
	P_UV float dist = distance( tc, uv );

	//// Brightness.
	//
	// We want maximum brightness near the origin.
	// We want minimum brightness near the edges of the star.
	P_UV float unitized_dist = ( ( minimum_full_radius - dist ) / minimum_full_radius );

	// Use exponential ease-out to smooth the edges.
	// We could use "smoothstep()" instead, to have smooth edge of a
	// specific width.
	P_UV float brightness = ( 1.0 - pow( 2.0, ( -10.0 * unitized_dist ) ) );
	//
	////

	//// Visibility.
	//
	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	P_UV float visibility = step( dist, minimum_full_radius );
	//
	////

	P_COLOR vec4 color = texture2D( u_FillSampler0, uv );

	return ( color * v_ColorScale * visibility * brightness );
}
]]

return kernel
