//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

function ZigAddHandler(target,eventName,handlerName)
{
	if (target.attachEvent) {
		target.attachEvent("on" + eventName, handlerName);
	} else if ( target.addEventListener ) {
		target.addEventListener(eventName, handlerName, false);
	} else {
		target["on" + eventName] = handlerName;
	}
}

//-----------------------------------------------------------------------------
// ZigControlList
// A list of controls getting updates. Can be nested o'mercy
//-----------------------------------------------------------------------------

function ZigControlList()
{
	this.handPointControls = [];
	this.fullBodyControls = [];
	
	// public
	
	this.AddHandPointControl = function(control)
	{
		this.handPointControls.push(control);
	}
	
	this.AddFullBodyControl = function(control)
	{
		this.fullBodyControls.push(control);
	}
	
	this.AddControl = function(control)
	{
		this.fullBodyControls.push(control);
		this.handPointControls.push(control);
	}
	
	// internal callbacks
	
	this.onSkeletonUpdate = function(skeleton)
	{
		this.fullBodyControls.every(function(control) { control.onSkeletonUpdate(skeleton) });
	}
	
	this.onSessionStart = function(hands)
	{
		this.handPointControls.every(function(control) { control.onSessionStart(hands) });
	}
	
	this.onSessionUpdate = function(hands)
	{
		this.handPointControls.every(function(control) { control.onSessionUpdate(hands) });
	}
	
	this.onSessionEnd = function()
	{
		this.handPointControls.every(function(control) { control.onSessionEnd() });
	}
	
	// TODO: "ActiveControl" stuff
	// SetActiveControl();
	// Back();
	// pass the TrackedUser and ControlList in every event?
}

//-----------------------------------------------------------------------------
// Tracked user
// A tracked user is created for every user we "see".
//-----------------------------------------------------------------------------

function ZigTrackedUser()
{
	this.controls = new ZigControlList();
	this.skeleton = [];
	this.hands = [];
	this.isInHandpointSession = false;
	
	this.UpdateFullbody = function(skeleton)
	{
		this.controls.onSkeletonUpdate(skeleton);
	}
	
	this.UpdateHands = function(hands)
	{
		// if we aren't in session, but should be
		if (!this.isInHandpointSession && hands.length > 0) {
			this.controls.onSessionStart(hands);
			this.isInHandpointSession = true;
		}
		
		// if we are in session, but shouldn't be
		if (this.isInHandpointSession && hands.length == 0) {
			this.controls.onSessionEnd();
			this.isInHandpointSession = false;
		}
	
		// at this point we know if we are in a session or not,
		// and we sent the start/end notifications. all thats
		// left is updating the controls if we're in session
		if (this.isInHandpointSession) {
			this.controls.onSessionUpdate(hands);
		}
	}
}

//-----------------------------------------------------------------------------
// User tracker
// This is the main Zig entry point. The user tracker is fed with raw data
// from the acquisition layer in the form of a list of users, and a list of
// hand points.
// The user tracker maps users and hands from the acquisition layer to 
// TrackedUsers that keep lists of active hand point and full body controls
//-----------------------------------------------------------------------------

