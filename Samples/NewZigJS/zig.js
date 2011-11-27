//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
function include(file)
{
	var script  = document.createElement('script');
	script.src  = file;
	script.type = 'text/javascript';
	script.defer = true;
	document.getElementsByTagName('head').item(0).appendChild(script);
}

include("sylvester.js");

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
	this.isInHandpointSession = false;
	this.focusPoint = [0,0,0];
	
	// public
	
	this.AddHandPointControl = function(control)
	{
		this.handPointControls.push(control);
		if (this.isInHandpointSession) {
			control.onSessionStart(this.focusPoint);
		}
	}
	
	this.RemoveHandPointControl = function(control)
	{
		removed = this.handPointControls.splice(this.handPointControls.indexOf(control), 1);
		if (this.isInHandpointSession) {
			removed[0].onSessionEnd();
		}
	}
	
	this.AddFullBodyControl = function(control)
	{
		this.fullBodyControls.push(control);
	}
	
	this.RemoveFullBodyControl = function(control)
	{
		this.fullBodyControls.splice(this.fullBodyControls.indexOf(control), 1);
	}
	
	this.AddControl = function(control)
	{
		this.AddHandPointControl(control);
		this.AddFullBodyControl(control);
	}
	
	this.RemoveControl = function(control)
	{
		this.RemoveHandPointControl(control);
		this.RemoveFullBodyControl(control);
	}
	
	// internal callbacks
	
	this.onMove = function(position) {
		this.fullBodyControls.every(function(control) { control.onMove(position); });
	}
	
	this.onSkeletonUpdate = function(skeleton)
	{
		this.fullBodyControls.every(function(control) { control.onSkeletonUpdate(skeleton) });
	}
	
	this.onSessionStart = function(focusPoint)
	{
		this.isInHandpointSession = true;
		this.focusPoint = focusPoint;
		this.handPointControls.every(function(control) { control.onSessionStart(focusPoint) });
	}
	
	this.onSessionUpdate = function(hands)
	{
		this.handPointControls.every(function(control) { control.onSessionUpdate(hands) });
	}
	
	this.onSessionEnd = function()
	{
		this.handPointControls.every(function(control) { control.onSessionEnd() });
		this.isInHandpointSession = false;
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

function ZigTrackedUser(userid)
{
	this.controls = new ZigControlList();
	this.skeleton = [];
	this.hands = [];
	this.userid = userid;
	this.centerofmass = [];
	this.isInHandpointSession = false;
	
	this.UpdateFullbody = function(centerofmass, skeleton)
	{
		this.centerofmass = centerofmass;
		this.skeleton = skeleton;
		this.controls.onSkeletonUpdate(skeleton);
		this.controls.onMove(centerofmass);
	}
	
	this.UpdateHands = function(hands)
	{
		this.hands = hands;
		
		// if we aren't in session, but should be
		if (!this.isInHandpointSession && hands.length > 0) {
			this.controls.onSessionStart(hands[0].position);
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
	
	this.listeners = [];
	
	// public events
	// TODO: make real events (listeners for now)
	this.onNewUser = function(trackedUser)
	{
		this.listeners.every(function(listener) { listener.onNewUser(trackedUser) });
	}
	
	this.onLostUser = function(trackedUser)
	{
		this.listeners.every(function(listener) { listener.onLostUser(trackedUser) });
	}
	
	this.onUpdate = function()
	{
		this.listeners.every(function(listener) { listener.onUpdate() });
	}
	
	this.init = function(plugin) 
	{
		usertracker = this;
		ZigAddHandler(plugin, "HandListUpdated", function () { usertracker.UpdateHands(plugin.hands); });
		ZigAddHandler(plugin, "UserListUpdated", function () { usertracker.UpdateUsers(plugin.users); });
	}
	
	this.ProcessNewUser = function(userid)
	{
		this.log("Zig: new user " + userid);
	
		// if we aren't tracking this user yet, start now
		// (its possible that we are already tracking the user
		//  from a previous hand point etc.)
		if (!this.isUserTracked(userid)) {
			this.trackedUsers[userid] = new ZigTrackedUser(userid);
			this.onNewUser(this.trackedUsers[userid]);
		}
	}

	this.ProcessLostUser = function(userid)
	{
		this.log("Zig: lost user " + userid);
		
		if (this.isUserTracked(userid)) {
			lost = this.trackedUsers[userid];
			delete this.trackedUsers[userid];
			this.onLostUser(lost);
		}
	}

	this.ProcessNewHand = function(handid, userid)
	{
		this.log("Zig: new hand " + handid);
		
		// no user id
		if (userid <= 0) {
			this.log("Zig: new hand belongs to non-tracked user");
			
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
		this.log("Zig: new hand associated with user " + userid);
		this.trackedHands[handid] = userid;
	}

	this.ProcessLostHand = function(handid)
	{
		this.log("Zig: lost hand");
		
		// remove the hand->user association
		userid = this.trackedHands[handid];
		delete this.trackedHands[handid];
		
		// if this user is "fake" (created for this specific 
		// hand point) then get rid of it
		if (!this.isRealUser(userid)) {
			this.log("Zig: lost hand was with fake user, removing");
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
			if (!this.isUserTracked(users[user].id)) {
				this.ProcessNewUser(users[user].id);
			}
		}

		// save raw data before updating the fullbody controls
		this.rawUsers = users;
		
		// update stuff
		for (user in users) {
			this.trackedUsers[users[user].id].UpdateFullbody(users[user].centerofmass, users[user].joints);
		}
		
		this.listeners.every(function(listener) { listener.onUpdate(); });
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
		for (handindex in hands) {
			hand = hands[handindex];
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
			this.trackedUsers[userid].UpdateHands(currhands);
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
			if (collection[item].id == id) return item;
		}
		return undefined;
	}
	
	this.log = function(s)
	{
		if (this.verbose) {
			console.log(s);
		}
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
	this.onSessionStart = function(sessionStartPosition) {
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
	this.onSessionStart = function(sessionStartPosition) {
		this.center = sessionStartPosition;
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
// "Engagement" managers
//-----------------------------------------------------------------------------

function ZigEngageSingleUser(userid)
{	
	// the session manager can be inited with a valid userid
	// (for persisting state between pages, etc.)
	if (undefined == userid) {
		userid = 0;
	}
	
	this.PrimaryUser = new ZigControlList();
	this.userid = userid;
	
	this.onNewUser = function(trackeduser) {
		// not tracking anyone yet?
		if (0 == this.userid) {
			// start now
			this.userid = trackeduser.userid;
			trackeduser.controls.AddControl(this.PrimaryUser);
		}
	}
	
	this.onLostUser = function(trackeduser) {
		// lost the engaged user?
		if (trackeduser.userid == this.userid) {
			// bummer
			this.userid = 0;
		}
	}
}

// NOTE: This needs to be rewritten; user control that waits till the user is in alone in a "region"
// Init 2 of those per user, one for left and one for right
function ZigEngageSideBySide(usertracker, leftuserid, rightuserid)
{
	this.usertracker = usertracker;
	this.LeftUser = new ZigControlList();
	this.RightUser = new ZigControlList();
	this.leftuserid = (leftuserid == undefined ? 0 : leftuserid);
	this.rightuserid = (rightuserid == undefined ? 0 : rightuserid);
	
	this.onUsersEngaged = function() {}
	this.onUsersMissing = function() {}

	this.leftUserIdealPosition = [-1000,1000,2000];
	this.rightUserIdealPosition = [1000,1000,2000];

	this.getUserClosestTo = function(trackedusers, position) 
	{
		minDistance = -1;
		ret = 0;
		for (userid in trackedusers) {
			currDistance = $V(trackedusers[userid].centerofmass).distanceFrom($V(position));
			if (-1 == minDistance || currDistance < minDistance) {
				minDistance = currDistance;
				ret = userid;
			}
		}
	}
	
	this.onUpdate = function() {
		// if not all users are engaged
		allusersengaged = this.allUsersEngaged();
		trackedusers = this.usertracker.trackedUsers;
		if (!this.allUsersEngaged()) {
			// check distance of each user from the "ideal" positions
			closestLeft = this.getUserClosestTo(trackedusers, this.leftUserIdealPosition);
			closestRight = this.getUserClosestTo(trackedusers, this.rightUserIdealPosition);
			
			// if the perfect user for both positions is the same
			if (closestLeft == closestRight) {
				// keep looking for another one
				dLeft = $V(trackedusers[closestLeft].centerofmass).distanceFrom($V(this.leftUserIdealPosition));
				dRight = $V(trackedusers[closestRight].centerofmass).distanceFrom($V(this.rightUserIdealPosition));
				if (dLeft < dRight) {
					closestRight = 0; 
				} else {
					closestLeft = 0;
				}
			}
		}
		
		// should we fire the UsersEngaged event?
		if (!allusersengaged && this.allUsersEngaged()) {
			this.userTracker
			this.onUsersEngaged();
		}
	}
	
	this.onLostUser = function(trackeduser) {
		// is the lost user one of our engaged users?
		allusersengaged = this.allUsersEngaged();
		if (trackeduser.userid == this.leftuserid) {
			this.leftuserid = 0;
		}
		if (trackeduser.userid == this.rightuserid) {
			this.rightuserid = 0;
		}
		
		// should we fire the UsersMissing event?
		if (allusersengaged && !this.allUsersEngaged()) {
			this.onUsersMissing();
		}
	}
	
	this.allUsersEngaged = function() {
		return (this.leftuserid != 0 && this.rightuserid != 0);
	}
}

function ZigEngageSingleSession(usertracker, userid)
{
	this.Controls = new ZigControlList();

	// the session manager can be inited with a valid userid
	// (for persisting state between pages, etc.)
	if (undefined == userid) {
		userid = 0;
	} else {
		usertracker.trackedUsers[userid].controls.AddControl(this.Controls);
	}
	
	this.usertracker = usertracker;
	this.userid = userid;
	
	this.onNewUser = function(trackeduser) {
		// create a hand point control to do our "work" for us
		WaitForSession = function(parent, user) { 
			this.onSessionStart = function(focuspoint) { 
				// no active user
				if (parent.userid == 0) {
					// now we do
					parent.userid = user.userid;
					user.controls.AddControl(parent.Controls);
				}
			}
			this.onSessionEnd = function() {
				// active user was us
				if (parent.userid == user.userid) {
					// not anymore
					parent.userid = 0;
					user.controls.RemoveControl(parent.Controls);
				}
			}
			this.onSessionUpdate = function() {}
		}
		trackeduser.controls.AddHandPointControl(new WaitForSession(this, trackeduser));
	}
	
	this.onLostUser = function(trackeduser) {
		// lost the engaged user?
		if (trackeduser.userid == this.userid) {
			// bummer
			this.userid = 0;
		}
	}
	
	this.onUpdate = function() {}
}

//-----------------------------------------------------------------------------
// Actual zig object
//-----------------------------------------------------------------------------

Zig = new ZigUserTracker();

// "Session managers"
Zig.SingleUser = new ZigEngageSingleSession(Zig);
Zig.SideBySide = new ZigEngageSideBySide(Zig);
Zig.listeners.push(Zig.SingleUser);
Zig.listeners.push(Zig.SideBySide);

Zig.OrientationX = 0;
Zig.OrientationY = 1;
Zig.OrientationZ = 2;
Zig.verbose = true;
