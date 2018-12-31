local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "sunbeams"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "posX",
		default = 0.5,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
	{
		name = "posY",
		default = 0.5,
		min = 0,
		max = 1,
		index = 1, -- v_UserData.y
	},
	{
		name = "aspectRatio",
		default = 1,
		min = 0,
		max = 99999,
		index = 2, -- v_UserData.z
	},
	{
		name = "seed",
		default = 0,
		min = 0,
		max = 99999,
		index = 3, -- v_UserData.w
	},
}

kernel.fragment =
[[
P_RANDOM float unit_rand_1d( in P_RANDOM float v )
{
	return fract( sin( v ) * 43758.5453 );
}

P_RANDOM vec2 unit_rand_2d( in P_RANDOM vec2 v )
{
	v = vec2( dot( v, vec2( 127.1, 311.7 ) ),
				dot( v, vec2( 269.5, 183.3 ) ) );

	return fract( sin( v ) * 43758.5453 );
}

P_RANDOM float noise( in P_RANDOM float v )
{
	P_RANDOM float v0 = unit_rand_1d( floor( v ) );
	P_RANDOM float v1 = unit_rand_1d( ceil( v ) );
	P_RANDOM float m = fract( v );
	P_RANDOM float p = mix( v0, v1, m );

	return p;
}

P_COLOR float sunbeam( P_RANDOM float seed,
						P_UV vec2 uv,
						P_POSITION vec2 pos )
{
	P_UV vec2 main = uv-pos;
	P_UV vec2 uvd = uv*(length(uv));

	// Polar coordinates.
	P_POSITION float ang = atan(main.x,main.y);
	P_POSITION float dist=length(main);
	dist = pow(dist, 0.1);
	
	P_UV float f0 = 1.0/(length(uv-pos)*16.0+1.0);

	P_UV float f_intermediate0 = ( ( pos.x + pos.y ) * 2.2 );
	P_UV float f_intermediate1 = ( ang * 4.0 );
	P_UV float f_intermediate2 = ( seed +
									f_intermediate0 +
									f_intermediate1 +
									5.954 );
	P_RANDOM float f_intermediate3 = ( sin( noise( f_intermediate2 ) * 16.0 ) * 0.1 );

	f0 += ( f0 * ( f_intermediate3 +
					( dist * 0.1 ) +
					0.8 ) );

	return f0;
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_POSITION float aspectRatio = v_UserData.z;

	// aspectRatio = ( object.width / object.height )
	P_POSITION vec2 center_pos = vec2( ( v_UserData.x * aspectRatio ),
									v_UserData.y );

	// Current fragment position in texture-space.
	P_UV vec2 tc = vec2( ( texCoord.x * aspectRatio ),
							texCoord.y );

	P_RANDOM float seed = v_UserData.w;

	// Known issue: The intensity returned by sunbeam() varies with the "tc".
	// This has the effect of making the sunbeam look like it's slightly
	// rotating when the center of the sunbeam is moved.
	P_COLOR float intensity = sunbeam( seed, tc, center_pos );

	// Distance attenuation.
	P_UV float d = distance( tc, center_pos );
	intensity -= ( 0.5 * d );

	#if 0 // For debugging ONLY.

		return vec4( intensity );

	#else

		return ( vec4( 1.4, 1.2, 1.0, 1.0 ) * intensity );

	#endif
#else // FRAGMENT_SHADER_SUPPORTS_HIGHP
	return vec4( 0 );
#endif // FRAGMENT_SHADER_SUPPORTS_HIGHP
}
]]

return kernel
