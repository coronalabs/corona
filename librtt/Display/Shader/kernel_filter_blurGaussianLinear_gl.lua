local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "blurGaussianLinear"

kernel.graph = 
{
	nodes = {
		horizontal	=	{ effect="filter.blurLinearHorizontal",	input1="paint1" },
		vertical	=	{ effect="filter.blurLinearVertical",	input1="horizontal" },
	},
	output = "vertical",
}

return kernel
