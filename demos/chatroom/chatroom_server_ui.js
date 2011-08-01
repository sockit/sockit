/**
 * Helper function to scroll the chat box to the bottom
 */
function scrollDown() {
	$("#chat").scrollTop(10000000);
}

/**
 * Helper to set the username
 */
 function enter_username() {
	window.user = $("#username").val();
	if(window.user == "") {
		window.user = "anonymous"
	}

	$("#username").remove()
	$("#usernamebutton").remove()
    $("#chatter").show()
    $("#chatterbutton").show()
	$("#chatter").focus();
}

/**
 * Handles keypresses for the username box
 */
 function keyup_username(event) {
	if(event.keyCode == 13) {
		enter_username();
	}
}


/**
 * Helper invoked when this chat window tries to send a message
 */
function enter_message() {
	if(typeof window.user === 'undefined') {
		alert("enter a username");
	} else if( $("#chatter").val() != ""  ) {
		sendMessage( $("#chatter").val() );
		$("#chatter").val("")
		$("#chatter").focus();
	}
}

/**
 * Helper invoked on keypress for the message box, which sends the message if the key
 * was the enter key
 */
function keyup_chatter(event)
{
	if(event.keyCode == 13){
		enter_message();
	}
}
