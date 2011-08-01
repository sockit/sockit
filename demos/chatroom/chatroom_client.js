
/**
 * Helper function invoked when this chatroom client receives a message, invoked
 * by the 'data' event from the plugin.
 *
 *  @param  data    The data, or in this case, the chat message, we received
 *  @param  replier The object on which to send chat replies
 */
function receiveMessage(event) {

    // You tried to put in some html!
    if(event.read() == "##html##")
    {
        alert("Don't you try that!");
        window.location = "http://www.youtube.com/watch?v=dQw4w9WgXcQ";
        return;
    }
    
    var data = event.read();
	$("#temp").html(data);
	$("#temp").hide();
	data = $("#temp").text();
	$("#chat").html($("#chat").html() + "<div id='you'>" + data + "</div>");
	scrollDown();
}

/**
 * Starts a chat with the chatroom host, and starts the client
 * listening for response messages.
 *
 *  @param  otherIP The IP address of the chatroom host
 */
function startChat(otherIP) {
	window.client = sockit.createTcpClient(otherIP, 20202);
	window.client.addEventListener('data', receiveMessage);
}

/**
 * Helper function to log some data to the browser console
 */
function log(event) {
	console.log(event)
}

/**
 * Helper function to send a chat message
 *
 *  @param  data    The chat message to send
 */
function sendMessage(data) {
    window.client.send(window.user + ": " + data);
    $("#temp").html(data);
	$("#temp").hide();
	data = $("#temp").text();
    $("#chat").html($("#chat").html() + "<div id='me'>Me: " + data + "</div>");
	scrollDown();
}
