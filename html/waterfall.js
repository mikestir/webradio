function Waterfall(waterfall_target, scale_target) {
	this.canvas = document.getElementById(waterfall_target);
	this.context = this.canvas.getContext("2d");
	if (scale_target != null) {
		this.scale_canvas = document.getElementById(scale_target);
		this.scale_context = this.scale_canvas.getContext("2d");
	} else {
		this.scale_canvas = null;
		this.scale_context = null;
	}

	this.centreFrequency = 0;
	this.sampleRate = 0;
	this.nMarkers = 3; // number of markers either side of centre

	// Create waterfall colour scheme
	this.palette = new Array();
	for (var n = 0; n < 256; n++) {
		var r,g,b;
		if (n < 64) {
			r = 0; g = 0; b = n * 4;
		} else if (n < 128) {
			r = 0; g = (n - 64) * 4; b = 255;
		} else if (n < 192) {
			r = (n - 128) * 4; g = 255; b = 255 - (n - 128) * 4;
		} else {
			r = 255; g = 255 - (n - 192) * 4; b = 0;
		}

//		r = 0; g = n; b = 0; //matrix!
		this.palette.push("rgba(" + r + "," + g + "," + b + ", 0.5)");
	}

	// Pre-fill canvas
	this.context.fillStyle = "black";
	this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
}

Waterfall.prototype.setCentreFrequency = function(f) {
	this.centreFrequency = f;
	this.redrawScale();
}

Waterfall.prototype.setSampleRate = function(r) {
	this.sampleRate = r;
	this.redrawScale();
}

Waterfall.prototype.redrawScale = function() {
	if (this.scale_canvas == null)
		return;

	var ctx = this.scale_context;
	var w = this.scale_canvas.width;
	var h = this.scale_canvas.height;

	// Clear scale canvas
	ctx.fillStyle = "rgba(0,0,0,0)";
	ctx.fillRect(0, 0, w, h - 18);
	ctx.fillStyle = "rgba(0,0,0,0.8)";
	ctx.fillRect(0, h - 18, w, 18);

	// Redraw scale text
	ctx.font = "15px Arial";
	ctx.textAlign = "center";
	ctx.strokeStyle = "white";
	var freq = this.centreFrequency - this.sampleRate / 2;
	for (n = 0; n < this.nMarkers * 2 + 1; n++) {
		var xpos = n * w / this.nMarkers / 2;
		var label = (freq / 1000000.0).toFixed(3).toString();
		ctx.strokeText(label, xpos, h - 2);
		freq = freq + this.sampleRate / this.nMarkers / 2;
	}

	// Redraw markers
	var grad = ctx.createLinearGradient(0, 0, 0, h);
	grad.addColorStop(0, "rgba(0,0,0,0)");
	grad.addColorStop(0.5, "rgba(255,255,255,1)");
	grad.addColorStop(1, "rgba(0,0,0,0)");
	ctx.beginPath();
	for (n = 1; n < this.nMarkers * 2; n++) {
		var xpos = n * w / this.nMarkers / 2;
		
		ctx.moveTo(xpos, 0);
		ctx.lineTo(xpos, h - 15);
	}
	ctx.strokeStyle = grad;
	ctx.stroke();

}

Waterfall.prototype.update = function(series) {
	var ctx = this.context;
	var w = this.canvas.width / series.length;
	
	function colour(self, val) {
		val = val * 255.0;
		val = Math.floor(val);
		if (val < 0) val = 0;
		if (val > 255) val = 255;
		return self.palette[val];
	}
	
	// Draw new series into first row
	for (var bin = 0; bin < series.length; bin++) {
		ctx.fillStyle = colour(this, (series[bin] + 50.0) / 25.0); // x2 here for some reason?
		ctx.fillRect(Math.floor(bin * w), 0, Math.ceil(w), 1);
	}
}

Waterfall.prototype.scroll = function() {
	var ctx = this.context;
	var w = this.canvas.width;
	var h = this.canvas.height;

	// Copy image one row lower
	var image = ctx.getImageData(0,0,w,h - 1);
	ctx.putImageData(image, 0, 1);
}
