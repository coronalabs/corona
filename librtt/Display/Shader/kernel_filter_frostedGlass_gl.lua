local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "frostedGlass"

kernel.isTimeDependent = true

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "scale",
		default = 64,
		min = 1,
		max = 99999,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_RANDOM vec4 mod289( in P_RANDOM vec4 x )
{
    return ( x - floor( x * ( 1.0 / 289.0 ) ) * 289.0 );
}
 
P_RANDOM vec4 permute( in P_RANDOM vec4 x )
{
    return mod289( ( ( x * 34.0 ) + 1.0 ) * x );
}

P_RANDOM vec4 taylorInvSqrt( in P_RANDOM vec4 r )
{
    return ( 1.79284291400159 - ( 0.85373472095314 * r ) );
}
 
P_RANDOM vec2 fade( in P_RANDOM vec2 t )
{
    return ( t * t * t * ( t * ( t * 6.0 - 15.0 ) + 10.0 ) );
}

// This function is duplicated in these:
//		kernel_filter_marble_gl.lua
//		kernel_filter_perlinNoise_gl.lua
// Classic Perlin noise
P_RANDOM float cnoise( in P_RANDOM vec2 P )
{
    P_RANDOM vec4 P_i = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    P_RANDOM vec4 P_f = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    P_i = mod289(P_i); // To avoid truncation effects in permutation
    P_RANDOM vec4 i_x = P_i.xzxz;
    P_RANDOM vec4 i_y = P_i.yyww;
    P_RANDOM vec4 f_x = P_f.xzxz;
    P_RANDOM vec4 f_y = P_f.yyww;
     
    P_RANDOM vec4 i = permute(permute(i_x) + i_y);
     
    P_RANDOM vec4 g_x = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    P_RANDOM vec4 g_y = abs(g_x) - 0.5;
    P_RANDOM vec4 t_x = floor(g_x + 0.5);
    g_x = g_x - t_x;

    P_RANDOM vec2 g_00 = vec2(g_x.x,g_y.x);
    P_RANDOM vec2 g_10 = vec2(g_x.y,g_y.y);
    P_RANDOM vec2 g_01 = vec2(g_x.z,g_y.z);
    P_RANDOM vec2 g_11 = vec2(g_x.w,g_y.w);
     
	P_RANDOM vec4 norm = taylorInvSqrt( vec4( dot( g_00, g_00 ),
											dot( g_01, g_01 ),
											dot( g_10, g_10 ),
											dot( g_11, g_11 ) ) );
    g_00 *= norm.x;  
    g_01 *= norm.y;  
    g_10 *= norm.z;  
    g_11 *= norm.w;  
     
    P_RANDOM float n_00 = dot(g_00, vec2(f_x.x, f_y.x));
    P_RANDOM float n_10 = dot(g_10, vec2(f_x.y, f_y.y));
    P_RANDOM float n_01 = dot(g_01, vec2(f_x.z, f_y.z));
    P_RANDOM float n_11 = dot(g_11, vec2(f_x.w, f_y.w));
     
    P_RANDOM vec2 fade_xy = fade(P_f.xy);
    P_RANDOM vec2 n_x = mix(vec2(n_00, n_01), vec2(n_10, n_11), fade_xy.x);
    P_RANDOM float n_xy = mix(n_x.x, n_x.y, fade_xy.y);

    return 2.3 * n_xy;
}

P_NORMAL vec3 get_fragment_normal( in P_NORMAL vec2 texCoord,
								in P_NORMAL vec4 texelSize,
								in P_NORMAL float scale )
{
	// We're generating the fragment normal from 3 height values
	// sampled from a Perlin-noise-based height map.

	P_NORMAL vec2 texCoord0 = texCoord;
	P_NORMAL vec2 texCoord1 = texCoord + vec2( texelSize.x, 0.0 );
	P_NORMAL vec2 texCoord2 = texCoord + vec2( 0.0, texelSize.y );

	P_NORMAL float height0 = ( ( cnoise( texCoord0 * scale ) + 1.0 ) * 0.5 );
	P_NORMAL float height1 = ( ( cnoise( texCoord1 * scale ) + 1.0 ) * 0.5 );
	P_NORMAL float height2 = ( ( cnoise( texCoord2 * scale ) + 1.0 ) * 0.5 );

	P_NORMAL vec3 v0 = vec3( texCoord0, height0 );
	P_NORMAL vec3 v1 = vec3( texCoord1, height1 );
	P_NORMAL vec3 v2 = vec3( texCoord2, height2 );

	P_NORMAL vec3 n = cross( ( v1 - v0 ),
							( v2 - v0 ) );

	return normalize( n );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float scale = v_UserData.x;

	P_NORMAL vec3 n = get_fragment_normal( texCoord,
											u_TexelSize,
											scale );

	P_NORMAL vec3 incident = vec3( 0.0, 0.0, 1.0 );
	P_NORMAL float intensity = dot( n, incident );

	#if 0 // For debugging ONLY.

		// This will repeat the texture hozirontally,
		// and scroll it from right to left.
		texCoord.x = fract( texCoord.x + ( u_TotalTime * 0.1 ) );

	#endif

	// This ISN'T what we want.
	//P_NORMAL vec3 r = refract( vec3( 0.0, 0.0, 1.0 ), n, 1.3330 );

	//! TOFIX: We SHOULDN'T hard code "0.01".
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord + ( n.xy * 0.01 ) );

	#if 0 // Experiment.

		return ( vec4( ( texColor.rgb * ( 0.5 + intensity ) ), 1.0 ) * v_ColorScale );

	#endif

	return texColor;
}
]]

return kernel
