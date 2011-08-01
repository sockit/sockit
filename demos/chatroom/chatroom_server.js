
/**
 * Helper function invoked when this chatroom server receives a message from  
 * a client, invoked by the 'ondata' event from the plugin.
 *
 *  @param  data    The data, or in this case, the chat message, we received
 *  @param  replier The object on which to send chat replies
 */
 function receiveMessage(event) {

    var data = event.read();
	$("#temp").html(data);
	$("#temp").hide();
	var clean_data = $("#temp").text();

    var diff = data.length - clean_data.length;
    if(diff < -2 || diff > 2)
    {
        event.send("##html##");
        data = "'" + event.getHost() + ":" + event.getPort() + "' just got rick rolled!"; 
    }
    else
    {
        data = clean_data;
    }

	$("#chat").html($("#chat").html() + "<div id='you'>" + data + "</div>");
	
	scrollDown();
	
	var isNewHost = true;
	var newId = event.getHost() + event.getPort();	
	for(id in window.connectedClients) {
		if(newId == id) {
			isNewHost = false;
		} else {
			window.connectedClients[id].send(data);
		}
	}
	
	if(isNewHost) {
		console.log("Adding new endpoint '" + newId + "' to list!");
		window.connectedClients[newId] = event; 
	}
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

	$("#temp").html(data);
	$("#temp").hide();
	data = $("#temp").text();

	for(id in window.connectedClients) {
		window.connectedClients[id].send(window.user + ": " + data);
	}

	$("#chat").html($("#chat").html() + "<div id='me'>Me: " + data + "</div>");
	scrollDown();
}
