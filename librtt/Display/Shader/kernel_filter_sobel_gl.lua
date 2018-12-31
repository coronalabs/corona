local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "sobel"

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// This is our 3 x 3 kernel.
	P_COLOR vec4 sample[9];

	sample[ 0 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, 0.0028125) );
	sample[ 1 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.00, 0.0028125) );
	sample[ 2 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, 0.0028125) );
	sample[ 3 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, 0.00 ) );
	sample[ 4 ] = texture2D( u_FillSampler0, texCoord.st ); // This will drive our alpha.
	sample[ 5 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, 0.0028125) );
	sample[ 6 ] = texture2D( u_FillSampler0, texCoord.st + vec2(-0.0028125, -0.0028125) );
	sample[ 7 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.00, -0.0028125) );
	sample[ 8 ] = texture2D( u_FillSampler0, texCoord.st + vec2(0.0028125, -0.0028125) );

	// Horizontal and vertical weighting.
	//
	//     -1 -2 -1       1  0 -1
	// H =  0  0  0  V =  2  0 -2
	//      1  2  1       1  0 -1
	P_UV vec4 horiz_edge = sample[2] + ( 2.0 * sample[5]) + sample[8] - (sample[0] + (2.0*sample[3]) + sample[6]);
	P_UV vec4 vert_edge = sample[0] + ( 2.0 * sample[1]) + sample[2] - (sample[6] + (2.0*sample[7]) + sample[8]);

	return vec4( sqrt((horiz_edge.rgb * horiz_edge.rgb) + (vert_edge.rgb * vert_edge.rgb)), sample[ 4 ].a ) * v_ColorScale;
}
]]

return kernel
