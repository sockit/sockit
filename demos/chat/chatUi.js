
/**
 * Helper function invoked when the user enters the IP address
 */
function enter_ip() {
	ipaddr = $("#ip").val();
	if(ipaddr != "") {
		window.ipaddr = ipaddr
		startChat( ipaddr )
		$("#ip").remove()
		$("#ipbutton").remove()
        $("#username").show()
        $("#usernamebutton").show();
		$("#username").focus();
	}
	else {
		alert("ip address cannot be empty");
	}
}

/**
 * Helper invoked on keypress for the IP input box, that enters the IP
 */
function keyup_ip(event) {
	if(event.keyCode == 13){
		enter_ip();
	}
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
	}
	else if(typeof window.ipaddr === 'undefined') {
		alert("please enter the ip address of the other client");
	}
	else if( $("#chatter").val() != ""  ) {
		sendMessage( $("#chatter").val() );
		$("#chatter").val("")
		$("#chatter").focus();
	}
}

/**
 * Helper invoked on keypress for the message box, which sends the message if the key
 * was the enter key
 */
function keyup_chatter() {
	if(event.keyCode == 13){
		enter_message();
	}
}

