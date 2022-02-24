// JS plugin is an chield object of 'window'
window.myplugin_js = 
{
	strProp: "Hello World!",
	u8StrProp: "👋 🌎!",
	boolProp: true,
	floadProp: 3.14,
	intProp: 42,
	arrProp: [1,2,3],
	objProp: { message:"Hello World", arr: [1,2,3]},

	skipArr: [1,2,3, function(){}, function(){}, 4, function(){}, 5, function(){}],
	skipObj: { message:"Hello World", f:function(){}, arr:[1,2,function(){},3], trailing:[function(){}] },

	empty: function(){},

	echo: function() {
		return JSON.stringify({data:Array.prototype.slice.call(arguments)});
	},

	echo1: function(e) {
		return e;
	},

	get: function(property) {
		return this[property];
	},

	set: function(property, value) {
		this[property] = value;
	},

	log: function() {
		console.log(this, arguments)
	},

	callSayHello: function(from) {
		return this.sayHello(from)
	},

	simpleCallbacker: function(cb) {
		var l = LuaCreateFunction(cb)
		l(42)
		LuaReleaseFunction(l);
	},


	withComplexCallbacks: function(f, obj) {
		try {
			if(LuaIsFunction(f)) {
				var l = LuaCreateFunction(f)
				l(42)
				LuaReleaseFunction(l);
			}
			if(LuaIsFunction(obj.func)) {
				var l = LuaCreateFunction(obj.func)
				l(42)
				LuaReleaseFunction(l);
			}
		} catch (error) {
			console.error("Error while testing functions: ", error)
			return false
		}
		return true
	},

	doubleFun: function(v, f) {
		f = LuaCreateFunction(f)
		var f2 = f(v)
		assert(LuaIsFunction(f2))
		f2 = LuaCreateFunction(f2)
		var ret = f2()
		LuaReleaseFunction(f2)
		LuaReleaseFunction(f)
		return ret
	},

	asyncTest: function(params) {
		assert(LuaIsFunction(params.callback))
		var callback = LuaCreateFunction(params.callback)
		setTimeout(() => {
			callback(params.param)
			LuaReleaseFunction(callback)
		}, params.delay);
	},
	


	r_strProp: function() { return "Hello World!" },
	r_u8StrProp: function() { return "👋 🌎!" },
	r_boolProp: function() { return true },
	r_floadProp: function() { return 3.14 },
	r_intProp: function() { return 42 },
	r_arrProp: function() { return [1,2,3] },
	r_objProp: function() { return { message:"Hello World", arr: [1,2,3]} },

};
