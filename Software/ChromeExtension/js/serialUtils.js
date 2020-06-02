//Utils developed by Diego Schmaedech (MIT License). Modified/Generalized by Joshua Brewster (MIT License)
class chromeSerial {
    constructor(defaultUI=true, parentId='serialmenu', streamMonitorId="serialmonitor") {
        this.displayPorts = [];
        this.defaultUI = defaultUI;

        this.encodedBuffer = "";
        this.connectionId = -1;

        this.recordData = false;
        this.recorded = [];

        this.monitoring = false;
        this.newSamples = 0;
        this.monitorSamples = 10000; //Max 10000 samples visible in stream monitor by default
        this.monitorData = [];
        this.monitorIdx = 0;

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

    setupMonitor(parentId) {

        if(this.monitorData.length > this.monitorSamples){ 
            this.monitorData.splice(0, this.monitorData.length - this.monitorSamples);
        }

        var div = document.createElement('div');
        div.setAttribute('id','streamMonitor');
        this.monitorData.forEach((item,idx)=>{
            div.innerHTML += '<div id='+this.monitorIdx+'>'+item+'</div>';
            this.monitorIdx++;
        });
        this.newSamples = 0;
        var frag = document.createDocumentFragment();
        frag.appendChild(div);
        
        document.getElementById(parentId).appendChild(frag);

        var monitorAnim = () => {
            if(this.newSamples > 0){
                if(this.monitorData.length > this.monitorSamples){ 
                    //Remove old samples if over the limit
                    for(var i = this.monitorIdx - this.monitorSamples - (this.monitorData.length - this.monitorSamples); i > this.monitorIdx - this.monitorSamples; i++){
                        document.getElementById(i).remove();
                    }
                    this.monitorData.splice(0, this.monitorData.length - this.monitorSamples);
                }
                //Load new samples
                for(var i = 0; i < newSamples; i++) {
                    var newdiv = document.createElement('div');
                    newdiv.innerHTML = '<div id="'+this.monitorIdx+'">'+this.monitorData[this.monitorData.length - 1 - i]+'</div>';
                    var frag = document.createDocumentFragment();
                    frag.appendChild(newdiv);        
                    document.getElementById(parentId).appendChild(frag);
                    this.monitorIdx++;

                    var elem = document.getElementById('streamMonitor');
                    elem.scrollTop = elem.scrollHeight;
                }
                setTimeout(requestAnimationFrame(monitorAnim),15);
            }
        }
        requestAnimationFrame(monitorAnim);
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
            if(this.recordData == true) {
                this.recorded.push(line);
            }
            if(this.monitoring = true){
                this.newSamples++;
                this.monitorData.push(line);
            }
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

    saveCsv(data=this.recorded, name=new Date().toISOString(),delimiter="|",header="Header\n"){
        var csvDat = header;
        data.forEach((line) => {
            csvDat += line.split(delimiter).join(",")+"\n";
        });

        var hiddenElement = document.createElement('a');
        hiddenElement.href = "data:text/csv;charset=utf-8," + encodeURI(csvDat);
        hiddenElement.target = "_blank";
        if(name != ""){
            hiddenElement.download = name+".csv";
        }
        else{
            hiddenElement.download = new Date().toISOString()+".csv";
        }
        hiddenElement.click();
    }

    openFile(delimiter=",") {
        var input = document.createElement('input');
        input.type = 'file';
    
        input.onchange = e => {
        this.csvDat = [];
        var file = e.target.files[0];
        var reader = new FileReader();
        reader.readAsText(file);
        reader.onload = event => {
          var tempcsvData = event.target.result;
          var tempcsvArr = tempcsvData.split("\n");
          tempcsvArr.pop();
          tempcsvArr.forEach((row,i) => {
            if(i==0){ var temp = row.split(delimiter); }
            else{
              var temp = row.split(delimiter);
              this.csvDat.push(temp);
            }
          });
          this.onOpen();
         }
         input.value = '';
        }
        input.click();
    } 

    onOpen() { // Customize this function in your init script, access data with ex. console.log(serialMonitor.csvDat), where var serialMonitor = new chromeSerial(defaultUI=false)
        alert("CSV Opened!");
    }
}
