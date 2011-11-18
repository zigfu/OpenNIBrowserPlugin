
OrientationX = 0;
OrientationY = 1;
OrientationZ = 2;

function clamp(x, min, max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

var Hand = Class.create({
	initialize: function(position)
	{
		this.handPos = position;
	}
});

var SessionManager = Class.create({
 
	initialize: function() {
		//alert("new session manager");
		this.controls = new Array();
		this.hands = new Hash();
		this.isInSession = false;
		this.sessionHandID = -1;
	},

	rawHandUpdate: function (handID, x,y,z, t) {
		var position = [x,y,z];
		if (handID == this.sessionHandID)
		{
			this.controls.each(function(hpc){hpc.onHandUpdate(position);})
		}

	},
	rawHandCreate: function (handID, x,y,z, t) {
		//alert("hand create");
		var position = [x,y,z];
	//	alert("new hand create");
	//TODO: add rotation based on user/position of hand relative to sensor
		this.hands[handID] = new Hand(position);
		if (!this.isInSession)
		{
			this.isInSession = true;
			this.sessionHandID = handID;
			this.focusPoint = position;
		}
		this.controls.each(function(hpc){hpc.onHandCreate(position);});

	},
	rawHandDestroy: function (handID, t) {
		
		if (handID == this.sessionHandID)
		{
			this.controls.each(function(hpc){hpc.onHandDestroy();})
			this.isInSession = false;
			this.sessionHandID = -1;
		}
	},
	
	addListener: function(hpc){
		this.controls.push(hpc);
		if (this.isInSession)
		{
			hpc.onHandCreate(this.handPos);
		}
	},
 
	removeListener: function(hpc){
		this.controls = this.controls.without(hpc);
	},
   
	
});
	
	
	
var HandPointControls = Class.create(
{
	initialize: function(SessionManager)
	{
		SessionManager.addListener(this);
		this.sessionManager = SessionManager;
	},
	onHandCreate: function(pos) {},
	onHandUpdate: function(pos) {},
	onHandDestroy: function() {}
});
var Fader = Class.create(HandPointControls, {
	initialize: function($super, SessionManager,size,orientation)
	{
		$super(SessionManager);
		this.size = size;
		this.orientation = orientation;
		this.value = 0;
		this.center = [0,0,0];
	},
	onHandCreate: function(position)
	{
		this.center = position;
		this.onHandUpdate(position);
	},
	onHandUpdate: function(position)
	{
		distanceFromCenter = position[this.orientation] - this.center[this.orientation];
		ret = distanceFromCenter / this.size + 0.5;
		this.value = 1 - clamp(ret, 0, 1);
	},
	
});

var ItemSelector = Class.create(HandPointControls,
{
	initialize: function ($super, SessionManager, size, orientation)
	{
		this.fader = new Fader(SessionManager, size, orientation)
		$super(SessionManager);
		this.itemsCount = 1;
		this.hysteresis = 0.3;
		this.selectedItem = 0;
	},
	onItemSelected: function(item) {},
	onItemUnselected: function(item) {},
	onHandCreate: function(pos) {
	this.selectedItem = Math.floor(this.itemsCount / 2);
	},
	
	onHandUpdate: function(pos) {
		newSelected = this.selectedItem;
		minValue = (this.selectedItem * (1 / this.itemsCount)) - this.hysteresis;
		maxValue = (this.selectedItem + 1) * (1 / this.itemsCount) + this.hysteresis;
		if (this.fader.value > maxValue) {
			newSelected++;
		}
		if (this.fader.value < minValue) {
			newSelected--;
		}
	
		if (newSelected != this.selectedItem) {
			this.onItemUnselected(this.selectedItem);
			this.selectedItem = newSelected;
			this.onItemSelected(newSelected);
		}
	},
	
	onHandDestroy: function() {
		
		this.onItemUnselected(this.selectedItem)
	}
});

/*
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
*/