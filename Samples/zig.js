
OrientationX = 0;
OrientationY = 1;
OrientationZ = 2;

function clamp(x, min, max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

var Fader = Class.create({
	initialize : function(size, orientation) {
		this.size = size;
		this.orientation = orientation;	
		this.center = [0,0,0];
	},

	recenter : function(position) {
		this.center = position;
	},

	getvalue : function(position) {
		distanceFromCenter = position[this.orientation] - this.center[this.orientation];
		ret = distanceFromCenter / this.size + 0.5;
		return clamp(ret, 0, 1);
	}
});