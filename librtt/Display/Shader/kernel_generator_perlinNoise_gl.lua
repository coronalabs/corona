local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "perlinNoise"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.uniformData =
{
	{
		name = "color1",
		default = { 1, 0, 0, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "color2",
		default = { 0, 0, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 1, -- u_UserData1
	},
	{
		name = "scale",
		default = 8,
		min = 0,
		max = 99999,
		type="scalar",
		index = 2, -- u_UserData2
	},
}

kernel.vertex =
[[
uniform P_COLOR vec4 u_UserData0; // color1
uniform P_COLOR vec4 u_UserData1; // color2
uniform P_COLOR float u_UserData2; // scale

varying P_COLOR vec4 colorDiff;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_COLOR vec4 color1 = u_UserData0;
	P_COLOR vec4 color2 = u_UserData1;
	P_COLOR float scale = u_UserData2;

	colorDiff = ( color2 - color1 );

	return position;
}
]]

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // color1
uniform P_COLOR vec4 u_UserData1; // color2
uniform P_COLOR float u_UserData2; // scale

varying P_COLOR vec4 colorDiff;

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
//		kernel_filter_frostedGlass_gl.lua
//		kernel_filter_marble_gl.lua
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

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 color1 = u_UserData0;
	P_COLOR vec4 color2 = u_UserData1;
	P_COLOR float scale = u_UserData2;

	//WE *SHOULD* BE ABLE TO MOVE ( texCoord * scale ) TO THE VERTEX SHADER!!!
	P_COLOR float n1 = ( ( cnoise( texCoord * scale ) + 1.0 ) / 2.0 );

	return ( ( color1 + ( colorDiff * n1 ) ) * v_ColorScale );
}
]]

return kernel
