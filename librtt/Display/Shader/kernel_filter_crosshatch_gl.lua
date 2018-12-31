local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "crosshatch"

kernel.vertexData =
{
	{
		name = "grain",
		default = 0,
		min = 0,
		max = 1,
		index = 0, -- a_UserData.x
	},
}

kernel.vertex =
[[
varying P_UV float grain;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	grain = floor( a_UserData.x );

	return position;
}
]]

kernel.fragment =
[[
const P_UV float THRESHOLD = 1.0;

const P_UV vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

varying P_UV float grain;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	P_COLOR float luminance = dot( texColor.rgb, kWeights );

	if( luminance < 1.0 )
	{
		if( mod( ( gl_FragCoord.x + gl_FragCoord.y ), grain ) < THRESHOLD )
		{
			return v_ColorScale;
		}
	}

	if( luminance < 0.75 )
	{
		if( mod( ( gl_FragCoord.x - gl_FragCoord.y ), grain ) < THRESHOLD )
		{
			return v_ColorScale;
		}
	}

	if( luminance < 0.50 )
	{
		if( mod( ( gl_FragCoord.x + gl_FragCoord.y - 5.0 ), grain ) < THRESHOLD )
		{
			return v_ColorScale;
		}
	}

	if( luminance < 0.25 )
	{
		if( mod( ( gl_FragCoord.x - gl_FragCoord.y - 5.0 ), grain ) < THRESHOLD )
		{
			return v_ColorScale;
		}
	}

	return vec4( 0.0 );
}
]]

return kernel
