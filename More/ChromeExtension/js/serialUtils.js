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
    }

    setupSelect(parentId) {
        var displayOptions = document.createElement('select'); //Element ready to be appended
        displayOptions.setAttribute('id','serialports')
        var frag = document.createDocumentFragment();
        frag.appendChild(displayOptions);
        document.getElementById(parentId).innerHTML = '<button id="refreshSerial">Get</button><button id="connectSerial">Set</button>';
        document.getElementById(parentId).appendChild(frag);

        document.getElementById('refreshSerial').onclick = () => {
            this.setupSerial();
        }
        document.getElementById('connectSerial').onclick = () => {
            if(this.connectionId != -1 ) {this.connectSelected(false)}; // Disconnect previous
            this.connectSelected(true, document.getElementById('serialports').value);
        }
    }

    onGetDevices = (ports) => {
        document.getElementById('serialports').innerHTML = '';
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

    onReceive = (receiveInfo) => {
        //console.log("onReceive");
        //if (receiveInfo.connectionId !== this.connectionId) {
        //    console.log("ERR: Receive ID:", receiveInfo.connectionId);
        //    return;
        //}
        var bufView = new Uint8Array(receiveInfo.data);
        var encodedString = String.fromCharCode.apply(null, bufView);

        this.encodedBuffer += decodeURIComponent(escape(encodedString));
        //console.log(this.encodedBuffer.length);
        var index;
        while ((index = this.encodedBuffer.indexOf('\n')) >= 0) {
            var line = this.encodedBuffer.substr(0, index + 1);
            this.onReadLine(line);
            this.encodedBuffer = this.encodedBuffer.substr(index + 1);
        }
    }

    onReceiveError(errorInfo) {
        console.log("onReceiveError");
        if (errorInfo.connectionId === this.connectionId) {
            console.log("Error from ID:", errorInfo.connectionId)
            this.onError.dispatch(errorInfo.error);
            console.log("Error: " + errorInfo.error);
        }
    }

    finalCallback() { //Customize this one for the front end integration after the device is successfully connected.
        console.log("USB device Ready!")
    }

    onConnectComplete = (connectionInfo) => {
        this.connectionId = connectionInfo.connectionId;
        console.log("Connected! ID:", this.connectionId);

        chrome.serial.onReceive.addListener(this.onReceive);
        chrome.serial.onReceiveError.addListener(this.onReceiveError);

        this.finalCallback()
    }

    sendMessage(msg) {
        msg+="\n";
        if (typeof chrome.serial !== 'undefined' && chrome.serial !== null) {
            if (this.connectionId > -1) {
                var encodedString = unescape(encodeURIComponent(msg));
                var bytes = new Uint8Array(encodedString.length);
                for (var i = 0; i < encodedString.length; ++i) {
                    bytes[i] = encodedString.charCodeAt(i);
                }
                chrome.serial.send(this.connectionId, bytes.buffer, this.onSendCallback);
                console.log("Send message:", msg);
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

    connectSelected(connect=true, devicePath='') { //Set connect to false to disconnect  
        if ((connect == true) && (devicePath != '')) {
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
