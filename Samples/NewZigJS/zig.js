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

OrientationX = 0;
OrientationY = 1;
OrientationZ = 2;

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
	this.listeners = [];
	this.focusedControl = undefined;
	this.isInHandpointSession = false;
	this.focusPoint = [0,0,0];
	
	// public
	
	this.AddControl = function(control)
	{
		if (undefined === control) return;
		
		this.listeners.push(control);
		if (this.isInHandpointSession) {
			control.onSessionStart(this.focusPoint);
		}
	}
	
	this.RemoveControl = function(control)
	{
		if (undefined === control) return;
		
		var removed = this.listeners.splice(this.listeners.indexOf(control), 1);
		if (this.isInHandpointSession) {
			removed[0].onSessionEnd();
		}
	}
	
	// this allows nesting control lists
	this.onDoUpdate = function(trackedUser) {
		this.listeners.forEach(function(control) { control.onDoUpdate(trackedUser); });
	}	
	
	this.onSessionStart = function(focuspoint) {
		this.isInHandpointSession = true;
		this.focusPoint = focuspoint;
		this.listeners.forEach(function(control) { control.onSessionStart(focuspoint) });
	}
	
	this.onSessionUpdate = function(hands) {
		this.listeners.forEach(function(control) { control.onSessionUpdate(hands) });
	}
	
	this.onSessionEnd = function() {
		this.listeners.forEach(function(control) { control.onSessionEnd() });	
		this.isInHandpointSession = false;
	}
	
	this.SetFocusedControl = function(control) {
		if (control == this.focusedControl) return;
		
		this.RemoveControl(this.focusedControl);
		this.focusedControl = control;
		this.AddControl(control);
	}
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
	this.position = [];
	this.centerofmass = [];
	this.isInHandpointSession = false;
	
	this.UpdateFullbody = function(centerofmass, skeleton)
	{
		this.centerofmass = centerofmass;
		this.position = centerofmass;
		this.skeleton = skeleton;
	}
	
	this.UpdateHands = function(hands)
	{
		this.hands = hands;
	}
	
	this.NotifyListeners = function()
	{
		// if we aren't in session, but should be
		if (!this.isInHandpointSession && this.hands.length > 0) {
			this.isInHandpointSession = true;
			var focusPoint = this.hands[0].position;
			this.controls.onSessionStart(focusPoint);
		}
		
		// if we are in session, but shouldn't be
		if (this.isInHandpointSession && this.hands.length == 0) {
			this.controls.onSessionEnd();
			this.isInHandpointSession = false;
		}
	
		// at this point we know if we are in a session or not,
		// and we sent the start/end notifications. all thats
		// left is updating the controls if we're in session
		if (this.isInHandpointSession) {
			this.controls.onSessionUpdate(this.hands);
		}
		
		this.controls.onDoUpdate(this);
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
		this.listeners.forEach(function(listener) { listener.onNewUser(trackedUser) });
	}
	
	this.onLostUser = function(trackedUser)
	{
		this.listeners.forEach(function(listener) { listener.onLostUser(trackedUser) });
	}
	
	this.onUpdate = function(userTracker)
	{
		this.listeners.forEach(function(listener) { listener.onUpdate(userTracker) });
	}
	
	this.init = function(plugin) 
	{
		var usertracker = this;
		//ZigAddHandler(plugin, "HandListUpdated", function () { usertracker.UpdateHands(plugin.hands); });
		//ZigAddHandler(plugin, "UserListUpdated", function () { usertracker.UpdateUsers(plugin.users); });
		ZigAddHandler(plugin, "NewFrame", function () { usertracker.DoUpdate(plugin.users, plugin.hands); });
		ZigAddHandler(plugin, "HandListUpdated", function () { usertracker.DoUpdate(plugin.users, plugin.hands); });
		this.log("Zig: inited");
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
		var userid = this.trackedHands[handid];
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
			var curruser = this.getItemById(users, userid);
			if (undefined === curruser && this.isRealUser(userid)) {
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
	}

	this.UpdateHands = function(hands)
	{
		// get rid of old hands
		for (handid in this.trackedHands) {
			var currhand = this.getItemById(hands, handid);
			if (undefined === currhand) {
				this.ProcessLostHand(handid);
			}
		}
		
		// add new hands
		for (handindex in hands) {
			var hand = hands[handindex];
			if (undefined === this.trackedHands[hand.id]) {
				this.ProcessNewHand(hand.id, hand.userid);
			}
		}

		// save raw data before updating the handpoint controls
		this.rawHands = hands;
		
		// update hand points
		// go through list of users
		for (userid in this.trackedUsers) {
			// find hands belonging to this user
			var currhands = [];
			for (handid in this.trackedHands) {
				if (this.trackedHands[handid] == userid) {
					currhands.push(this.getItemById(hands, handid));
				}
			}
			this.trackedUsers[userid].UpdateHands(currhands);
		}
	}
	
	this.DoUpdate = function(users, hands)
	{
		this.UpdateUsers(users);
		this.UpdateHands(hands);
		this.trackedUsers.forEach(function(trackedUser) { trackedUser.NotifyListeners(); });
		var userTracker = this;
		this.listeners.forEach(function(listener) { listener.onUpdate(userTracker); });
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
		var ret = this.lastFakeUserId;
		this.lastFakeUserId++;
		return ret;
	}
	
	this.containsId = function(collection, id)
	{
		return (this.getItemById(collection, id) !== undefined);
	}
	
	// TODO: change from "userid" to "id" so it will work for both hands
	// and users
	this.getItemById = function(collection, id)
	{
		for (item in collection) {
			if (collection[item].id == id) return collection[item];
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
		// DO SKELETON RELATED STUFF HERE
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
function Fader(orientation, size)
{
	// defaults
	if (undefined === size) {
		size = 250;
	}

	// vars
	this.size = size;
	this.orientation = orientation;
	this.value = 0;
	this.center = [0,0,0];
	this.itemsCount = 1;
	this.hysteresis = 0.1;
	this.selectedItem = 0;
	this.initialValue = 0.5;

	// events
	// TODO: make real events
	this.onItemSelected = function(item){};
	this.onItemUnselected = function(item){};

	// hand point control callbacks
	this.onSessionStart = function(sessionStartPosition) {
		this.center = sessionStartPosition;
		this.value = this.initialValue;
		this.selectedItem = Math.floor(this.itemsCount * this.value);
		this.onItemSelected(this.selectedItem);
	}

	this.onSessionUpdate = function(hands) {
		var position = hands[0].position;
		var distanceFromCenter = position[this.orientation] - this.center[this.orientation];
		var ret = (distanceFromCenter / this.size) + 0.5;
		this.value = 1 - this.clamp(ret, 0, 1);

		var newSelected = this.selectedItem;
		var minValue = (this.selectedItem * (1 / this.itemsCount)) - this.hysteresis;
		var maxValue = (this.selectedItem + 1) * (1 / this.itemsCount) + this.hysteresis;
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

	this.onSessionEnd = function() {
		this.onItemUnselected(this.selectedItem);
	}
	
	this.onDoUpdate = function() {};
	
	// internal functions
	
	this.clamp = function(x, min, max)
	{
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}
}

function PushDetector(size)
{
	this.isPushed = false;
	this.pushProgress = 0;
	this.pushTime = 0;

	// events
	this.onPush = function() {}
	this.onRelease = function() {}
	this.onClick = function() {}
	
	if (undefined === size) {
		size = 150;
	}
		
	this.fader = new Fader(OrientationZ, size);
	this.fader.initialValue = 0.8; // TODO: initial value is reversed (1.0 - value) for some reason, i'll fix it later
	
	// callbacks
	this.onSessionStart = function(focusPoint) {
		this.fader.onSessionStart(focusPoint);
	}
	
	this.onSessionUpdate = function(hands) {
		var position = hands[0].position;
		
		// TODO: Move fader to contain current hand point
		this.fader.onSessionUpdate(hands);
		this.pushProgress = this.fader.value;
		
		if (!this.isPushed) {
			if (1.0 == this.pushProgress) {
				this.isPushed = true;
				this.pushTime = (new Date()).getTime();
				this.pushPosition = position;
				this.onPush();
			}
		} else {
			if (this.pushProgress < 0.5) {
				this.isPushed = false;
				if (this.isClick()) {
					this.onClick();
				}
				this.onRelease();
			}
		}
		
		// TODO: drift if we aren't pushed
	}
	
	this.onSessionEnd = function() {
		this.fader.onSessionEnd();
		if (this.isPushed) {
			this.isPushed = false;
			this.onRelease();
		}
	}
	
	this.onDoUpdate = function() {}
	
	// internal
	
	this.isClick = function() 
	{
		var delta = (new Date()).getTime() - this.pushTime;
		return (delta < 1000);
	} 
}

function SwipeDetector(orientation, size) 
{
	// vars
	this.isSwiped = false;

	// events
	this.onSwipeMin = function() {};
	this.onSwipeMax = function() {};
		
	this.fader = new Fader(orientation, size);
	this.fader.initialValue = 0.5;
	
	// callbacks
	this.onSessionStart = function(focusPoint) {
		this.fader.onSessionStart(focusPoint);
	}
	
	this.onSessionUpdate = function(hands) {
		var position = hands[0].position;
		
		// TODO: Move fader to contain current hand point
		this.fader.onSessionUpdate(hands);
		
		if (!this.isSwiped) {
			if (1 == this.fader.value || 0 == this.fader.value) {
				this.isSwiped = true;
				this.swipeValue = this.fader.value;
				if (1 == this.fader.value) {
					this.onSwipeMax();
				} else {
					this.onSwipeMin();
				}
			}
		} else {
			if (Math.abs(this.fader.value - this.swipeValue) >= 0.3) {
				this.onSwipeRelease();
				this.isSwiped = false;
			}
		}
		
		// TODO: drift if we aren't pushed
	}
	
	this.onSessionEnd = function() {
		this.fader.onSessionEnd();
		if (this.isSwiped) {
			this.isSwiped = false;
			this.onSwipeRelease();
		}
	}
	
	this.onDoUpdate = function() {}
}

function HorizontalSwipeDetector(size)
{
	if (undefined === size) {
		size = 350;
	}
	
	this.onSwipeLeft = function() {}
	this.onSwipeRight = function() {}
	this.onSwipeRelease = function() {}
	
	var me = this;
	this.sd = new SwipeDetector(OrientationX, size);
	this.sd.onSwipeMin = function() { me.onSwipeLeft(); }
	this.sd.onSwipeMax = function() { me.onSwipeRight(); }
	this.sd.onSwipeRelease = function() { me.onSwipeRelease(); }
	
	this.onSessionStart = function(focusPoint) { this.sd.onSessionStart(focusPoint); }
	this.onSessionEnd = function() { this.sd.onSessionEnd(); }
	this.onSessionUpdate = function(hands) { this.sd.onSessionUpdate(hands); }
	this.onDoUpdate = function(user) { this.sd.onDoUpdate(user); }
}

function VerticalSwipeDetector(size)
{
	if (undefined === size) {
		size = 250;
	}
	
	this.onSwipeUp = function() {}
	this.onSwipeDown = function() {}
	this.onSwipeRelease = function() {}

	var me = this;
	this.sd = new SwipeDetector(OrientationY, size);
	this.sd.onSwipeMin = function() { me.onSwipeUp(); }
	this.sd.onSwipeMax = function() { me.onSwipeDown(); }
	this.sd.onSwipeRelease = function() { me.onSwipeRelease(); }
	
	this.onSessionStart = function(focusPoint) { this.sd.onSessionStart(focusPoint); }
	this.onSessionEnd = function() { this.sd.onSessionEnd(); }
	this.onSessionUpdate = function(hands) { this.sd.onSessionUpdate(hands); }
	this.onDoUpdate = function(user) { this.sd.onDoUpdate(user); }
}

function ZigTopDownUsersRadar(parentElement, userClass)
{
	this.parentElement = parentElement;
	this.users = [];
	
	this.onNewUser = function(user) {
		var el = document.createElement('div');
		el.classList.add(userClass);
		this.users[user.userid] = el;
		this.parentElement.appendChild(el);
	}
	
	this.onLostUser = function(user) {
		this.parentElement.removeChild(this.users[user.userid]);
		delete this.users[user];
	}
	
	this.DoUserEngaged = function(user) {
		this.users[user.userid].classList.add('active');
	}
	
	this.DoUserDisengaged = function(user) {
		this.users[user.userid].classList.remove('active');
	}
	
	this.onUpdate = function(userTracker) 
	{
		for (var userIndex in userTracker.trackedUsers) {
			var currUser = userTracker.trackedUsers[userIndex];
			var el = this.users[currUser.userid];
			var pos = currUser.position;
			
			el.style.left = ((1 - ((pos[0] / 4000) + 0.5)) * this.parentElement.offsetWidth - (el.offsetWidth / 2)) + "px";
			el.style.top = ((pos[2] / 4000.0) * this.parentElement.offsetHeight - (el.offsetHeight / 2)) + "px";
		}		
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
	
	this.onUpdate = function() {}
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
		var minDistance = -1;
		var ret = 0;
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
		var allusersengaged = this.allUsersEngaged();
		var trackedusers = this.usertracker.trackedUsers;
		if (!this.allUsersEngaged()) {
			// check distance of each user from the "ideal" positions
			var closestLeft = this.getUserClosestTo(trackedusers, this.leftUserIdealPosition);
			var closestRight = this.getUserClosestTo(trackedusers, this.rightUserIdealPosition);
			
			// if the perfect user for both positions is the same
			if (closestLeft == closestRight) {
				// keep looking for another one
				var dLeft = $V(trackedusers[closestLeft].centerofmass).distanceFrom($V(this.leftUserIdealPosition));
				var dRight = $V(trackedusers[closestRight].centerofmass).distanceFrom($V(this.rightUserIdealPosition));
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
		var allusersengaged = this.allUsersEngaged();
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
	this.controls = new ZigControlList();
	
	// the session manager can be inited with a valid userid
	// (for persisting state between pages, etc.)
	if (undefined === userid) {
		userid = 0;
	} else {
		usertracker.trackedUsers[userid].controls.AddControl(this.Controls);
	}
	
	this.usertracker = usertracker;
	this.userid = userid;

	// public events
	this.onUserEngaged = function(user) {}
	this.onUserDisengaged = function(user) {}
	
	this.onNewUser = function(trackeduser) {
		// create a hand point control to do our "work" for us
		var WaitForSession = function(parent, user) { 
			this.onSessionStart = function(focuspoint) { 
				// no active user
				if (parent.userid == 0) {
					// now we do
					parent.userid = user.userid;
					user.engaged = true;
					user.controls.AddControl(parent.controls);
					parent.onUserEngaged(user);
				}
			}
			this.onSessionEnd = function() {
				// active user was us
				if (parent.userid == user.userid) {
					// not anymore
					parent.userid = 0;
					user.engaged = false;
					user.controls.RemoveControl(parent.controls);
					parent.onUserDisengaged(user);
				}
			}
			this.onSessionUpdate = function() {}
			this.onDoUpdate = function() { }
		}
		trackeduser.controls.AddControl(new WaitForSession(this, trackeduser));
	}
	
	this.onLostUser = function(trackeduser) {
		// lost the engaged user?
		if (trackeduser.userid == this.userid) {
			// bummer
			this.userid = 0;
		}
	}
	
	this.onUpdate = function() {}
	
	this.Reset = function() {
		if (this.userid != 0) {
			this.userTracker.trackedUsers[this.userid].controls.RemoveControl(this.Controls);
			this.userid = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Actual zig object
//-----------------------------------------------------------------------------

Zig = new ZigUserTracker();

// "Session managers"
Zig.SingleUser = new ZigEngageSingleSession(Zig);
Zig.SideBySide = new ZigEngageSideBySide(Zig);
Zig.listeners.push(Zig.SingleUser);
//Zig.listeners.push(Zig.SideBySide);

// controls
Zig.Fader = Fader;
Zig.PushDetector = PushDetector;
Zig.VerticalSwipeDetector = VerticalSwipeDetector;
Zig.HorizontalSwipeDetector = HorizontalSwipeDetector;
Zig.TopDownUsersRadar = ZigTopDownUsersRadar;

// consts
Zig.OrientationX = OrientationX;
Zig.OrientationY = OrientationY;
Zig.OrientationZ = OrientationZ;
Zig.verbose = true;
