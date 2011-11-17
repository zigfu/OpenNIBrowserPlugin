
OrientationX = 0;
OrientationY = 1;
OrientationZ = 2;

function clamp(x, min, max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

function Fader(size, orientation)
{
	this.size = size;
	this.orientation = orientation;
	this.value = 0;
	this.center = [0,0,0];
	this.itemsCount = 1;
	this.hysteresis = 0.3;
	this.selectedItem = 0;
	
	this.onItemSelected = function(item){};
	this.onItemUnselected = function(item){};
}

Fader.prototype.handcreate = function(position)
{
	this.center = position;
	this.selectedItem = Math.floor(this.itemsCount / 2);
	this.handupdate(position);
	this.onItemSelected(newSelected);
}

Fader.prototype.handupdate = function(position)
{
	distanceFromCenter = position[this.orientation] - this.center[this.orientation];
	ret = distanceFromCenter / this.size + 0.5;
	this.value = 1 - clamp(ret, 0, 1);

	newSelected = this.selectedItem;
	
	minValue = (this.selectedItem * (1 / this.itemsCount)) - this.hysteresis;
	maxValue = (this.selectedItem + 1) * (1 / this.itemsCount) + this.hysteresis;
	if (this.value > maxValue) {
		newSelected++;
	}
	if (this.value < minValue) {
		newSelected--;
	}
	
	if (newSelected != this.selectedItem) {
		this.onItemUnselected(this.selectedItem);
		this.selectedItem = newSelected;
		this.onItemSelected(newSelected);
	}
}

Fader.prototype.handdestroy = function(position)
{
	this.onItemUnselected(this.selectedItem);
}