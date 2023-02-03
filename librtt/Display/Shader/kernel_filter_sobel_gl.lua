local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "sobel"

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// This is our 3 x 3 kernel.
	P_COLOR vec4 samples[9];

	samples[ 0 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, 0.0028125) );
	samples[ 1 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.00, 0.0028125) );
	samples[ 2 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, 0.0028125) );
	samples[ 3 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, 0.00 ) );
	samples[ 4 ] = texture2D( u_FillSampler0, texCoord.st ); // This will drive our alpha.
	samples[ 5 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, 0.0028125) );
	samples[ 6 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, -0.0028125) );
	samples[ 7 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.00, -0.0028125) );
	samples[ 8 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, -0.0028125) );

	// Horizontal and vertical weighting.
	//
	//     -1 -2 -1       1  0 -1
	// H =  0  0  0  V =  2  0 -2
	//      1  2  1       1  0 -1
	P_UV vec4 horiz_edge = samples[2] + ( 2.0 * samples[5]) + samples[8] - (samples[0] + (2.0*samples[3]) + samples[6]);
	P_UV vec4 vert_edge = samples[0] + ( 2.0 * samples[1]) + samples[2] - (samples[6] + (2.0*samples[7]) + samples[8]);

	return vec4( sqrt((horiz_edge.rgb * horiz_edge.rgb) + (vert_edge.rgb * vert_edge.rgb)), samples[ 4 ].a ) * v_ColorScale;
}
]]

return kernel
