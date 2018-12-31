local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "hue"

kernel.vertexData =
{
	{
		name = "angle",
		default = 0,
		min = 0,
		max = 360,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec3 rgb2hsv( P_COLOR vec3 c )
{
	P_RANDOM vec4 K = vec4( 0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0 );
	P_RANDOM vec4 p = mix( vec4( c.bg, K.wz ), vec4( c.gb, K.xy ), step( c.b, c.g ) );
	P_RANDOM vec4 q = mix( vec4( p.xyw, c.r ), vec4( c.r, p.yzx ), step( p.x, c.r ) );

	P_RANDOM float d = ( q.x - min( q.w, q.y ) );
	P_RANDOM float e = 1.0e-10;

	return vec3( abs( q.z + ( q.w - q.y ) / ( 6.0 * d + e ) ), d / ( q.x + e ), q.x );
}

P_COLOR vec3 hsv2rgb( P_COLOR vec3 c )
{
	P_RANDOM vec4 K = vec4( 1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0 );
	P_RANDOM vec3 p = abs( fract( c.xxx + K.xyz ) * 6.0 - K.www );

	return c.z * mix( K.xxx, clamp( p - K.xxx, 0.0, 1.0 ), c.y );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	// texColor has pre-multiplied alpha.
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	if ( texColor.a == 0.0 )
	{
		return texColor;
	}

	// Remove the alpha from the texColor.
	P_COLOR vec4 texColorWithoutPremultipliedAlpha = vec4( ( texColor.rgb / texColor.a ), texColor.a );

	//// Shift the hue.
	//
	P_COLOR vec3 hsv = rgb2hsv( texColorWithoutPremultipliedAlpha.rgb );

	hsv.r += ( v_UserData.x / -360.0 );

	P_COLOR vec3 rgb = hsv2rgb( hsv );
	//
	////

	// Re-apply the pre-multiplied alpha.
	P_COLOR vec4 rgbWithPremultipliedAlpha = vec4( rgb * texColor.a, texColor.a );

	return ( rgbWithPremultipliedAlpha * v_ColorScale );
#else
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	return texColor * v_ColorScale;
#endif
}
]]

return kernel
