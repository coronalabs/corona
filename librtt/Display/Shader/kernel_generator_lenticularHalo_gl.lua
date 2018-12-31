local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "lenticularHalo"

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

P_UV float sunbeam( P_RANDOM float seed,
						P_UV vec2 uv,
						P_POSITION vec2 pos )
{
	P_UV vec2 main = uv-pos;
	P_UV vec2 uvd = uv*(length(uv));

	// Polar coordinates.
	P_UV float ang = atan(main.x,main.y);
	P_UV float dist=length(main);
	dist = pow(dist, 0.1);
	
	P_UV float f0 = 1.0/(length(uv-pos)*16.0+1.0);

	P_UV float f_intermediate0 = ( ( pos.x + pos.y ) * 2.2 );
	P_UV float f_intermediate1 = ( ang * 4.0 );
	P_UV float f_intermediate2 = ( seed +
									f_intermediate0 +
									f_intermediate1 +
									5.954 );
	P_RANDOM float f_intermediate3 = ( sin( noise( f_intermediate2 ) * 32.0 ) * 0.5 );

	f0 += ( f0 * ( f_intermediate3 +
					( dist * 0.1 ) +
					0.0 ) );

	return f0;
}

P_COLOR float Hue_2_RGB(P_COLOR float v1, P_COLOR float v2, P_COLOR float vH )
{
	P_COLOR float ret;

	if ( vH < 0.0 )
		vH += 1.0;

	if ( vH > 1.0 )
		vH -= 1.0;

	if ( ( 6.0 * vH ) < 1.0 )
		ret = ( v1 + ( v2 - v1 ) * 6.0 * vH );
	else if ( ( 2.0 * vH ) < 1.0 )
		ret = ( v2 );
	else if ( ( 3.0 * vH ) < 2.0 )
		ret = ( v1 + ( v2 - v1 ) * ( ( 2.0 / 3.0 ) - vH ) * 6.0 );
	else
		ret = v1;

	return ret;
}

P_COLOR vec3 shift_hue( in P_COLOR vec3 input_color,
						in P_COLOR float hue_shift_in_degrees )
{
	// IMPORTANT: This hue-shifting function ISN'T exactly
	// the same as the one in kernel_filter_hue_gl.lua.

	P_COLOR float Cmax, Cmin;

	P_COLOR float D;

	P_COLOR float H, S, L;
	P_COLOR float R, G, B;

	R = input_color.r;
	G = input_color.g;
	B = input_color.b;

	// convert to HSL

	Cmax = max (R, max (G, B));
	Cmin = min (R, min (G, B));

	// calculate lightness
	L = (Cmax + Cmin) / 2.0;

	if (Cmax == Cmin) // it's grey
	{
		H = 0.0; // it's actually undefined
		S = 0.0;
	}
	else
	{
		D = Cmax - Cmin; // we know D != 0 so we cas safely divide by it

		// calculate Saturation
		if (L < 0.5)
		{
			S = D / (Cmax + Cmin);
		}
		else
		{
			S = D / (2.0 - (Cmax + Cmin));
		}

		// calculate Hue
		if (R == Cmax)
		{
			H = (G - B) / D;
		}
		else
		{
			if (G == Cmax)
			{
				H = 2.0 + (B - R) /D;
			}
			else
			{
				H = 4.0 + (R - G) / D;
			}
		}

		H = H / 6.0;
	}

	// modify H/S/L values
	// "-" : To change the direction of the shift to match what users expect.
	P_COLOR float hue_shift_in_radians = radians( hue_shift_in_degrees );
	H += hue_shift_in_radians;

	#if 0 // For testing ONLY.

		S = 1.0;
		L = 0.8;

	#endif

	if (H < 0.0)
	{
		H = H + 1.0;
	}

	// clamp H,S and L to [0..1]
	H = clamp(H, 0.0, 1.0);
	S = clamp(S, 0.0, 1.0);
	L = clamp(L, 0.0, 1.0);

	// convert back to RGB

	P_COLOR float var_2, var_1;

	if (S == 0.0)
	{
		R = L;
		G = L;
		B = L;
	}
	else
	{
		if ( L < 0.5 )
		{
			var_2 = L * ( 1.0 + S );
		}
		else
		{
			var_2 = ( L + S ) - ( S * L );
		}

		var_1 = 2.0 * L - var_2;

		R = Hue_2_RGB( var_1, var_2, H + ( 1.0 / 3.0 ) );
		G = Hue_2_RGB( var_1, var_2, H );
		B = Hue_2_RGB( var_1, var_2, H - ( 1.0 / 3.0 ) );
	}

	return vec3(R,G,B);
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_UV float aspectRatio = v_UserData.z;

	// Current fragment position in texture-space.
	// aspectRatio = ( object.width / object.height )
	P_UV vec2 position = vec2( ( v_UserData.x * aspectRatio ),
								v_UserData.y );

	// aspectRatio = ( object.width / object.height )
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 tc = vec2( ( texCoord.x * aspectRatio ),
							texCoord.y );

	P_RANDOM float seed = v_UserData.w;

	// Known issue: The intensity returned by sunbeam() varies with the "tc".
	P_UV float intensity = sunbeam( seed, tc, position );

	// Distance attenuation.
	P_UV float d = distance( tc, position );

	// Attenuate edges.
	// 2.0: The maximum diagonal distance squared. ie: ( 1^2 + 1^2 ).
	const P_UV float MAX_DIST = 1.4142135623730951; // sqrt( 2.0 ).
	intensity -= pow( ( ( 4.0 * d ) - MAX_DIST ), 2.0 );
	intensity = max( intensity, 0.0 );

	P_COLOR vec3 start_color = vec3( 1.0, 0.0, 0.0 );

	P_COLOR float hue_shift_in_degrees = ( ( max( ( d - 0.1 ), 0.0 ) / MAX_DIST ) * 1.4 * -180.0 );

	P_COLOR vec3 color = shift_hue( start_color, hue_shift_in_degrees );

	#if 0 // For debugging ONLY.

		return vec4( color, 1.0 );

	#else

		return vec4( color, 1.0 ) * intensity;

	#endif
#else // FRAGMENT_SHADER_SUPPORTS_HIGHP
	return vec4( 0 );
#endif // FRAGMENT_SHADER_SUPPORTS_HIGHP
}
]]

return kernel
