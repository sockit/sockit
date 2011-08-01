/**
 * Helper function to scroll the chat box down.
 */
function scrollDown() {
	$("#chat").scrollTop(10000000);
}

/**
 * Invoked whenever this chat window receives a message
 *
 *  @param  data    The text to output on the chat window.
 */
function receiveMessage(event) {
    var data = event.read();
	$("#temp").html(data);
	$("#temp").hide();
	data = $("#temp").text();
	$("#chat").html($("#chat").html() + "<div id='you'>" + data + "</div>");
	scrollDown();
}

/**
 * Start the chat application up, and connect to the other chat window, which,
 * should be listening on port 20202.
 *
 *  @param  otherIP The IP address of the remote chat window
 */
function startChat(otherIP) {
	window.client = sockit.createTcpClient(otherIP, 20202);
}

/**
 * Helper function to log some data to the browser console
 *
 *  @param  event   The data to log
 */
function log(event) {
	console.log(event)
}

/**
 * Helper function to send a message to the other chat application
 *
 *  @param  data    The message to send
 */
function sendMessage(data) {
	$("#temp").html(data);
	$("#temp").hide();
	data = $("#temp").text();
	window.client.send(window.user + ": " + data);
	$("#chat").html($("#chat").html() + "<div id='me'>Me: " + data + "</div>");
	scrollDown();
}
