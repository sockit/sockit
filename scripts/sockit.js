function loadSockitPlugin() {

    var mimeType = "application/x-sockit";

    if (window['sockit'] === undefined) {

        var pluginAvailable = false;
        for (var i = 0; i < navigator.mimeTypes.length; i++) {
            if (navigator.mimeTypes[i].type == mimeType) {
                pluginAvailable = true;
                break;
            }
        }

        if (!pluginAvailable) {
            var redirect = confirm("SockIt plugin is not currently installed, would you like to be redirected to the SockIt plugin download page?");
            if(redirect) {
                window.location = "http://sockit.github.com/downloads.html";
            }
        }

        var plugin = document.createElement('object');
        plugin.setAttribute('type', mimeType);
        plugin.setAttribute('style', 'width: 0; height: 0;');
        document.documentElement.appendChild(plugin);
        window.sockit = plugin;
    }
    return window.sockit;
}
