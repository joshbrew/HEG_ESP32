//Utils provided by Diego Schmaedech (MIT License), Statechanger Brazil. Modified by Joshua Brewster (MIT License)
class chromeSerial {
    constructor() {
        this.displayPorts = []
        if (typeof chrome.serial !== 'undefined' && chrome.serial !== null) {
            setupSerial();
        }  
        else {
            console.log("ERROR: Cannot locate chrome.serial.")
        }
    }
    onGetDevices(ports) {
        var paths = [];
        for (var i = 0; i < ports.length; i++) {
            console.log(ports[i].path);
        }
        ports.forEach(function (port) {
            var displayName = port["displayName"] + "(" + port.path + ")";
            console.log("displayName " + displayName);
            if (!displayName)
                displayName = port.path;  
            paths.push({'displayName':displayName, 'path':port.path});
        });
        this.displayPorts = paths;
    }

    onReceive(receiveInfo) {
        console.log("onReceive");
        if (receiveInfo.connectionId !== connectionId) {
            return;
        }
        var bufView = new Uint8Array(receiveInfo.data);
        var encodedString = String.fromCharCode.apply(null, bufView);

        encodedBuffer += decodeURIComponent(escape(encodedString));

        var index;
        while ((index = encodedBuffer.indexOf('\n')) >= 0) {
            var line = encodedBuffer.substr(0, index + 1);
            onReadLine(line);
            encodedBuffer = encodedBuffer.substr(index + 1);
        }
    }

    onReceiveError(errorInfo) {
        console.log("onReceiveError");
        if (errorInfo.connectionId === this.connectionId) {
            this.onError.dispatch(errorInfo.error);
            console.log("Error: " + errorInfo.error);
        }
    }

    onConnectComplete(connectionInfo) {
        console.log("onConnectComplete", connectionInfo.connectionId);
        connectionId = connectionInfo.connectionId;

        chrome.serial.onReceive.addListener(this.onReceive);
        chrome.serial.onReceiveError.addListener(this.onReceiveError);
    }

    onSendCallback(sendInfo) {
        console.log("sendInfo", sendInfo);
    }

    onReadLine(line) {
        console.log(line);
    }

    connectSelected(devicePath, connect=true) {
        
        if (connect == true) {
            console.log("Connecting", devicePath);
            chrome.serial.connect(devicePath, {bitrate: 115200}, this.onConnectComplete);

        } else {
            console.log("Disconnect" + devicePath);
            if (connectionId < 0) {
                console.log("connectionId", connectionId);
                return;
            }
            encodedBuffer = "";
            chrome.serial.onReceive.removeListener(this.onReceive);
            chrome.serial.onReceiveError.removeListener(this.onReceiveError);
            chrome.serial.flush(connectionId, function () {
                console.log("chrome.serial.flush", connectionId);
            });
            chrome.serial.disconnect(connectionId, function () {
                console.log("chrome.serial.disconnect", connectionId);
            });
        }

    }

    setupSerial() {
        chrome.serial.getDevices(this.onGetDevices);
    }
}
