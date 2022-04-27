//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

'use strict';

var platformLibrary =
{
	// globals 
	$jsLocaleLanguage: null,
	$jsLocaleCountry: null,
	$jsLanguage: null,

	jsOpenURL: function (_url) {
		var url = UTF8ToString(_url);
		var win = window.open(url, 'Corona Labs');
		if (win) {
			//Browser has allowed it to be opened
			win.focus();
			return true;
		}
		else {
			//Browser has blocked it
			Module.printErr('Error: Failed not open\n' + url + '\nTry to disable popup blockers');
		}
		return false;
	},

	jsShowAlert: function (_title, _msg, buttons, numButtons, _resource, pointerSize) {
		var title = UTF8ToString(_title);
		var msg = UTF8ToString(_msg);
		var resource = _resource;

		// sanity check, close old popup window
		var obj = document.getElementById('showAlert');
		if (obj) {
			obj.close();
		}

		var awindow = document.createElement('div');
		awindow.id = 'showAlert';
		awindow.close = function () {
			_jsAlertCallback(-1, resource);
			awindow.remove();
		};

		var rect = canvas.getBoundingClientRect();
		awindow.style.width = '+' + rect.width / 2 + 'px';

		var span = document.createElement('div');
		awindow.appendChild(span);
		span.style.color = "grey";
		span.style.marginTop = "10px";
		span.style.marginLeft = "10px";
		span.appendChild(document.createTextNode(title));

		var span = document.createElement('div');
		awindow.appendChild(span);
		span.style.color = "black";
		span.style.textAlign = "center";
		span.style.marginLeft = "10px";
		span.style.marginRight = "10px";
		span.style.marginBottom = "10px";

		var lines = msg.split('\n');
		for (var i = 0; i < lines.length; i++) {
			var t = document.createTextNode(lines[i]);
			span.appendChild(document.createElement("br"));
			span.appendChild(t);
		}

		// close button
		var b = document.createElement('button');
		awindow.appendChild(b);
		b.style = "top:0;right:0;position:absolute;z-index: 9999"
		b.appendChild(document.createTextNode("X"));
		b.addEventListener("click", function () {
			_jsAlertCallback(-1, resource);
			awindow.remove();
		});

		// add buttons
		for (var i = 0; i < numButtons; i++)
		{
			var _caption = getValue(buttons + i * pointerSize, '*');
			var caption = UTF8ToString(_caption);

			var b = document.createElement('button');
			awindow.appendChild(b);
			b.style.marginTop = "20px";
			b.style.marginBottom = "10px";
			b.style.marginLeft = "10px";
			b.appendChild(document.createTextNode(caption));
			b.index = i;
			b.addEventListener("click", function () {
				_jsAlertCallback(this.index, resource);
				awindow.remove();
			});
		}

//		document.body.appendChild(awindow);
		canvas.parentNode.appendChild(awindow);

		// move to center
		awindow.style.backgroundColor = "lightgrey";
		awindow.style.position = "fixed";
		awindow.style.left = '50%';
		awindow.style.top = '50%';
		awindow.style.marginLeft = '-' + Math.floor(rect.width / 4) + 'px';
		awindow.style.marginTop = '-' + Math.floor(rect.height / 4) + 'px';

		return 0;
	},

	jsGetPreference: function (_key, _valPtr) {
		if (typeof (Storage) != "undefined") {
			var key = UTF8ToString(_key);		// key
			var valPtr = _valPtr;		// char*
			val = localStorage.getItem(key);
			if (val) {
				var maxBytesToWrite = 1024;		// hack
				stringToUTF8(val, valPtr, maxBytesToWrite);
				return true;
			}
			return false;
		}
		console.log('Sorry! No Web Storage support..');
		return false;
	},

	jsSetPreference: function (_key, _valPtr) {
		if (typeof (Storage) != "undefined") {
			var key = UTF8ToString(_key);		// char*
			var val = UTF8ToString(_valPtr);		// char*
			localStorage.setItem(key, val);
			return true;
		}
		console.log('Sorry! No Web Storage support..');
		return false;
	},

	jsDeletePreference: function (_key) {
		if (typeof (Storage) != "undefined") {
			var key = UTF8ToString(_key);		// char*
			localStorage.removeItem(key);
			return true;
		}
		console.log('Sorry! No Web Storage support..');
		return false;
	},


	jsSetActivityIndicator: function (visible) {
		var canvas = document.getElementById('canvas');
		canvas.style.cursor = visible ? 'wait' : 'auto';
	},

	// caller must free(p)
	jsGetLocaleLanguage: function()
	{
		var localization = window.navigator.userLanguage || window.navigator.language;	// en-US
		var a = localization.split('-');
		var s = a[0];

		var maxBytesToWrite = 64;
		if (jsLocaleLanguage == null)
		{
			jsLocaleLanguage = Module._malloc(maxBytesToWrite);
		}
		stringToUTF8(s, jsLocaleLanguage, maxBytesToWrite);
		return jsLocaleLanguage;
	},

	// caller must free(p)
	jsGetLocaleCountry: function ()
	{
		var localization = window.navigator.userLanguage || window.navigator.language;	// en-US
		var a = localization.split('-');
		var s = a[1] || localization;

		var maxBytesToWrite = 64;
		if (jsLocaleCountry == null)
		{
			jsLocaleCountry = Module._malloc(maxBytesToWrite);
		}
		stringToUTF8(s, jsLocaleCountry, maxBytesToWrite);
		return jsLocaleCountry;
	},

	// caller must free(p)
	jsGetLanguage: function ()
	{
		var localization = window.navigator.userLanguage || window.navigator.language;	// en-US
		var a = localization.split('-');
		var s = a[0] + '_' + (a[1] || a[0]).toUpperCase();

		var maxBytesToWrite = 64;
		if (jsLanguage == null)
		{
			jsLanguage = Module._malloc(maxBytesToWrite);
		}
		stringToUTF8(s, jsLanguage, maxBytesToWrite);
		return jsLanguage;
	},

	//
	//	native objects
	//

	jsDisplayObjectDelete: function (id) {
		var obj = document.getElementById(id);
		if (obj) {
			//console.log('JS delete', obj.type, 'id=', obj.id);
			obj.remove();
		}
	},

	jsDisplayObjectShowControls: function(id, val) {
		var obj = document.getElementById(id);
		if (obj) {
			obj.controls = id;
		}
	},

	jsDisplayObjectSetVisible: function (id, val) {
		var obj = document.getElementById(id);
		if (obj) {
			obj.style.visibility = (val == true) ? 'visible' : 'hidden';
		}
	},

	jsDisplayObjectHasBackground: function (id, val) {
		var obj = document.getElementById(id);
		if (obj) {
			obj.style.backgroundColor = (val == true) ? 'white' : 'transparent';
			obj.style.borderColor = (val == true) ? 'white' : 'transparent';
		}
	},

	jsDisplayObjectsetBackgroundColor: function(id, val) {
		var obj = document.getElementById(id);
		if (obj) {
			var rgba = UTF8ToString(val);
			obj.style.backgroundColor = rgba;
		}
	},
	
	jsDisplayObjectGetVisible: function (id) {
		var obj = document.getElementById(id);
		if (obj && obj.style.visibility == 'hidden') {
			return true;
		}
		return false;
	},

	jsTextFieldSetValue: function(id, s) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.value = UTF8ToString(s);
		}
	},

	jsDisplayObjectCreate: function (x, y, w, h, etype, thiz) {
		var fType = UTF8ToString(etype);
		var obj = document.createElement(fType);
		obj.id = Module.appElementCounter++;
		obj.type = fType;
		obj.thiz = thiz;
		canvas.parentNode.appendChild(obj);

		obj.x = x + 1;	// +1 border
		obj.y = y + 1;	// +1 border
		obj.w = w - 2;	// -2 border
		obj.h = h - 2;	// -2 border
		obj.style.position = "absolute";
		obj.style.width = obj.w + 'px';
		obj.style.height = obj.h + 'px';
		obj.style.borderWidth = "1px 1px 1px 1px";
		
		window.refreshNativeObject(obj.id);
		// console.log('JS create', fType, 'id=', obj.id, x,y,w,h);
		return obj.id;
	},

	jsDisplayObjectSetBounds: function (id, x, y, w, h) {
		var obj = document.getElementById(id);
		if (obj) {
			obj.x = x + 1;	// +1 border
			obj.y = y + 1;	// +1 border
			obj.w = w - 2;	// -2 border
			obj.h = h - 2;	// -2 border
			obj.style.width = obj.w + 'px';
			obj.style.height = obj.h + 'px';
			window.refreshNativeObject(obj.id);
		}
	},

	jsTextFieldInitialize: function(id)	{
		var obj = document.getElementById(id);
		if (obj) {
			obj.oldText = obj.value;

			// disable SDL keyboard handler and enable native JS handler 
			obj.onfocus = function (e) {
				// dispatch event
				_jsEnableKeyboard(0);
				_jsTextBoxCallback(this.thiz, 1);		// began
			}

			// enable SDL keyboard handler and disable native JS handler
			obj.onblur = function (e) {
				// dispatch event
				_jsTextBoxCallback(this.thiz, 3);		// ended
				_jsEnableKeyboard(1);
			}

			// the input event triggers every time a value is modified.
			obj.oninput = function (e) {
				// dispatch event
				_jsTextBoxCallback(this.thiz, 2);		// editing
			}

			obj.onkeydown = function (e) {
				this.oldText = this.value;
			}

			obj.onkeyup = function (e) {
				if (e.keyCode === 13) {
					// dispatch event
					_jsTextBoxCallback(this.thiz, 4);		// submit
				}
			}
		}
	},
	
	jsTextFieldGetValue: function (id, buf, bufsize) {
		var obj = document.getElementById(id);
		if (obj) {
			stringToUTF8(obj.value, buf, bufsize);
		}
	},

	jsTextFieldGetStringProperty: function (id, buf, bufsize, _name) {
		var name = UTF8ToString(_name);
		var obj = document.getElementById(id);
		if (obj) {
			stringToUTF8(obj[name], buf, bufsize);
		}
	},

	jsTextFieldGetIntProperty: function (id, _name) {
		var name = UTF8ToString(_name);
		var obj = document.getElementById(id);
		if (obj) {
			return Number(obj[name]);
		}
		return 0;
	},

	jsTextFieldGetFontSize: function(id) {
		var obj = document.getElementById(id);
		if (obj)
		{
			var style = window.getComputedStyle(obj, null).getPropertyValue('font-size');
			return parseFloat(style);
		}
		return 0;
	},

	jsTextFieldSetFontSize: function(id, size) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.style.fontSize = (size) + 'px';
		}
	},

	jsTextFieldGetAlign: function(id, buf, size) { 
		var obj = document.getElementById(id);
		if (obj)
		{
			stringToUTF8(obj.style.textAlign, buf, size);
		}
	},

	jsTextFieldSetAlign: function(id, align) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.style.textAlign = UTF8ToString(align);
		}
	},

	jsTextFieldGetSecure: function(id, buf, size) {
		var obj = document.getElementById(id);
		if (obj)
		{
			stringToUTF8(obj.type, buf, size);
		}
	},

	jsTextFieldGetInputType: function(id, buf, size) {
		var obj = document.getElementById(id);
		if (obj)
		{
			stringToUTF8(obj.type, buf, size);
		}
	},

	jsTextFieldSetInputType: function(id, inputType) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.type = UTF8ToString(inputType);
		}
	},

	jsTextFieldGetEditable: function(id) {
		var obj = document.getElementById(id);
		if (obj)
		{
			return obj.readonly;
		}
		return false;
	},

	jsTextFieldSetEditable: function(id, isEditable) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.readonly = isEditable;
		}
	},

	jsTextFieldSetFont: function(id, name, size) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.style.fontFamily = UTF8ToString(name);
			obj.style.fontSize = (size) + 'px';
		}
	},

	jsTextFieldSetPlaceholder: function(id, text) {
		var obj = document.getElementById(id);
		if (obj)
		{
			var s = UTF8ToString(text);
			obj.placeholder = s;
		}
	},

	jsTextFieldSetSecure: function(id, secure) {
		var obj = document.getElementById(id);
		if (obj)
		{
			obj.type = secure ? "password" : "text";
		}
	},

	jsTextFieldSetColor: function(id, _rgb) {
		var obj = document.getElementById(id);
		if (obj)
		{
			var rgb = UTF8ToString(_rgb);
			obj.style.color = rgb;
		}
	},

	//
	// context
	//

	jsContextLoadFonts: function(_name, buf, size)
	{
		var name = UTF8ToString(_name);
		var body = new Uint8Array(size);
		for (var i = 0; i < size; i++) {
			body[i] = getValue(buf + i, 'i8');
		}

		var a = name.split('/');
		var b = a[a.length - 1];		// filename
		var c = b.split('.');		// name + type
		if (c.length < 2) {
			console.log('jsContextLoadFonts: Invalid font file name ', name);
			return 0;
		}
		var fontType = c[c.length - 1];
		c = b.split('.' + fontType);
		var fontName = c[0];
//		console.log('jsContextLoadFonts:', name, fontName, fontType);

		var blob = new Blob([body], { type: ("font/" + fontType) });
		var url = URL.createObjectURL(blob);
		var rule = '@font-face { font-family: "' + fontName + '";	src: url("' + url + '") format("truetype"); }';

		// declare font
		var fontface = document.createElement('style');
		fontface.appendChild(document.createTextNode(rule))
		document.head.appendChild(fontface);

		function onFontLoaded() {
			//console.log('font ' + fontName + ' loaded');
			Module.loadingFonts--;
		};

		function onFontFailed() {
			console.log('Failed to load font:', fontName);
			Module.loadingFonts--;
		}

		// load font
		if (document.fonts && typeof(document.fonts.load) == 'function') {
			if (!Module.loadingFonts) {
				Module.loadingFonts = 0;
			}
			Module.loadingFonts++;
			document.fonts.load('72px "' + fontName + '"').then(onFontLoaded, onFontFailed);
			console.log('loading ', name);
			return 1;
		}
		console.log('jsContextLoadFonts: no FontFaceSet.load() ');
		return 0;
	},

	jsContextInit: function (appWidth, appHeight, orientation) {
		// global app JS settings
		Module.appObjects = [];
		Module.appElementCounter = 1;	// element counter
		Module.appInitWidth = appWidth;
		Module.appInitHeight = appHeight;
		Module.appOrientation = orientation;
		Module.appContentWidth = 0;
		Module.appContentHeight = 0;
		Module.documentsDirLoaded = 0;
		Module.appTextMeters = {};

		// JS string to C string
		Module.jstr2cstr = function (jstr) {
			var len = lengthBytesUTF8(jstr) + 1;
			var cstr = _malloc(len + 1);
			stringToUTF8(jstr, cstr, len + 1);
			return cstr;
		}

		// JS array to C array
		Module.jarray2carray = function (jarr) {
			var carr = _malloc(jarr.byteLength);
			HEAPU8.set(jarr, carr);
			return carr;
		}

		var parent = document.getElementById('canvas').parentNode;
		parent.id = 'emscripten_border';

		// override Emscripten's function
		Module.requestFullscreen = function ()
		{
			var element = document.getElementById('canvas');
			element = element.parentNode;

			// Supports most browsers and their versions.
			var requestMethod = element.requestFullScreen || element.webkitRequestFullScreen || element.mozRequestFullScreen || element.msRequestFullScreen;
			if (requestMethod) {
				// Native full screen.
				requestMethod.call(element);
			}
			else if (typeof window.ActiveXObject !== "undefined") {
				// Older IE.
				var wscript = new ActiveXObject("WScript.Shell");
				if (wscript !== null) {
					wscript.SendKeys("{F11}");
				}
			}
		}

		// Safari uses pre-calculated pixels, so use this feature to detect Safari
		var canva = document.createElement('canvas');
		var ctx = canva.getContext("2d");
		var img = ctx.getImageData(0, 0, 1, 1);
		var pix = img.data;		// byte array, rgba
		Module.isSafari = (pix[3] != 0);	// alpha in Safari is not zero

		window.refreshNativeObject = function (id) {
			var obj = document.getElementById(id);
			if (obj) {
				var viewPort = canvas.getBoundingClientRect();
				var scrollLeft = window.pageXOffset || document.documentElement.scrollLeft;
				var scrollTop = window.pageYOffset || document.documentElement.scrollTop;

				var scale = viewPort.width / Module.appInitWidth;
				scale *= (Module.appContentWidth > 0) ? Module.appInitWidth / Module.appContentWidth : 0.5;

				var left = scrollLeft + viewPort.left + Math.ceil((obj.x + obj.w / 2) * scale);
				var top = scrollTop + viewPort.top + Math.ceil((obj.y + obj.h / 2) * scale);
				obj.style.left = left + 'px';
				obj.style.top = top + 'px';
				obj.style.transform = "translate(-50%, -50%) " + "scale(" + scale + ")";
			}
		};

		window.addEventListener("click", function (e) {
			var obj = document.getElementById('showAlert');
			if (obj) {
				if (e.target != obj) {
					obj.close();
				}
			}
		});

	},

	jsContextConfig: function(w, h)	{
		Module.appContentWidth = w;
		Module.appContentHeight = h;
	},

	jsContextGetWindowWidth: function() {	return window.innerWidth;	},
	jsContextGetWindowHeight: function() { return window.innerHeight;	},

	jsContextUnlockAudio: function () {
		// create empty buffer and play it
		if (audioCtx.state != 'running') {
			var buffer = audioCtx.createBuffer(1, 1, 22050);
			var source = audioCtx.createBufferSource();
			source.buffer = buffer;
			source.connect(audioCtx.destination);
			source.start(0);
		}
	},

	jsContextSyncFS: function() {
		Module.idbfsSynced = 0;
		try {
			FS.syncfs(function (err) {
				if (err != null) {
					Module.printErr('Error: Failed to sync IDBFS\n', err);
				}
				Module.idbfsSynced = 1;
			});
		}
		catch (e) {
			Module.printErr('Error: Failed to sync IDBFS\n', e);
		}
		//console.log("Syncing started");
	},

	jsContextResizeNativeObjects: function () {
//		var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
//		if (fullscreenElement == null) {
			var parent = canvas.parentNode.childNodes;
			for (var item in parent) {
				var obj = parent[item];
				if (parseInt(obj.id, 10) > 0)		// native object ?
				{
					window.refreshNativeObject(obj.id);
				}
			}
//		}
	},

	jsContextMountFS: function () {
		// first, check if IDBFS supported
		if (window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB) {
			try {
				FS.mkdir('/documentsDir');
				FS.mount(IDBFS, {}, '/documentsDir');
				FS.syncfs(true, function (err) {
					if (err != null) {
						Module.printErr('Error: Failed to mount IDBFS\n', err);
					}
					Module.documentsDirLoaded = 1;
				});

				// wait for mounting
				return 1;
			}
			catch (e) {
				Module.printErr('Error: Failed to mount IDBFS\n', e);
			}
		}
		console.log('Warning: IndexedDB not supported');
		return 0;
	},

	jsContextGetIntModuleItem: function (item) {
		var name = UTF8ToString(item);
		return Module[name] ? Module[name] : 0;
	},

	//
	// Network
	//

	jsNetworkRequest: function (_url, _method, _headers, _body, body_size, progress, _requestPtr) {
	//  progress:	UNKNOWN		= 0, 	Upload		= 1, 	Download	= 2, 	None		= 3
		var url = UTF8ToString(_url);
		var method = UTF8ToString(_method);
		var headers = UTF8ToString(_headers);

		// get body
		var body = new Uint8Array(body_size);
		for (var i = 0; i < body_size; i++) {
			body[i] = getValue(_body + i, 'i8');
		}

		//console.log('network.request==> url:', url, ' method:', method, ' headers:', headers, ' _body:', _body, ' body_size:', body_size, ' hash:', key, body);

		var xml = new XMLHttpRequest();
		xml.url = url;		// save for callback
		xml.responseType = "arraybuffer";


		// http event handler
		xml.onreadystatechange = function () {

			switch (this.readyState) {
				case XMLHttpRequest.DONE:			// val=4

					var body = new Uint8Array();
					if (this.response) {
						body = new Uint8Array(this.response);
					}

					var headers = this.getAllResponseHeaders();
					if (!headers) {
						headers = "";
					}

					var cheaders = Module.jstr2cstr(headers);
					var cbody = Module.jarray2carray(body);
					_jsNetworkDispatch(_requestPtr, this.readyState, this.status, body.byteLength, cbody, cheaders);
					_free(cbody);
					_free(cheaders);
					break;

				default:
					break;
			}
		};

		xml.open(method, url, true);		// async request

		// key + ": " + value + "\r\n";
		var hlines = headers.split("|");
		for (var i = 0; i < hlines.length; i++) {
			var pair = hlines[i].split(': ');
			if (pair.length == 2) {
				var hkey = pair[0].trim();
				var hval = pair[1].trim();
				if (hkey.length > 0 && hval.length > 0) {
					xml.setRequestHeader(hkey, hval);
					//console.log('setRequestHeader:', hkey, '=', hval);
				}
			}
		}

		xml.send(body);
		return xml;
	},

	// Measures text by creating a DIV in the document and adding the relevant text to it.
	$measureText: function (text, bold, font, size) {
		// This global variable is used to cache repeated calls with the same arguments
		var str = text + ':' + bold + ':' + font + ':' + size;
		if (Module.appTextMeters.hasOwnProperty(str)) {
			return Module.appTextMeters[str];
		}

		var divMain = document.createElement('span');
		var div = document.createElement('span');
		div.innerHTML = text;
		div.style.display = "inline-block";
		div.style.position = 'absolute';
		div.style.top = '-100px';
		div.style.left = '-100px';
		div.style.fontFamily = font;
		div.style.margin = "0px 0px 0px 0px";
		div.style.padding = "0px 0px 0px 0px";
		//	div.style.fontWeight = bold ? 'bold' : 'normal';
		div.style.fontSize = size + 'px';

		divMain.appendChild(div);
		document.body.appendChild(divMain);

		var size = [div.offsetWidth, div.offsetHeight];
		document.body.removeChild(divMain);

		// Add the sizes to the cache as adding DOM elements is costly and can cause slow downs
		Module.appTextMeters[str] = size;
		return size;
	},

	//
	// text render
	//
	jsRenderText: function (thiz, _text, w, h, _alignment, _fontName, fontSize) {
		var text = UTF8ToString(_text);
		var alignment = UTF8ToString(_alignment);

		var fontName = UTF8ToString(_fontName);
		var a = fontName.split('/');
		var fontName = a[a.length - 1];		// filename

		var a = fontName.split('.');
		fontName = a[0];
		var ext = a[1];

		var canva = document.createElement('canvas');
		canva.width = canvas.width;
		canva.height = canvas.height;
		canva.style.position = "absolute";
		var ctx = canva.getContext("2d");

		if (Module.isSafari) {
			ctx.fillStyle = 'red';
		}

		// check if font exists
		// the text whose final pixel size I want to measure
		var testtext = "ABCM|abcdefghijklmnopqrstuvwxyz0123456789";

		// specifying the baseline font
		ctx.font = "72px monospace";

		// checking the size of the baseline text
		var baselineSize = ctx.measureText(testtext).width;

		// specifying the font whose existence we want to check
		ctx.font = "72px '" + fontName + "', monospace";

		// checking the size of the font we want to check
		var newSize = ctx.measureText(testtext).width;

		// If the size of the two text instances is the same, the font does not exist because it is being rendered
		fontExist = newSize != baselineSize;

		if (fontName === '' || fontExist == false) {
		//	console.log(fontName + ' not found, using sans-serif');
			fontName = 'sans-serif';		// Default value
		}
		ctx.font = String(fontSize) + 'px ' + fontName;

		ctx.textBaseline = 'top';
		ctx.textAlign = alignment;

		var a = measureText(testtext, false, fontName, fontSize);
		var lineHeight = a[1];

		if (w == 0) {
			// calc width
			var line = '';
			for (var i = 0; i < text.length; i++) {
				if (text.charAt(i) == '\n') {
					line = '';
				}
				else {
					line += text.charAt(i);
					var metrics = ctx.measureText(line);
					if (metrics.width > w) {
						w = metrics.width;
					}
				}
			}
			// last line
			var metrics = ctx.measureText(line);
			w = Math.max(w, metrics.width);
		}

		var x = 0;
		var y = 0;
		if (alignment === 'right') {
			x = w;
		}
		else
			if (alignment === 'center') {
				x = w / 2;
			}

		// wrap text

		var ww = 0;
		var hh = 0;
		var line = '';
		for (var i = 0; i < text.length; i++) {
			if (text.charAt(i) == '\n') {
				ctx.fillText(line, x, y);
				line = '';
				y += lineHeight;
			}
			else {
				var testLine = line + text.charAt(i);
				var metrics = ctx.measureText(testLine);
				if (metrics.width > w) {
					if (text.charAt(i) === ' ') {
						// ignore last space
						ctx.fillText(line, x, y);
						line = '';
					}
					else {
						// delete last uncomplete word if space exists
						var a = line.split(' ');
						if (a.length > 1)	{
							var line = a[a.length - 1] + text.charAt(i);	// the beginning of next line
							a.pop();		// remove last
							var s = a.join(' ');
							ctx.fillText(s, x, y);
						}
						else{
							// no words, draw line as is 
							ctx.fillText(line, x, y);
							line = text.charAt(i);	// the beginning of next line
						}
					}
					y += lineHeight;
				}
				else {
					line = testLine;
				}
			}
		}

		// last line
		ctx.fillText(line, x, y);

		hh = h > 0 ? h : y + lineHeight;
		ww = w > 0 ? w : 1;

		// it's needs for corona ?
		if ((ww & 0x3) != 0) {
			ww = (ww + 3) & -4;
		}

		//console.log('render: ', metrics, text, w, h, ww, hh, alignment, fontName, fontSize);

		var myImageData = ctx.getImageData(0, 0, ww, hh);
		var img = Module.jarray2carray(myImageData.data);
		_jsEmscriptenBitmapSaveImage(thiz, myImageData.data.length, img, myImageData.width, myImageData.height, Module.isSafari);
		_free(img);

		//var body = document.getElementsByTagName("body")[0];
		//body.appendChild(canva);
		//canva.remove();
	},

	jsContextSetClearColor: function(r, g, b, a)
	{
		var rgba = 'rgba(' + r + ', ' + g + ', ' + b + ', ' + a + ')';
		var canvas = document.getElementById('canvas');
		canvas.parentNode.style.backgroundColor = rgba;

		// restore
		document.body.style.cursor = 'default';
	},

}

autoAddDeps(platformLibrary, '$jsLocaleLanguage');
autoAddDeps(platformLibrary, '$jsLocaleCountry');
autoAddDeps(platformLibrary, '$jsLanguage');
autoAddDeps(platformLibrary, '$measureText');
mergeInto(LibraryManager.library, platformLibrary);
