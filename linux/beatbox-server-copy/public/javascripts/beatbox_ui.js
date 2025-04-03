"use strict";
// Client-side interactions with the browser for web interface

// Make connection to server when web page is fully loaded.
var socket = io.connect();
var volume = 80;
var tempo = 120;
var communicationsTimeout = null;
$(document).ready(function() {
	setupServerMessageHandlers(socket);

	// Setup a repeating function (every 1s)
	window.setInterval(function() {sendCommandToServer('read-uptime')}, 1000);
	window.setInterval(function() {sendCommandToServer('vol', '-1')}, 1000);
	window.setInterval(function() {sendCommandToServer('beat', '-1')}, 1000);
	window.setInterval(function() {sendCommandToServer('bpm', '-1')}, 1000);

	// Start off by "polling" the volume, mode, and tempo:
	sendCommandToServer('vol', '-1');
	sendCommandToServer('beat', '-1');
	sendCommandToServer('bpm', '-1');


	// Setup the button clicks:
	$('#modeNone').click(function() {
		sendCommandToServer('beat', "2");
	});
	$('#modeRock1').click(function() {
		sendCommandToServer('beat', "0");
	});
	$('#modeRock2').click(function() {
		sendCommandToServer('beat', "1");
	});

	$('#volumeUp').click(function() {
		volume += 5;
		if (volume > 100) {
			volume = 100;
		}
		sendCommandToServer('vol', volume);
	});
	$('#volumeDown').click(function() {
		volume -= 5;
		if (volume < 0) {
			volume = 0;
		}
		sendCommandToServer('vol', volume);
	});

	$('#tempoUp').click(function() {
		var newtempo = tempo + 5;
		if (newtempo > 300) {
			newtempo = 300;
		}
		sendCommandToServer('bpm', "" + newtempo);
	});

	$('#tempoDown').click(function() {
		var newtempo = tempo - 5;
		if (newtempo < 40) {
			newtempo = 40;
		}
		sendCommandToServer('bpm', "" + newtempo);
	});

	$('#tempoSet').click(function() {
		var newtempo = tempo - 5;
		if (newtempo < 40) {
			newtempo = 40;
		}
		sendCommandToServer('bpm', "" + newtempo);
	});

	$('#hi-hat').click(function() {
		console.log("Playing 1");
		sendCommandToServer('play', 'hi_hat');
	});
	$('#snare').click(function() {
		console.log("Playing 2");
		sendCommandToServer('play', "snare");
	});
	$('#base').click(function() {
		console.log("Playing 0");
		sendCommandToServer('play', "base_drum");
	});

	$('#stop').click(function() {
		console.log("Terminating program");
		sendCommandToServer('quit', "0");
	});
});

var hideErrorTimeout;
function setupServerMessageHandlers(socket) {
	// Hide error display:
	$('#error-box').hide(); 


	socket.on('beat-reply', function(message) {
		console.log("Receive Reply: beat-reply " + message);
		var name = "Unknown!";
		switch(Number(message)) {
			case 0: name = "Rock"; break;
			case 1: name = "Custom"; break;
			case 2: name = "None"; break;
		}
		$('#modeid').text(name);
		clearServerTimeout();
	});

	socket.on('vol-reply', function(message) {
		console.log("Receive Reply: vol-reply " + message);
		volume = Number(message);
		$('#volumeid').val(message);
		clearServerTimeout();
	});

	socket.on('bpm-reply', function(message) {
		console.log("Receive Reply: bpm-reply " + message);
		tempo = Number(message);
		$('#tempoid').val(message);
		clearServerTimeout();
	});

	socket.on('play-reply', function(message) {
		console.log("Receive Reply: play-reply " + message);
		clearServerTimeout();
	});

	socket.on('uptime-reply', function(message) {
		var times = message.split(" ");
		var seconds = Number(times[0]);

		var hours = Math.floor(seconds/60/60);
		var minutes = Math.floor((seconds / 60) % 60);
		seconds = Math.floor(seconds % 60);

		var display = "Device up for: " + hours + ":" + minutes + ":" + seconds + "(H:M:S)";

		$('#status').html(display);
		clearServerTimeout();
	});

	socket.on('beatbox-error', errorHandler);
}

function sendCommandToServer(command, options) {
	if (communicationsTimeout == null) {
		communicationsTimeout = setTimeout(errorHandler, 1000, 
			"ERROR: Unable to communicate to HTTP server. Is nodeJS server running?");
	}
	socket.emit(command, options);
}
function clearServerTimeout() {
	clearTimeout(communicationsTimeout);
	communicationsTimeout = null;
}

function errorHandler(message) {
	console.log("ERROR Handler: " + message);
	// Make linefeeds into <br> tag.
	//	message = replaceAll(message, "\n", "<br/>");

	$('#error-text').html(message);	
	$('#error-box').show();

	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	hideErrorTimeout = window.setTimeout(function() {$('#error-box').hide();}, 5000);
	clearServerTimeout();
}
