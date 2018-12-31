local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "crystallize"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "numTiles",
		default = 16,
		min = 2,
		max = 99999,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
const highp float FLT_MAX = 1e38;
/*
P_RANDOM float unit_rand_1d( P_RANDOM float p )
{
	return fract( sin( p ) * 43758.5453 );
}
*/
P_RANDOM vec2 unit_rand_2d( in P_RANDOM vec2 p )
{
	p = vec2( dot( p, vec2( 127.1, 311.7 ) ),
				dot( p, vec2( 269.5, 183.3 ) ) );

	return fract( sin( p ) * 43758.5453 );
}

P_UV vec2 get_voronoi_tc( in P_UV vec2 p,
							in P_UV float numTiles )
{
	P_UV vec2 n = floor(p * numTiles);
	P_UV vec2 f = fract(p * numTiles);

	P_RANDOM vec2 seed;
	highp float min_dist_squared = FLT_MAX;

	for( int j=-1; j<=1; j++ )
	{
		for( int i=-1; i<=1; i++ )
		{
			P_UV vec2 constant_unit_offset = vec2(float(i),float(j));

			P_RANDOM vec2 random_unit_offset = unit_rand_2d( n + constant_unit_offset );

			P_RANDOM vec2 random_offset = ( constant_unit_offset + random_unit_offset );

			P_RANDOM vec2 r = ( random_offset - f );

			highp float dist_squared = dot( r, r );

			#if 0

				// Branching version.

				if( dist_squared < min_dist_squared )
				{
					min_dist_squared = dist_squared;

					seed = ( n + random_offset);
				}

			#else

				// Branchless version.

				highp float useNewValue = step( dist_squared, min_dist_squared );
				highp float useOldValue = ( 1.0 - useNewValue );

				// useNewValue = 0 : min_dist_squared = min_dist_squared. (No change.)
				// useNewValue = 1 : min_dist_squared = dist_squared. (Update the minimum.)
				min_dist_squared = ( ( useNewValue * dist_squared ) +
										( useOldValue * min_dist_squared ) );

				// useNewValue = 0 : seed = seed. (No change.)
				// useNewValue = 1 : seed = ( n + random_offset). (Update the seed.)
				seed = ( ( useNewValue * ( n + random_offset) ) +
							( useOldValue * seed ) );

			#endif
		}
	}

	return ( seed * ( 1.0 / numTiles ) );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// Tile count. MUST be greater than 1.0 (ie: 2.0 or greater).
	P_UV float numTiles = v_UserData.x;

	P_UV vec2 tc = get_voronoi_tc( texCoord, numTiles );

	#if 0 // For debugging ONLY.

		// Return a solid color to represent the center of the crystallize.

		P_UV float voronoi_distance = distance( tc, texCoord );

		// We DON'T want this to be proportional to u_TexelSize because
		// we want the circle to be of a constant size, NOT proportional
		// to the texture resolution.

		if( voronoi_distance < ( 1.0 / ( numTiles * 16 ) ) )
		{
			return vec4( 0.0, 0.0, 0.0, 1.0 );
		}
		else if( voronoi_distance < ( 1.0 / ( numTiles * 8 ) ) )
		{
			return vec4( 1.0, 1.0, 1.0, 1.0 );
		}

	#endif

	P_COLOR vec4 color = texture2D( u_FillSampler0, tc );

	return ( color * v_ColorScale );
}
]]

return kernel