function ZigUserTracker()
{
	// raw data from last frame, directly from acquisition layer
	this.rawUsers = [];
	this.rawHands = [];
	// mapping of userid's to TrackedUser's
	this.trackedUsers = [];
	// mapping of handid's to userid's
	this.trackedHands = [];

	// do we allow hand point sessions from hand points
	// with no associated userid?
	this.allowHandsForUntrackedUsers = true;
	
	// public events
	// TODO: make real events
	this.onNewUser = function(trackedUser){};
	this.onLostUser = function(trackedUser){};
	
	this.init = function(plugin) 
	{
		usertracker = this;
		ZigAddHandler(plugin, "UserListUpdated", function () { usertracker.UpdateUsers(plugin.users); });
		ZigAddHandler(plugin, "HandListUpdated", function () { usertracker.UpdateHands(plugin.users); });
	}
	
	this.ProcessNewUser = function(userid)
	{
		// if we aren't tracking this user yet, start now
		// (its possible that we are already tracking the user
		//  from a previous hand point etc.)
		if (!this.isUserTracked(userid)) {
			this.trackedUsers[userid] = new ZigTrackedUser();
			this.onNewUser(this.trackedUsers[userid]);
		}
	}

	this.ProcessLostUser = function(userid)
	{
		if (this.isUserTracked(userid)) {
			lost = this.trackedUsers[userid];
			delete this.trackedUsers[userid];
			this.onLostUser(lost);
		}
	}

	this.ProcessNewHand = function(handid, userid)
	{
		// no user id
		if (userid <= 0) {
			// get out if we dont allow such hands
			if (!this.allowHandsForUntrackedUsers) return;
			
			// otherwise allocate a "fake" user id and use it
			userid = this.getFakeUserId();
		}
	
		// add the user if neccessary
		if (!this.isUserTracked(userid)) {
			this.ProcessNewUser(userid);
		}
		
		// associate this hand with the user
		this.trackedHands[handid] = userid;
	}

	this.ProcessLostHand = function(handid)
	{
		// remove the hand->user association
		userid = this.trackedHands[handid];
		delete this.trackedHands[handid];
		
		// if this user is "fake" (created for this specific 
		// hand point) then get rid of it
		if (!this.isRealUser(userid)) {
			this.ProcessLostUser(userid);
		}
	}
	
	this.UpdateUsers = function(users)
	{
		// get rid of old users
		for (userid in this.trackedUsers) {
			curruser = this.getItemById(users, userid);
			if (undefined == curruser) {
				this.ProcessLostUser(userid);
			}
		}
		
		// add new users
		for (user in users) {
			if (!this.isUserTracked(user.userid)) {
				this.ProcessNewUser(user.userid);
			}
		}

		// save raw data before updating the fullbody controls
		this.rawUsers = users;
		
		// update full body sessions
		for (user in users) {
			this.trackedUsers[user.userid].UpdateFullbody(user.joints);
		}
	}

	this.UpdateHands = function(hands)
	{
		// get rid of old hands
		for (handid in this.trackedHands) {
			currhand = this.getItemById(hands, handid);
			if (undefined == currhand) {
				this.ProcessLostHand(handid);
			}
		}
		
		// add new hands
		for (hand in hands) {
			if (undefined == this.trackedHands[hand.id]) {
				this.ProcessNewHand(hand.id, hand.userid);
			}
		}

		// save raw data before updating the handpoint controls
		this.rawHands = hands;
		
		// update hand points
		// go through list of users
		for (userid in this.trackedUsers) {
			// find hands belonging to this user
			currhands = [];
			for (handid in this.trackedHands) {
				if (this.trackedHands[handid] == userid) {
					currhands.push(this.getItemById(hands, handid));
				}
			}
			this.trackedUsers[user.userid].UpdateHands(currhands);
		}
	}
	
	// does this user actually exist in the acquisition layer?
	this.isRealUser = function(userid)
	{
		return this.containsId(this.rawUsers, userid);
	}
	
	// are we tracking this (real or "fake") user?
	// a fake user is one that doesn't exist in the acquisition layer (from external 
	// hand tracker, for instance)
	this.isUserTracked = function(userid)
	{
		return (typeof(this.trackedUsers[userid]) != 'undefined');
	}
	
	// we want to make sure our "fake" user id range doesn't conflict with
	// user id's from the acquisition layer.
	this.lastFakeUserId = 1337;
	this.getFakeUserId = function()
	{
		ret = this.lastFakeUserId;
		this.lastFakeUserId++;
		return ret;
	}
	
	this.containsId = function(collection, id)
	{
		return (this.getItemById(collection, id) != undefined);
	}
	
	// TODO: change from "userid" to "id" so it will work for both hands
	// and users
	this.getItemById = function(collection, id)
	{
		for (item in collection) {
			if (item.userid == id) return item;
		}
		return undefined;
	}
}

//-----------------------------------------------------------------------------
// Full Body Controls
//-----------------------------------------------------------------------------

// template
function FullBodyControl()
{
	this.onSkeletonUpdate = function(skeleton) 
	{
		// DO SKELETON RELATED SHIT HERE
	}
}

//-----------------------------------------------------------------------------
// Hand Point Controls
//-----------------------------------------------------------------------------

// Template control
function HandPointControl()
{
	this.onSessionStart = function(hands) {
		// DO SESSION START STUFF HERE
	}
	
	this.onSessionUpdate = function(hands) {
		// DO SESSION UPDATE STUFF HERE
	}

	this.onSessionEnd = function() {
		// DO SESSION END STUFF HERE
	}
}

// Fader
function Fader(size, orientation)
{
	// const
	this.OrientationX = 0;
	this.OrientationY = 1;
	this.OrientationZ = 2;

	// var
	this.size = size;
	this.orientation = orientation;
	this.value = 0;
	this.center = [0,0,0];
	this.itemsCount = 1;
	this.hysteresis = 0.3;
	this.selectedItem = 0;

	// events
	// TODO: make real events
	this.onItemSelected = function(item){};
	this.onItemUnselected = function(item){};

	// hand point control callbacks
	this.onSessionStart = function(hands) {
		this.center = hands[0];
		this.selectedItem = Math.floor(this.itemsCount / 2);
		this.onItemSelected(newSelected);
	}

	this.onSessionUpdate = function(hands) {
		position = hands[0];
		distanceFromCenter = position[this.orientation] - this.center[this.orientation];
		ret = distanceFromCenter / this.size + 0.5;
		this.value = 1 - this.clamp(ret, 0, 1);

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

	this.onSessionDestroy = function() {
		this.onItemUnselected(this.selectedItem);
	}
	
	// internal functions
	
	this.clamp = function(x, min, max)
	{
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}
}

//-----------------------------------------------------------------------------
// Actual zig object
//-----------------------------------------------------------------------------

Zig = new ZigUserTracker();