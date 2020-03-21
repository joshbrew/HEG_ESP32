//Utils developed by Diego Schmaedech (MIT License). Modified/Generalized by Joshua Brewster (MIT License)
class chromeSerial {
    constructor(defaultUI=true, parentId='serialmenu') {
        this.displayPorts = [];
        this.defaultUI = defaultUI;

        this.encodedBuffer = "";
        this.connectionId = -1;
        if (typeof chrome.serial !== 'undefined' && chrome.serial !== null) {
            if(defaultUI == true) {
                this.setupSelect(parentId);
            }
            this.setupSerial();
        }  
        else {
            console.log("ERROR: Cannot locate chrome.serial.");
        }

        document.getElementById('serialports').onclick = () => {
            
        }
    }

    
    setupSelect(parentId) {
        var displayOptions = document.createElement('select'); //Element ready to be appended
        displayOptions.setAttribute('id','serialports')
        var frag = document.createDocumentFragment();
        frag.appendChild(displayOptions);
        document.getElementById(parentId).innerHTML = '<button id="connectSerial">Set</button>';
        document.getElementById(parentId).appendChild(frag);

        document.getElementById('connectSerial').onclick = () => {
            this.connectSelected(false); // Disconnect previous
            this.connectSelected(true, document.getElementById('serialports').value);
        }
    }


    onGetDevices = (ports) => {
        var paths = [];
        for (var i = 0; i < ports.length; i++) {
            console.log(ports[i].path);
        }
        ports.forEach((port) => {
            var displayName = port["displayName"] + "(" + port.path + ")";
            console.log("displayName " + displayName);
            if (!displayName)
                displayName = port.path;  
            paths.push({'option':displayName, 'value':port.path});
            console.log(this.defaultUI);
            if(this.defaultUI == true) {
                var newOption = document.createElement("option");
                newOption.text = displayName;
                newOption.value = port.path;
                console.log('option', newOption);
                document.getElementById('serialports').appendChild(newOption);
            }
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

        this.encodedBuffer += decodeURIComponent(escape(encodedString));

        var index;
        while ((index = this.encodedBuffer.indexOf('\n')) >= 0) {
            var line = this.encodedBuffer.substr(0, index + 1);
            onReadLine(line);
            this.encodedBuffer = this.encodedBuffer.substr(index + 1);
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
        this.connectionId = connectionInfo.connectionId;

        chrome.serial.onReceive.addListener(this.onReceive);
        chrome.serial.onReceiveError.addListener(this.onReceiveError);
    }

    sendMessage(msg) {
        if (typeof chrome.serial !== 'undefined' && chrome.serial !== null) {
            if (this.connectionId > -1) {
                console.log("Send Message: ", msg);
                var encodedString = unescape(encodeURIComponent(msg));
                var bytes = new Uint8Array(encodedString.length);
                for (var i = 0; i < encodedString.length; ++i) {
                    bytes[i] = encodedString.charCodeAt(i);
                }
                chrome.serial.send(this.connectionId, bytes.buffer, this.onSendCallback);
                console.log("Send message", msg);
            } else {
                console.log("Device is disconnected!");
            }
        }
    }

    onSendCallback(sendInfo) {
        console.log("sendInfo", sendInfo);
    }

    onReadLine(line) {
        console.log(line);
    }

    connectSelected(connect=true, devicePath=null) { //Set connect to false to disconnect  
        if ((connect == true) && (devicePath != null)) {
            console.log("Connecting", devicePath);
            chrome.serial.connect(devicePath, {bitrate: 115200}, this.onConnectComplete);
        } else {
            console.log("Disconnect" + devicePath);
            if (this.connectionId < 0) {
                console.log("connectionId", this.connectionId);
                return;
            }
            this.encodedBuffer = "";
            chrome.serial.onReceive.removeListener(this.onReceive);
            chrome.serial.onReceiveError.removeListener(this.onReceiveError);
            chrome.serial.flush(this.connectionId, function () {
                console.log("chrome.serial.flush", this.connectionId);
            });
            chrome.serial.disconnect(this.connectionId, function () {
                console.log("chrome.serial.disconnect", this.connectionId);
            });
        }
    }

    setupSerial() {
        chrome.serial.getDevices(this.onGetDevices);
    }
}
