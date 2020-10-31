class HEGwebAPI { //Create HEG sessions, define custom data stream params as needed.
  constructor(host='', header=["us","Red","IR","Ratio","Ambient","drdt","ddrdt"], delimiter="|", uIdx=0, rIdx=3, defaultUI=true, parentId="main_body"){
    
    this.alloutput = [];
    this.raw = [];
    this.filtered = [];
    
    this.clock = [];
    this.useMs = false;

    this.ratio=[];

    this.startTime=0;
    this.slowSMA = 0;
    this.slowSMAarr = [0];
    this.fastSMA = 0;
    this.fastSMAarr = [0];
    this.smaSlope = 0;
    this.scoreArr = [0];
    this.replay = false;
    
    this.csvDat = [];
    this.csvIndex = 0;

    this.curIndex = 0;
    this.noteIndex = [];
    this.noteText = [];

    this.host = host;
    this.source="";
    this.platform = navigator.userAgent.toLowerCase();

    this.sensitivity = null;

    this.header = header; //Data header for incoming data
    this.delimiter = delimiter; //Data stream delimiter
    this.uIdx = uIdx; //Index of timing value (microseconds presumed)
    this.rIdx = rIdx; //Index of HEG Ratio

    window.requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.msRequestAnimationFrame;
    window.cancelAnimationFrame = window.cancelAnimationFrame || window.webkitCancelAnimationFrame || window.mozCancelAnimationFrame || window.msCancelAnimationFrame;
    
    this.defaultUI = defaultUI;
    this.ui = false;
    if(defaultUI==true){
      this.parentId = parentId;
      this.createUI(parentId,header);
    }
    /*
    window.addEventListener('message', e => {
      if(e.origin.startsWith(window.location.hostname)) {
        
      }
      else {
        this.handleEventData(e); 
      }
    }); //Generic event listener for postMessage events (for iframes)
    */
    this.createEventListeners(host);
  }

  resetVars() {
    this.startTime = 0;

    this.alloutput = [];
    this.raw = [];
    this.filtered = [];
    this.clock = [];
    this.ratio = [];
    
    this.slowSMA = 0;
    this.slowSMAarr = [0];
    this.fastSMA = 0;
    this.fastSMAarr[0];
    this.smaSlope = 0;
    this.scoreArr = [0];
    this.replay = false;

    this.csvDat = [];
    this.csvIndex = 0;

    this.curIndex = 0;
    this.noteIndex = [];
    this.noteText = [];
  }

  //appendId is the element Id you want to append this fragment to
  static appendFragment(HTMLtoAppend, parentId) {

      var fragment = document.createDocumentFragment();
      var newDiv = document.createElement('div');
      newDiv.innerHTML = HTMLtoAppend;
      newDiv.setAttribute("id", parentId + '_child');

      fragment.appendChild(newDiv);

      document.getElementById(parentId).appendChild(fragment);
  }

  //delete selected fragment. Will delete the most recent fragment if Ids are shared.
  static deleteFragment(parentId,fragmentId) {
      var this_fragment = document.getElementById(fragmentId);
      document.getElementById(parentId).removeChild(this_fragment);
  }

  //Separates the appendId from the fragmentId so you can make multiple child threads with different Ids
  static appendFragmentMulti(HTMLtoAppend, parentId, fragmentId) {

      var fragment = document.createDocumentFragment();
      var newDiv = document.createElement('div');
      newDiv.innerHTML = HTMLtoAppend;
      newDiv.setAttribute("id", parentId + '_child');

      fragment.appendChild(newDiv);

      document.getElementById(parentId).appendChild(fragment);
  }

  //Remove Element By Id
  static removeElement(elementId) {
    // Removes an element from the document
    var element = document.getElementById(elementId);
    element.parentNode.removeChild(element);
  }

  //Remove Element Parent By Element Id (for those pesky anonymous child fragment containers)
  static removeParent(elementId) {
    // Removes an element from the document
    var element = document.getElementById(elementId);
    element.parentNode.parentNode.removeChild(element.parentNode);
  }

  //Remove the parent of a parent of an element (for even more pesky anonymous nested fragment containers) 
  static removeParentParent(elementId){
    var element = document.getElementById(elementId);
    element.parentNode.parentNode.parentNode.removeChild(element.parentNode.parentNode);

  }

  //Input data and averaging window, output array of moving averages (should be same size as input array, initial values not fully averaged due to window)
  static sma(arr, window) {
    var temp = []; //console.log(arr);
    for(var i = 0; i < arr.length; i++) {
      if((i == 0)) {
        temp.push(arr[0]);
      }
      else if(i < window) { //average partial window (prevents delays on screen)
        var arrslice = arr.slice(0,i+1);
        temp.push(arrslice.reduce((previous,current) => current += previous ) / (i+1));
      }
      else { //average windows
        var arrslice = arr.slice(i-window,i);
        temp.push(arrslice.reduce((previous,current) => current += previous) / window);
      }
    } 
    //console.log(temp);
    return temp;
  }

  smaScore(input) {
    this.slowSMAarr = HEGwebAPI.sma(input,40);
    this.fastSMAarr = HEGwebAPI.sma(input,20);
    this.fastSMA = this.fastSMAarr[this.fastSMAarr.length - 1];
    this.slowSMA = this.slowSMAarr[this.slowSMAarr.length - 1];

    this.smaSlope = this.fastSMA - this.slowSMA;
    //this.scoreArr.push(this.scoreArr[this.scoreArr.length-1]+this.smaSlope);
  }

  //Converts arrays of strings representing lines of data into CSVs
  saveCSV(data=this.filtered, name=new Date().toISOString(), delimiter=this.delimiter, header=this.header.join(",")+",Notes", saveNotes=true){
    var csvDat = header+"\n";
    data.forEach((line, i) => {
      csvDat += line.split(delimiter).join(",");
      if(saveNotes == true){
        if(this.noteIndex.indexOf(i) != -1) {csvDat+=","+[this.noteText[this.noteIndex.indexOf(i)]]}
      }
      if(line.indexOf('\n') < 0) {csvDat+="\n";}
    });

    var hiddenElement = document.createElement('a');
    hiddenElement.href = "data:text/csv;charset=utf-8," + encodeURI(csvDat);
    hiddenElement.target = "_blank";
    if(name != ""){
        hiddenElement.download = name+".csv";
    }
    else{
        hiddenElement.download = Date().toISOString()+".csv";
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

  handleScore() {
    //Define in-script
  }

  replayCSV() {
    if(this.csvIndex < 2){
      if(this.startTime == 0) { this.startTime = this.csvDat[this.csvIndex][0]}
      this.clock.push(parseInt(this.csvDat[this.csvIndex][0]));
      this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
    }
    this.csvIndex++;
    if(this.csvIndex < this.csvDat.length - 1){
      if(this.startTime == 0) { this.startTime = this.csvDat[this.csvIndex][0]}
      this.clock.push(parseInt(this.csvDat[this.csvIndex][0]));
      this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
      if(this.clock.length >= 2){
        this.handleScore();
        this.updateStreamRow(this.csvDat[this.csvIndex]);
      }
    }
    else {
      this.replay = false;
      this.csvDat = [];
      this.csvIndex = 0;
    }
    //this.endOfEvent();
    if(this.useMs == true){ setTimeout(() => {this.replayCSV();},(this.clock[this.csvIndex]-this.clock[this.csvIndex-1])); } //Call until end of index.
    else{ setTimeout(() => {this.replayCSV();},(this.clock[this.csvIndex]-this.clock[this.csvIndex-1])*0.001); } //Call until end of index.
  }
  
  openCSV() {
    var input = document.createElement('input');
    input.type = 'file';

    input.onchange = e => {
    this.csvDat = [];
    this.csvIndex = 0;
    var file = e.target.files[0];
    var reader = new FileReader();
    reader.readAsText(file);
    reader.onload = event => {
      this.resetVars();
      var tempcsvData = event.target.result;
      var tempcsvArr = tempcsvData.split("\n");
      tempcsvArr.pop();
      tempcsvArr.forEach((row,i) => {
       if(i==0){ var temp = row.split(","); }
        else{
          var temp = row.split(",");
          this.csvDat.push(temp);
       }
      });
      this.replay = true;
      this.replayCSV();
     }
     input.value = '';
    }
    input.click();
  }

  handleEventData(data, delimiter="|", uIdx=0, rIdx=3){ // Can set custom delimiters, time counters (us) index, and ratio index of incoming data.
    console.log("HEGDUINO", data);
    if(this.raw[this.raw.length - 1] != data){  //on new output
      if(this.ui == true){
        document.getElementById("heg").innerHTML = data;
      }
      //Create event for posting data from an iframe implementation of this code.
      var onRead = new CustomEvent('on_read', { detail: {data: data} }); 
      window.parent.dispatchEvent(onRead); 
      window.parent.postMessage(data, "*");

      if(data.includes(delimiter)) { //Checks that it's a data line of specified format
        this.raw.push(data);
        var dataArray = data.split(delimiter);
        var thisRatio = parseFloat(dataArray[rIdx]);
        if(thisRatio > 0) { 
          if(this.startTime == 0) { this.startTime = parseInt(dataArray[0]); }
          this.clock.push(parseInt(dataArray[uIdx]));
          this.ratio.push(parseFloat(dataArray[rIdx]));

          if(this.clock.length > 5) { // SMA filtering for ratio
            var temp = HEGwebAPI.sma(this.ratio.slice(this.ratio.length - 5, this.ratio.length), 5); 
            //console.log(temp);
            if((this.ratio[this.ratio.length - 1] < temp[4] * 0.7) || (this.ratio[this.ratio.length - 1] > temp[4] * 1.3)) {
              this.ratio[this.ratio.length - 1] = this.ratio[this.ratio.length - 2]; // Roll the ratio back if outside margin 
              dataArray[rIdx] = temp;
            } 
            this.filtered.push(dataArray.join(delimiter));
          }
          //handle new data
          this.handleScore();
          if(this.defaultUI == true){
            this.updateStreamRow(dataArray);
          }
        } 
      }
    }
    //handle if data not changed
    else if (this.replay == false) {
      //s.smaSlope = s.scoreArr[s.scoreArr.length - 1];
      //g.graphY1.shift();
      //g.graphY1.push(s.scoreArr[s.scoreArr.length - 1 - g.xoffset]);
      //s.scoreArr.push(s.smaSlope);
    }
    this.endOfEvent();
  }

  endOfEvent() {
    // Define in-script
  }

  openEvent = (e) => {
    this.alloutput.push(e.data);
    console.log("HEGDUINO", "Events Connected");
    //document.getElementById("message").innerHTML = "Output:";
  }

  errorEvent = (e) => {
    if (e.target.readyState != EventSource.OPEN) {
      this.alloutput.push(e.data);
      console.log("HEGDUINO", "Events Disconnected");
    }
  }

  messageEvent = (e) => {
    console.log("HEGDUINO", e.data);
    this.alloutput.push(e.data);
    //document.getElementById("message").innerHTML = e.data;
  }

  //Event with incoming data
  hegEvent = (e) => {
    this.handleEventData(e.data,this.delimiter,this.uIdx,this.rIdx);
  }

  createEventListeners(host='') { //Set custom hostname (e.g. http://192.168.4.1). Leave blank for local hosted sessions (i.e. served from the board)
    if (!!window.EventSource) {
      this.source = new EventSource(host+'/events');
      this.source.addEventListener('open', this.openEvent, false);
      this.source.addEventListener('error', this.errorEvent, false);
      this.source.addEventListener('message', this.messageEvent, false);
      this.source.addEventListener('heg', this.hegEvent, false);
    }
  }

  removeEventListeners() {
    if(window.EventSource){
    this.source.close();
    this.source.removeEventListener('open', this.openEvent, false);
    this.source.removeEventListener('error', this.errorEvent, false);
    this.source.removeEventListener('message', this.messageEvent, false);
    this.source.removeEventListener('heg', this.hegEvent, false);
    }
  }

  updateStreamHeader(header=this.header){
    var HTMLtoAppend = '<tr>';
    header.forEach((value)=>{HTMLtoAppend += '<th>'+value+'</th>'});
    HTMLtoAppend += '</tr>'
    document.getElementById("dataNames").innerHTML = HTMLtoAppend;
  }

  updateStreamRow(dataArray){
    var HTMLtoAppend = '<tr>';
    dataArray.forEach((value)=>{HTMLtoAppend += '<td>'+value+'</td>'});
    HTMLtoAppend += '</tr>'
    document.getElementById("dataTable").innerHTML = HTMLtoAppend;
  }

  makeStreamTable(header=this.header){
    var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"></table></div>';
    var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><td>Awaiting Data...</td></tr></table></div>';
    HEGwebAPI.appendFragment(tableHeadHTML,"sTableContainer");
    HEGwebAPI.appendFragment(tableDatHTML,"sTableContainer");
    this.updateStreamHeader(header);
    this.defaultUI = true; //sets defaultUI to true for streaming
  }

  createUI(parentId,header=this.header) {
    var hegapiHTML = '<div id="hegapi" class="hegapi"> \
      <table> \
      <tr><td><button id="startbutton" class="button startbutton">Start HEG</button></td> \
        <td><button id="stopbutton" class="button stopbutton" type="submit">Stop HEG</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr id="commandrow"><td><input type="text" id="command" name="command" placeholder="Command"></td><td><button id="sendbutton" class="button sendbutton">Send</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2"><button class="button" id="resetSession" name="resetSession">Reset Session</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2" id="sensitivityLabel">Scoring Sensitivity</td></tr> \
      <tr id="sensitivityrow"><td><button class="button" id="reset_s">Default</button></td> \
        <td>Sensitivity: <span id="sensitivityVal">1.00</span><br><input type="range" class="slider" id="sensitivity" min="1" max="1000" value="100"></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr id="timerow"><td><div id="timestamp">Get Current Time</div></td><td><button id="getTime" class="button">Get Time</button></td></tr> \
      <tr id="noterow"><td colspan="2"><textarea id="noteText" placeholder="Point of Interest"></textarea></td></tr>\
      <tr id-"savenoterow"><td colspan="2"><button id="saveNote" class="button">Annotate</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr id="csvrow"><td ><input type="text" id="csvname" name="csvname" placeholder="session_data"></input></td> \
        <td><button class="button saveLoadButtons" id="savecsv">Save CSV</button></td></tr> \
      <tr><td colspan="2"><button class="button saveLoadButtons" id="replaycsv">Replay CSV</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2" id="hostlabel">Host</td></tr> \
      <tr id="hostrow"><td><input type="text" id="hostname" name="hostname" placeholder="http://192.168.4.1"></input></td><td><button id="submithost" class="button">Connect</button></td></tr>  \
      <tr><td colspan="2"><hr></td></tr> \
      </table></div> \
      <iframe name="dummyframe" id="dummyframe" class="dummy"></iframe> \
      ';

    var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>';
    var tableHTML = '<div id="rawDiv"><div id="sTableContainer"></div></div>';
    var messageHTML = '<msgDiv id="message">Output:</div>';
    var eventHTML = '<eventDiv id="heg">Not connected...</eventDiv>';

    HEGwebAPI.appendFragment(dataDivHTML, parentId);
    HEGwebAPI.appendFragment(hegapiHTML, "dataDiv");
    HEGwebAPI.appendFragment(tableHTML, "dataDiv");
    HEGwebAPI.appendFragment(messageHTML,"rawDiv");
    HEGwebAPI.appendFragment(eventHTML,"rawDiv");

    this.makeStreamTable(header);
    
    document.getElementById("getTime").onclick = () => {
      this.curIndex = this.clock.length - 1;
      if(this.useMs == true) { document.getElementById("timestamp").innerHTML = (this.clock[this.clock.length - 1] * 0.001).toFixed(2) + "s"; }
      else { document.getElementById("timestamp").innerHTML = (this.clock[this.clock.length - 1] * 0.000001).toFixed(2) + "s"; }
    }

    document.getElementById("saveNote").onclick = () => {
      this.noteIndex.push(this.curIndex);
      this.noteText.push(document.getElementById("noteText").value);
      document.getElementById("noteText").value = "";
      this.curIndex = -1;
      document.getElementById("timestamp").innerHTML = "Get Current Time";
    }

    document.getElementById("savecsv").onclick = () => {this.saveCSV();}
    document.getElementById("replaycsv").onclick = () => {this.openCSV();}

    document.getElementById("resetSession").onclick = () => {this.resetVars();}

    this.sensitivity = document.getElementById("sensitivity");
    document.getElementById("reset_s").onclick = () => { 
      this.sensitivity.value = 100; 
      document.getElementById("sensitivityVal").innerHTML = (this.sensitivity.value * 0.01).toFixed(2);
    }
    document.getElementById("sensitivity").oninput = () => {
      document.getElementById("sensitivityVal").innerHTML = (this.sensitivity.value * 0.01).toFixed(2);
    }

    document.getElementById("startbutton").onclick = () => {
      var xhr = new XMLHttpRequest();
      xhr.open('POST', this.host+"/startHEG", true);
      xhr.send();
      xhr.onerror = function() { xhr.abort(); } //Make sure it doesn't keep the connection open
      //xhr.abort();
    }

    document.getElementById("stopbutton").onclick = () => {
      var xhr = new XMLHttpRequest();
      xhr.open('POST', this.host+"/stopHEG", true);
      xhr.send();
      xhr.onerror = function() { xhr.abort(); }
      //xhr.abort();
    }
    document.getElementById("sendbutton").onclick = () => {
      var data = new FormData();
      data.append('command', document.getElementById('command').value);
      var xhr = new XMLHttpRequest();
      xhr.open('POST', this.host+'/command', true);
      xhr.send(data);
      xhr.onerror = function() { xhr.abort(); }
      //xhr.abort();
    }

    document.getElementById("submithost").onclick = () => {
      if(window.EventSource){ 
        this.removeEventListeners();
      }
      if(document.getElementById("hostname").value.length > 2) {
        this.host = document.getElementById("hostname").value;
      }
      else{
        this.host = "http://192.168.4.1";
      }

      document.getElementById("startbutton").onclick = () => {
        var xhr = new XMLHttpRequest();
        xhr.open('POST', this.host+"/startHEG", true);
        xhr.send();
        xhr.onerror = function() { xhr.abort(); } //Make sure it doesn't keep the connection open
        //xhr.abort();
      }
  
      document.getElementById("stopbutton").onclick = () => {
        var xhr = new XMLHttpRequest();
        xhr.open('POST', this.host+"/stopHEG", true);
        xhr.send();
        xhr.onerror = function() { xhr.abort(); }
        //xhr.abort();
      }
      document.getElementById("sendbutton").onclick = () => {
        var data = new FormData();
        data.append('command', document.getElementById('command').value);
        var xhr = new XMLHttpRequest();
        xhr.open('POST', this.host+'/command', true);
        xhr.send(data);
        xhr.onerror = function() { xhr.abort(); }
        //xhr.abort();
      }

      this.createEventListeners(this.host);
      //var thisNode = document.getElementById("dataDiv").parentNode.parentNode;
      //thisNode.removeChild(document.getElementById("dataDiv").parentNode);
      //this.createUI(thisNode.id);
      console.log("Attempting connection at " + this.host);
    }

    this.ui = true;
  }
}

class graphJS {
  constructor(nPoints=[1155], color=[0,255,0,1], yscale=1, res=[1400,600], parentId="main_body", canvasId="g", defaultUI=true){
    //WebGL graph based on: https://tinyurl.com/y5roydhe
    //HTML : <canvas id={canvasId}></canvas><canvas id={canvasId+"text"}></canvas>;
    this.gl,
    this.shaderProgram,
    this.vertices,
    this.canvas;
    this.animationId = null;

    this.canvasId = canvasId;
    this.textId = canvasId + "text";
    this.color = color;
    this.res = res;
          
    this.sampleRate = null;
    this.clock = 0;
    this.useMs = false; //Get input in microseconds instead
    this.ratio = 0;
    this.score = 0;
    this.viewing = 0;
    this.autoscale = true;

    this.nPoints = nPoints;
    this.VERTEX_LENGTH = nPoints;
    this.graphY1 = [...Array(this.VERTEX_LENGTH).fill(0)];
    this.graphY2 = [...Array(this.VERTEX_LENGTH).fill(0)];
    
    this.yscale = yscale;
    this.invScale = 1/this.yscale;
    this.xoffset = 0; //Index offset
    this.yoffset = 0;

    this.xAxis = new Float32Array([
    -1.0,0.0,
    1.0,0.0
    ]);

    this.yAxis = new Float32Array([
    -0.765,-1.0,
    -0.765,1.0
    ]);

    this.xgradient = new Float32Array([
    -1.0,-0.75,
    1.0,-0.75,
    -1.0,-0.5,
    1.0,-0.5,
    -1.0,-0.25,
    1.0,-0.25,
    -1.0,0.25,
    1.0,0.25,
    -1.0,0.5,
    1.0,0.5,
    -1.0,0.75,
    1.0,0.75
    ]);

    this.ygradient = new Float32Array([
      -0.5,-1.0,
      -0.5,1.0,
      0.0,-1.0,
      0.0,1.0,
      0.5,-1.0,
      0.5,1.0
    ]);

    this.VERTEX_SHADER = `
      attribute vec4 coords;
      void main(void) {
        gl_Position = coords;
      }
    `;

    this.FRAGMENT_SHADER = `
      precision mediump float;
      uniform vec4 color;
      void main(void) {
        gl_FragColor = color;
      }
    `;

    var shaderHTML = '<div id="shaderContainer"> \
    <canvas class="webglcss" id="'+this.canvasId+'"></canvas><canvas class="webglcss" id="'+this.canvasId+'text"></canvas></div>' 
    HEGwebAPI.appendFragment(shaderHTML,parentId);

    this.defaultUI = defaultUI;
    if(defaultUI == true){
      this.createUI(parentId);
    }
    this.initGL(canvasId);
    this.createShader();
    this.createVertices(color);
    
    this.graphtext = document.getElementById(this.textId).getContext("2d");

    this.draw();

  }

  resetVars() {
    this.startTime = null;
    this.clock = 0;
    this.ratio = 0;
    this.score = 0;
    this.viewing = 0;
    this.VERTEX_LENGTH = this.nPoints;

    this.graphY1 = [...Array(this.VERTEX_LENGTH).fill(0)];
    this.graphY2 = [...Array(this.VERTEX_LENGTH).fill(0)];

    this.xoffset = 0; //Index offset
    this.yoffset = 0;
  }

  createUI(parentId){
    var graphOptions = '<div class="scale"> \
      <table id="graphSliderTable"> \
      <tr><td id="xoffsettd">X Offset:<br><input type="range" class="slider" id="xoffset" min=0 max=1000 value=0></td><td><button id="xoffsetbutton" class="button">Reset</button></td></tr> \
      <tr><td id="xscaletd">X Scale:<br><input type="range" class="slider" id="xscale" min=10 max='+(this.VERTEX_LENGTH * 5).toFixed(0)+' value='+this.VERTEX_LENGTH.toFixed(0)+'></td><td><button id="xscalebutton" class="button">Reset</button></td></tr> \
      <tr><td id="yoffsettd">Y Offset:<br><input type="range" class="slider" id="yoffset" min=0 max=10000 value=5000></td><td><button id="yoffsetbutton" class="button">Reset</button></td></tr> \
      <tr><td id="yscaletd">Y Scale:<br><input type="range" class="slider" id="yscale" min=1 max=400 value='+this.yscale*200+'></td><td><button id="yscalebutton" class="button">Reset</button></td></tr> \
      </table><br> \
      <table id="graphViewTable"><tr><td id="autoscaletd">Autoscale:<input type="checkbox" id="autoscale" checked></td><td>View:</td><td><form name="graphform">Score<input type="radio" name="graphview" value="0" checked>Ratio<input type="radio" name="graphview" value="1"></form></td></tr></table> \
      </div> \
    ';

    HEGwebAPI.appendFragment(graphOptions, parentId);

    this.xoffsetSlider = document.getElementById("xoffset");
    this.xscaleSlider = document.getElementById("xscale");
    this.yoffsetSlider = document.getElementById("yoffset");
    this.yscaleSlider = document.getElementById("yscale");

    this.yoffsetSlider.oninput = () => {
      this.yoffset = (this.yoffsetSlider.value - 5000) * 0.002;
    }

    this.yscaleSlider.oninput = () => {
      this.yscale = this.yscaleSlider.value * .005;
      this.invScale = 1/this.yscale;
    }

    document.getElementById("yoffsetbutton").onclick = () => {
      this.yoffset = 0;
    }

    document.getElementById("yscalebutton").onclick = () => {
      this.yscaleSlider.value = 200;
      this.yscale = 1;
      this.invScale = 1;
    }

    document.getElementById("autoscale").onclick = () => {
      this.autoscale = document.getElementById("autoscale").checked;
    }
    var radios = document.graphform.graphview;
    for (var i = 0; i < radios.length; i++) {
      radios[i].addEventListener('change', () => {
          this.viewing = radios.value;
      });
    }
  }

  setCanvasSize() {
    this.canvas.width = this.res[0];
    this.canvas.height = this.res[1];
    this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
  }

  initGL(canvasId) {
    this.canvas = document.querySelector('#'+canvasId);
    this.gl = this.canvas.getContext('webgl');
    this.setCanvasSize();

      console.log(this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
      this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
      this.gl.enable(this.gl.BLEND);
      this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
      this.gl.clearColor(0, 0, 0, 1);
  }
    
  makePoints(numPoints, yArr) {
    const highestPointNdx = numPoints - 1;
    return Array.from({length: numPoints * 2}, (_, i) => {
      const pointId = i * 0.5 | 0;
      const lerp0To1 = pointId / highestPointNdx;
      const isY = i % 2;
      if(this.autoscale == true){
        if(yArr[i] > this.invScale){
          this.invScale = yArr[i];
          this.yscale = 1/this.invScale;
          this.yscaleSlider.value = this.yscale * 200;
        }
        else if(yArr[i] < 0-this.invScale) {
          this.invScale = 0-yArr[i];
          this.yscale = 1/this.invScale;
          this.yscaleSlider.value = this.yscale * 200;
        }
      }
      return isY
        ? (yArr[i]*this.yscale + this.yoffset) // Y
        : (lerp0To1 * 4 - 1); // X
    });
  }

  createVertices(vArr,color=this.color) {
    const buffer = this.gl.createBuffer();
    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, buffer);
    this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(vArr), this.gl.DYNAMIC_DRAW);

    const coords = this.gl.getAttribLocation(this.shaderProgram, 'coords');
    this.gl.vertexAttribPointer(coords, 2, this.gl.FLOAT, false, 0, 0);
    this.gl.enableVertexAttribArray(coords);
    // gl.bindBuffer(gl.ARRAY_BUFFER, null);

    const uniformColor = this.gl.getUniformLocation(this.shaderProgram, 'color');
    this.gl.uniform4f(uniformColor, this.normalize(color[0]), this.normalize(color[1]), this.normalize(color[2]), color[3]);
  }

  createShader() {
    const vs = this.VERTEX_SHADER;

    const vertexShader = this.gl.createShader(this.gl.VERTEX_SHADER);
    this.gl.shaderSource(vertexShader, vs);
    this.gl.compileShader(vertexShader);

    const fs = this.FRAGMENT_SHADER;

    const fragmentShader = this.gl.createShader(this.gl.FRAGMENT_SHADER);
    this.gl.shaderSource(fragmentShader, fs);
    this.gl.compileShader(fragmentShader);

    this.shaderProgram = this.gl.createProgram();
    this.gl.attachShader(this.shaderProgram, vertexShader);
    this.gl.attachShader(this.shaderProgram, fragmentShader);

    this.gl.linkProgram(this.shaderProgram);
    this.gl.useProgram(this.shaderProgram);
  }

  deInit() {
    cancelAnimationFrame(this.animationId);
  }

  changeArr = (newArr) => {
    if(newArr.length < 10) {
      console.log("new array too small, needs 10 points minimum");
    }
    else {
      if(this.VERTEX_LENGTH > newArr.length){
        this.VERTEX_LENGTH = newArr.length;
      }
      if(this.viewing == 0){
        for(var i = 1; i <= this.VERTEX_LENGTH; i++) {
          this.graphY1[i] = newArr[newArr.length - 1 - (this.VERTEX_LENGTH - i)];
        }
      }
      if(this.viewing == 1){
        for(var i = 1; i <= this.VERTEX_LENGTH; i++) {
          this.graphY2[i] = newArr[newArr.length - 1 - (this.VERTEX_LENGTH - i)];
        }
      }
    }
  }

  normalize(val, max=255, min=0) { return (val - min) / (max - min); }

  draw = () => {
    //Create main graph
    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

    //xAxis
    this.createVertices(this.xAxis,[255,255,255,0.6]);
    this.gl.drawArrays(this.gl.LINES, 0, 2);

    //yAxis
    this.createVertices(this.yAxis, [255,255,255,0.6]);
    this.gl.drawArrays(this.gl.LINES, 0, 2);

    //xgradient
    this.createVertices(this.xgradient, [70,70,70,0.8]);
    this.gl.drawArrays(this.gl.LINES, 0, 12);

    //ygradient
    this.createVertices(this.ygradient, [70,70,70,0.8]);
    this.gl.drawArrays(this.gl.LINES, 0, 6);
    
    //Data line
    if(this.viewing == 0){
      this.vertices = this.makePoints(this.VERTEX_LENGTH, this.graphY1);
    }
    if(this.viewing == 1){
      this.vertices = this.makePoints(this.VERTEX_LENGTH, this.graphY2);
    }
    this.createVertices(this.vertices);
    this.gl.bufferSubData(this.gl.ARRAY_BUFFER, 0, new Float32Array(this.vertices));
    this.gl.drawArrays(this.gl.LINE_STRIP, 0, this.VERTEX_LENGTH);
    //Create text overlay -- TODO: Only update on change so it's not constantly redrawing
    this.graphtext.clearRect(0, 0, this.canvas.width, this.canvas.height);

    if(window.innerWidth > 700){
      this.graphtext.canvas.height = this.canvas.height*(window.innerHeight/window.innerWidth)*1.3;
    }
    else{
      this.graphtext.canvas.height = this.canvas.height;
    }
    this.graphtext.canvas.width = this.canvas.width*1.3;
    this.graphtext.font = "2em Arial";

    var seconds = 0;
    if(this.useMs == true){
      seconds = Math.floor(this.clock*0.001);
    }
    else {
      seconds = Math.floor(this.clock*0.000001);
    }
    var minutes = Math.floor(seconds*0.01667);
    seconds = seconds - minutes * 60
    if(seconds < 10){seconds = "0"+seconds}
    if(this.viewing == 0) {
      this.graphtext.fillStyle = "#00ff00";
      this.graphtext.fillText("  Time: " + minutes + ":" + seconds,this.graphtext.canvas.width - 300,50);
      this.graphtext.fillText("  Ratio: " + this.ratio.toFixed(2), this.graphtext.canvas.width - 500,50);
      this.graphtext.fillStyle = "#99ffbb";
      this.graphtext.fillText("    Score: " + this.graphY1[this.graphY1.length - 1].toFixed(2),this.graphtext.canvas.width - 720,50);
    }
    if(this.viewing == 1) {
      this.graphtext.fillStyle = "#00ff00";
      this.graphtext.fillText("  Time: " + minutes + ":" + seconds,this.graphtext.canvas.width - 300,50);
      this.graphtext.fillText("    Score: " + this.graphY1[this.graphY1.length - 1].toFixed(2) + "  ",this.graphtext.canvas.width - 720,50);
      this.graphtext.fillStyle = "#99ffbb";
      this.graphtext.fillText("   Ratio: " + this.ratio.toFixed(2), this.graphtext.canvas.width - 500,50);
    }
    this.graphtext.fillStyle = "#707070";
    var xoffset = this.graphtext.canvas.width * 0.125;
    this.graphtext.fillText((this.invScale * 0.75 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.125); 
    this.graphtext.fillText((this.invScale * 0.5 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.25); 
    this.graphtext.fillText((this.invScale * 0.25 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.375); 
    this.graphtext.fillText((this.invScale * -0.25 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.625); 
    this.graphtext.fillText((this.invScale * -0.5 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.75); 
    this.graphtext.fillText((this.invScale * -0.75 - this.yoffset).toFixed(3), xoffset, this.graphtext.canvas.height * 0.875); 
    
    if(this.sampleRate != null) { //X-axis approximation.
      this.graphtext.fillStyle = "#303030";
      this.graphtext.fillText((Math.ceil(this.sampleRate * this.VERTEX_LENGTH * 0.5)).toFixed(0)+"s", this.graphtext.canvas.width * 0.501, this.graphtext.canvas.height * 0.85);
      this.graphtext.fillText((Math.ceil(this.sampleRate * this.VERTEX_LENGTH * 0.25)).toFixed(0)+"s", this.graphtext.canvas.width * 0.751, this.graphtext.canvas.height * 0.85);
      this.graphtext.fillText((Math.ceil(this.sampleRate * this.VERTEX_LENGTH * 0.75)).toFixed(0)+"s", this.graphtext.canvas.width * 0.251, this.graphtext.canvas.height * 0.85);
    }
    //console.log("Graph updated", Date.now());
    setTimeout(()=>{this.animationId = requestAnimationFrame(this.draw);},40); 
    
  }
  
}
    
class circleJS {
  constructor(bgColor="#34baeb", cColor="#ff3a17", res=[window.innerWidth,"440"], parentId="main_body", canvasId="circlecanvas", defaultUI=true, canvasmenuId="circlecanvasmenu"){
    
    this.createCanvas(parentId, canvasId, res);
    this.c = document.getElementById(canvasId);
    this.ctx = this.c.getContext('2d');
    this.parentId = parentId;
    this.canvasId = canvasId;
    this.canvasmenuId = canvasmenuId;
    this.soundFX = null;
    
    this.defaultUI = defaultUI;
    this.hidden = false;
    if(defaultUI == true){  this.createUI(parentId);  }
 
    this.c.width = res[0];
    this.c.height = res[1];

    this.animationId = null;

    this.angle = 1.57;
    this.angleChange = 0;

    this.bgColor = bgColor;
    this.cColor = cColor;
    this.draw();
  }

  createCanvas(parentId,canvasId,res=["600","600"]){
    var canvasHTML = '<div id="canvasContainer" class="canvasContainer"><canvas class="circlecss" id="'+canvasId+'" height="'+res[1]+'" width="'+res[0]+'"></canvas></div>';

    //Setup page as fragments so updates to elements are asynchronous.
    HEGwebAPI.appendFragment(canvasHTML,parentId);
  }

  createUI(parentId) {
    var uiHTML = '<div id="'+this.canvasmenuId+'" class="circleapi"> \
    <table id="circletable" class="circletable"> \
    <tr><td><button class="button" id="circleAudiobutton">Audio</button></td></tr> \
    </table> \
    </div> \
    <button id="showhide" name="showhide" class="showhide">Hide UI</button>';

    HEGwebAPI.appendFragment(uiHTML, parentId);
    
    document.getElementById("circleAudiobutton").style.opacity = 0.3;

    document.getElementById("circleAudiobutton").onclick = () => {
      if(this.soundFX == null) { 
        this.soundFX = new SoundJS(); 
        this.soundFX.gain.gain.value = 0.1;
        this.soundFX.playFreq([300]);
        document.getElementById("circleAudiobutton").style.opacity = 1.0;
      }
      else{
        if(this.soundFX.gain.gain.value == 0) {
          this.soundFX.gain.gain.value = 0.1;
          document.getElementById("circleAudiobutton").style.opacity = 1.0;
        }
        else {
          this.soundFX.gain.gain.value = 0;
          document.getElementById("circleAudiobutton").style.opacity = 0.3;
        }
      }
    }

    document.getElementById("showhide").onclick = () => {
      if(this.hidden == false){
        document.getElementById(canvasmenuId).style.display = 'none';
        document.getElementById("showhide").innerHTML = "Show UI";
        this.hidden = true;
      }
      else{
        document.getElementById(canvasmenuId).style.display = '';
        document.getElementById("showhide").innerHTML = "Hide UI";
        this.hidden = false;
      }
    }
  }

  deInit() {
    cancelAnimationFrame(this.animationId);
    if(this.soundFX != null){
      this.soundFX.osc[0].stop(0);
    }
  }

  onData(score){
    this.angleChange = score;
  }

  draw = () => {
    if(this.defaultUI){
      if(window.innerWidth >= 700) {
        this.c.width = window.innerWidth - 30;
      }
      else {
        this.c.width = 700;
      }
    }
      var cWidth = this.c.width;
      var cHeight = this.c.height;
      this.ctx.clearRect(0, 0, cWidth, cHeight);
       
      // color in the background
      this.ctx.fillStyle = this.bgColor;
      this.ctx.fillRect(0, 0, cWidth, cHeight);

      // style the background
      var gradient = this.ctx.createRadialGradient(cWidth*0.5,cHeight*0.5,2,cWidth*0.5,cHeight*0.5,100*this.angle*this.angle);
      gradient.addColorStop(0,"purple");
      gradient.addColorStop(0.25,"dodgerblue");
      gradient.addColorStop(0.32,"skyblue");
      gradient.addColorStop(1,this.bgColor);
      this.ctx.fillStyle = gradient;
      this.ctx.fillRect(0,0,cWidth,cHeight);
       
      // draw the circle
      this.ctx.beginPath();

      if(((this.angle > 1.57) || (this.angleChange > 0)) && ((this.angle < 3.14) || (this.angleChange < 0))){ //generalize
          this.angle += this.angleChange*0.1;
          if(this.soundFX != null){
            this.soundFX.osc[0].frequency.value += this.angleChange*100;
          }
      }

      var radius = cHeight*0.04 + (cHeight*0.46) * Math.abs(Math.cos(this.angle));
      this.ctx.arc(cWidth*0.5, cHeight*0.5, radius, 0, Math.PI * 2, false);
      this.ctx.closePath();
       
      // color in the circle
      this.ctx.fillStyle = this.cColor;
      this.ctx.fill();
      
      setTimeout(()=>{this.animationId = requestAnimationFrame(this.draw);},15); 
  }
}

class videoJS {
      constructor(res=["700","440"], parentId="main_body", vidapiId="vidapi", vidContainerId="vidbox", defaultUI=true){
        this.playRate = 1;
        this.alpha = 0;
        this.volume = 0.5;

        this.useAlpha = true;
        this.useRate = true;
        this.useVol = true;
        this.useTime = false;

        this.ampScore = 0;
        this.ampThreshold = 0;
        this.diff = 0;
        

        this.enableControls = false;
        this.parentId = parentId;
        this.vidapiId = vidapiId
        this.vidContainerId = vidContainerId;
        this.animationId = null;

        this.vidQuery;
        this.c;
        this.gl;

        var videoHTML = '<div class="vidbox" id="'+this.vidContainerId+'"><video id="'+this.vidContainerId+'vid" height="'+res[1]+'px" width="'+res[0]+'px" class="vidcss" src="https://vjs.zencdn.net/v/oceans.mp4" type="video/mp4" autoplay loop muted></video> \
        <canvas class="vidglcss" id="'+this.vidContainerId+'canvas"></canvas></div> \
        <input class="file_wrapper" id="fs" name="fs" type="file" accept="video/*"/>';
      
        HEGwebAPI.appendFragment(videoHTML,parentId);

        this.defaultUI = defaultUI;
        this.sliderfocus = false;
        this.hidden = false;
        if(defaultUI == true){
          this.addUI(parentId);
        }
        this.init(defaultUI);
      }

      startVideo = () => {
        if(this.playRate < 0.1){ this.vidQuery.playbackRate = 0; }
        else{ this.vidQuery.playbackRate = this.playRate; }
      }

      stopVideo = () => {
        this.vidQuery.playbackRate = 0;
      }

      setupButtons() {

        document.getElementById("startbutton").addEventListener('click', this.startVideo, false);
        
        document.getElementById("stopbutton").addEventListener('click', this.stopVideo, false);
        
        document.getElementById("play").onclick = () => {
          if(this.vidQuery.playbackRate == 0){
            if(this.useRate == true){
              this.vidQuery.playbackRate = this.playRate;
            }
            else {
              this.playRate = 1;
              this.vidQuery.playbackRate = 1;
            }
            document.getElementById("play").innerHTML = "||";
          }
          else{
            this.vidQuery.playbackRate = 0;
            document.getElementById("play").innerHTML = ">";
          }
        }
        
        document.getElementById("useAlpha").onclick = () => {
         if(this.useAlpha == true){
           this.useAlpha = false;
           this.alpha = 0;
           document.getElementById("useAlpha").style.opacity = "0.3";
         }
         else{ this.useAlpha = true; document.getElementById("useAlpha").style.opacity = "1.0";}
        }

        document.getElementById("useRate").onclick = () => {
         if(this.useRate == true){
           this.useRate = false;
           this.playRate = 1;
           this.vidQuery.playbackRate = 1;
           document.getElementById("useRate").style.opacity = "0.3";
         }
         else{ 
           this.useTime = false; 
           this.useRate = true; 
           this.playRate = 1; 
           this.vidQuery.playbackRate = 1;
           document.getElementById("useRate").style.opacity = "1.0";
           document.getElementById("useTime").style.opacity = "0.3";
          }
        }

        document.getElementById("useVol").onclick = () => {
         if(this.useVol == true){
           this.vidQuery.muted = true;
           this.useVol = false;
           this.volume = 0;
           this.vidQuery.volume = 0;
           document.getElementById("useVol").style.opacity = "0.3";
         }
         else{ 
          this.useVol = true; 
          this.vidQuery.muted = false; 
          this.volume = 0.5; 
          this.vidQuery.volume = 0.5;
          document.getElementById("useVol").style.opacity = "1.0";
          }
        }

        document.getElementById("useTime").onclick = () => {
          if(this.useTime == true){
            this.useTime = false;
            this.playRate = 1;
            this.vidQuery.playbackRate = 1;
            document.getElementById("useTime").style.opacity = "0.3";
          }
          else {
            this.useRate = false;
            this.useTime = true;
            this.playRate = 0;
            this.vidQuery.playbackRate = 0;
            document.getElementById("useRate").style.opacity = "0.3";
            document.getElementById("useTime").style.opacity = "1.0";
          }
        }

        this.timeSlider.addEventListener("change", () => {
          // Calculate the new time
          var time = this.vidQuery.duration * (this.timeSlider.value / 1000);
        
          // Update the video time
          this.vidQuery.currentTime = time;
        });

        this.timeSlider.onmousedown = () => {
          this.sliderfocus = true;
        }

        this.timeSlider.ontouchstart = () => {
          this.sliderfocus = true;
        }

        this.timeSlider.onchange = () => {
          this.sliderfocus = false;
        }

        document.getElementById("minus1min").onclick = () => {
          this.vidQuery.currentTime -= 60;
        }
        document.getElementById("plus1min").onclick = () => {
          this.vidQuery.currentTime += 60;
        }
        document.getElementById("minus10sec").onclick = () => {
          this.vidQuery.currentTime -= 10;
        }
        document.getElementById("plus10sec").onclick = () => {
          this.vidQuery.currentTime += 10;
        }

        document.getElementById("showhide").onclick = () => {
          if(this.hidden == false) {
            this.hidden = true;
            document.getElementById("showhide").innerHTML = "Show UI";
            document.getElementById("vidbuttons").style.display = "none";
            document.getElementById("timeDiv").style.display = "none";
            document.getElementById("fs").style.display = "none";
          }
          else{
            this.hidden = false;
            document.getElementById("showhide").innerHTML = "Hide UI";
            document.getElementById("vidbuttons").style.display = "";
            document.getElementById("timeDiv").style.display = "";
            document.getElementById("fs").style.display = "";
          }
        }
      }

      deInit(){
        document.getElementById("startbutton").removeEventListener('click', this.startVideo);
        document.getElementById("stopbutton").removeEventListener('click', this.stopVideo);
        cancelAnimationFrame(this.animationId);
      }

      addUI(parentId){
       var videoapiHTML = '<div id="'+this.vidapiId+'"> \
       <button class="showhide" id="showhide" name="showhide">Hide UI</button> \
       <div id="timeDiv" class="timeDiv"><input id="timeSlider" class="slider timeSlider" type="range" min="0" max="1000" value="0"><br><br> \
       <div id="vidbar" class="vidbar"><button id="minus1min" name="minus1min">--</button><button id="minus10sec" name="minus10sec">-</button><button id="play" name="play">||</button><button id="plus10sec" name="plus10sec">+</button><button id="plus1min" name="plus1min">++</button></div></div> \
       <div id="vidbuttons" class="vidbuttons"><table class="vidtable"> \
          <tr><td>Feedback:</td></tr> \
          <tr><td><button class="button vdfade" id="useAlpha" name="useAlpha">Fade</button></td></tr> \
          <tr><td><button class="button vdspeed" id="useRate" name="useRate">Speed</button></td></tr> \
          <tr><td><button class="button vdvol" id="useVol" name="useVol">Volume</button></td></tr> \
          <tr><td><button class="button" id="useTime" name="useTime">Time</button></td></tr> \
          </table></div> \
        </div>';
       HEGwebAPI.appendFragment(videoapiHTML, parentId);

       document.getElementById("useTime").style.opacity = "0.3";
       this.timeSlider = document.getElementById("timeSlider");

       this.localFileVideoPlayer();
      }

     localFileVideoPlayer() {
      'use strict'
      var URL = window.URL || window.webkitURL;
      var displayMessage = function (message, isError) {
        var element = document.querySelector('#message');
        element.innerHTML = message;
        element.className = isError ? 'error' : 'info';
      }
      var playSelectedFile = function (event) {
        var file = this.files[0];
        var type = file.type;
        var videoNode = document.querySelector('video');
        var canPlay = videoNode.canPlayType(type);
        if (canPlay === ''){ canPlay = 'no';}
        var message = 'Can play type "' + type + '": ' + canPlay;
        var isError = canPlay === 'no';
        displayMessage(message, isError)
        if (isError) {
          return;
        }
        var fileURL = URL.createObjectURL(file);
        videoNode.src = fileURL;
      }
      var inputNode = document.querySelector('input[name="fs"]');
      inputNode.addEventListener('change', playSelectedFile, false);
    }

    onData(score){
      if(this.useAlpha == true) {
        if(((this.alpha < 0.8) || (score > 0)) && ((this.alpha > 0)||(score < 0))){
          if(this.alpha - score < 0){
            this.alpha = 0;
          }
          else if(this.alpha - score > 0.8){
            this.alpha = 0.8;
          }
          else{
            this.alpha -= score;
          }
        }
      }
      if(this.useRate == true){
        if(((this.vidQuery.playbackRate < 3) || (score < 0)) && ((this.vidQuery.playbackRate > 0) || (score > 0)))
        { 
          this.playRate = this.vidQuery.playbackRate + score*0.5;
          if((this.playRate < 0.05) && (this.playRate > 0)){
            this.vidQuery.playbackRate = 0;
          }
          else if(this.playRate < 0) {
            this.vidQuery.currentTime += score;
          }
          else if((this.playRate > 0.05) && (this.playRate < 0.1)){
            this.vidQuery.playbackRate = 0.1;
          }
          else{
            this.vidQuery.playbackRate = this.playRate;
          }
        }
      }
      if(this.useVol == true){
        if(((this.vidQuery.volume < 1) || (score < 0)) && ((this.vidQuery.volume > 0) || (score > 0)))
        {
          this.volume = this.vidQuery.volume + score*0.5;
          if(this.volume < 0){
            this.vidQuery.volume = 0;
          }
          else if(this.volume > 1){
            this.vidQuery.volume = 1;
          }
          else {
            this.vidQuery.volume = this.volume;
          }
        }
      }
      if(this.useTime == true){
        this.vidQuery.currentTime += score*10;
      }
    }
    
    animateRect = () => {
      if((this.defaultUI == true) && (this.sliderfocus == false)) {
        this.timeSlider.value = Math.floor(1000 * this.vidQuery.currentTime / this.vidQuery.duration);
      }
        this.gl.clearColor(0,0,0.1,this.alpha);
        this.gl.clear(this.gl.COLOR_BUFFER_BIT);
        setTimeout(()=>{this.animationId = requestAnimationFrame(this.animateRect);},15); 
    }

    init(defaultUI) {
       this.vidQuery = document.getElementById(this.vidContainerId+'vid');
       if(this.useVol == true){
        this.vidQuery.muted = false;
        this.vidQuery.volume = 0.5;
        this.volume = 0.5;
       } 
       this.c = document.getElementById(this.vidContainerId+'canvas');
       this.c.width = this.vidQuery.width;
       this.c.height = this.vidQuery.height;
       var rect = this.vidQuery.getBoundingClientRect();
       this.c.style.top = rect.top + 'px';
       this.c.style.height = (rect.bottom - rect.top) + 'px';
       this.gl = this.c.getContext("webgl");
       this.gl.clearColor(0,0,0.1,0);
       this.gl.clear(this.gl.COLOR_BUFFER_BIT);

      if(defaultUI==true){
       this.setupButtons();
      }

      this.animateRect();
     }
 }
 
class audioJS { //Heavily modified from: https://codepen.io/jackfuchs/pen/yOqzEW
  constructor(res=[window.innerWidth,"800"], parentId="main_body", audioId="audio", audmenuId="audmenu", defaultUI=true) {
    this.audioId = audioId;
    this.audmenuId = audmenuId;
    
    var visualizerHTML = '<div id="'+this.audioId+'" class="visualizerDiv" class-"canvasContainer"> \
    <canvas class="audiocanvas" id="'+this.audioId+'canvas" width="'+res[0]+'" height="'+res[1]+'"></canvas> \
      </div> \
    ';

    HEGwebAPI.appendFragment(visualizerHTML, parentId);

    this.c = document.getElementById(this.audioId+"canvas");
    this.ctx = this.c.getContext("2d");
    this.gradient = this.ctx.createLinearGradient(0, 0, 0, this.c.height);
    this.gradient.addColorStop(1, 'springgreen');
    this.gradient.addColorStop(0.75, 'yellow');
    this.gradient.addColorStop(0, 'red');

    this.defaultUI = defaultUI;
    this.hidden = false;
    if(defaultUI==true) {
      this.initUI(parentId);
    }

    this.maxVol = 0.5;
    this.file = null; //the current file
    this.fileName = null; //the current file name

    this.audio = null;

    this.info = document.getElementById('fileinfo').innerHTML; //this used to upgrade the UI information
    this.menu = document.getElementById(this.audmenuId);
    this.infoUpdateId = null; //to sotore the setTimeout ID and clear the interval
    this.animationId = null;
    this.status = 0; //flag for sound is playing 1 or stopped 0
    this.forceStop = false;
    this.allCapsReachBottom = false;

    this.useVol = true;

    this.meterWidth = 14; //relative width of the meters in the spectrum
    this.meterGap = 2; //relative gap between meters
    this.capHeight = 2; //relative cap height
    this.capStyle = '#fff';
    this.meterNum = 256; //count of the meters
    this.capYPositionArray = []; //store the vertical position of the caps for the previous frame

    this.relativeWidth = this.meterNum*(this.meterWidth+this.meterGap); //Width of the meter (px)
    
    this.mode = 2;

    this.init();
  }

  stopAudio(){
    //stop the previous sound if any
    if (this.animationId !== null) {
        cancelAnimationFrame(this.animationId);
    }
    if(this.audio !== null){
      if (this.audio.sourceList.length > 0) {
          this.audio.sourceList[0].stop(0);
      }
    }
  }

  createVisualizer(buffer){
      this.audio.finishedLoading([buffer]);

      this.audio.sourceList[0].start(0);
      this.audio.gain.gain.setValueAtTime(this.maxVol, this.audio.ctx.currentTime);
      this.status = 1;
      this.audio.sourceList[0].onended = () => {
          this.endAudio();
      };
      this.updateInfo('Playing ' + this.fileName, false);
      this.info = 'Playing ' + this.fileName;
      document.getElementById('fileWrapper').style.opacity = 0.2;
      this.draw();
  }

  initUI(parentId){
      var audiomenuHTML = '<div id="'+this.audmenuId+'"> \
        <table id="audtable" class="audtable">\
          <tr><td>Feedback: </td></tr> \
          <tr><td><button id="useVol" name="useVol" class="button">Volume</button></td></tr> \
          <tr><td><button id="modebutton" name="modebutton" class="button">Mode</button></td></tr> \
        </table>\
        <input type="range" class="slider volSlider" id="volSlider" name="volSlider" min="0" max="100" value="'+toString(this.maxVol*100)+'"> \
        <div id="fileWrapper" class="file_wrapper"> \
          <div id="fileinfo"></div> \
          <input type="file" id="uploadedFile"></input> \
        </div> \
        </div> \
        <button id="showhide" name="showhide" class="showhide">Hide UI</button> \
      ';
      
      HEGwebAPI.appendFragment(audiomenuHTML, parentId);

      document.getElementById("useVol").onclick = () => {
        if(this.useVol == false) {
          this.useVol = true;
          document.getElementById("useVol").style.opacity = "1.0";
        }
        else{
          this.useVol = false;
          this.maxVol = document.getElementById("volSlider").value * 0.01;
          if(this.audio.gain != null) {
            this.audio.gain.gain.setValueAtTime(this.maxVol, this.audio.ctx.currentTime);
          }
          document.getElementById("useVol").style.opacity = "0.3";
        }
      }

      document.getElementById("volSlider").oninput = () => {
        this.maxVol = document.getElementById("volSlider").value * 0.01;
        if(this.audio.gain != null) {
          this.audio.gain.gain.setValueAtTime(this.maxVol, this.audio.ctx.currentTime);
        }
      }

      document.getElementById("modebutton").onclick = () => {
        if(this.mode == 0) { this.mode = 1;}
        else if (this.mode == 1){this.mode = 2;}
        else{ this.mode = 0; }
      }

      document.getElementById("showhide").onclick = () => {
        if(this.hidden == false) {
          this.hidden = true;
          document.getElementById("showhide").innerHTML = "Show UI";
          document.getElementById(this.audmenuId).style.display = "none";
        }
        else{
          this.hidden = false;
          document.getElementById("showhide").innerHTML = "Hide UI";
          document.getElementById(this.audmenuId).style.display = "";
        }
      }
  }

  decodeAudio(){
      //read and decode the file into audio array buffer 
      var file = this.file;
      var fr = new FileReader();
      fr.onload = (e) => {
          var fileResult = e.target.result;
          if (this.audio.ctx === null) {
              return;
          };
          this.updateInfo('Decoding the audio', true);
          this.audio.ctx.decodeAudioData(fileResult, (buffer) => {
            this.updateInfo('Decode successful, starting the visualizer', true);
            this.createVisualizer(buffer);
          }, (e) => {
            this.updateInfo('Failed to decode the file!', false);
            console.log(e);
          });
      };
      fr.onerror = function(e) {
        this.updateInfo('Failed to read the file!', false);
        console.log(e);
      };
      //assign the file to the reader
      this.updateInfo('Starting read the file', true);
      fr.readAsArrayBuffer(file);
  }

  onData(score){
    if(this.useVol == true) {
      var newVol = this.audio.gain.gain.value + score;
      if(newVol > this.maxVol){
        newVol = this.maxVol;
      }
      if(newVol < 0){
        newVol = 0;
      }
      if(this.defaultUI == true) {
        document.getElementById("volSlider").value = newVol * 100;
      }
      this.audio.gain.gain.value = newVol;
    }
  }

  endAudio(){
    if (this.forceStop) {
      this.forceStop = false;
      this.status = 1;
      return;
    };
    this.status = 0;
    var text = 'Song ended...';
    document.getElementById('fileWrapper').style.opacity = 1;
    document.getElementById('fileinfo').innerHTML = text;
    this.info = text;
    document.getElementById('uploadedFile').value = '';
  }

  updateInfo(text, processing) {
    var infoBar = document.getElementById('fileinfo'),
    dots = '...',
    i = 0;
    infoBar.innerHTML = text + dots.substring(0, i++);
    if (this.infoUpdateId !== null) {
        clearTimeout(this.infoUpdateId);
    };
    if (processing) {
        //animate dots at the end of the info text
        var animateDot = () => {
            if (i > 3) {
                i = 0
            };
            infoBar.innerHTML = text + dots.substring(0, i++);
            this.infoUpdateId = setTimeout(animateDot, 250);
        }
        this.infoUpdateId = setTimeout(animateDot, 250);
    };
  }

  init(){

      var audioInput = document.getElementById('uploadedFile');
      var dropContainer = document.getElementById(this.audioId+"canvas");
      //listen the file upload
      audioInput.onchange = () => {
        this.audio = new SoundJS();
        if (this.audio.ctx===null) {return;};
        
        //the if statement fixes the file selection cancel, because the onchange will trigger even if the file selection has been cancelled
        if (audioInput.files.length !== 0) {
            //only process the first file
            this.file = audioInput.files[0];
            this.fileName = this.file.name;
            if (this.status === 1) {
                //the sound is still playing but we uploaded another file, so set the forceStop flag to true
                this.forceStop = true;
            };
            document.getElementById('fileWrapper').style.opacity = 1;
            this.updateInfo('Uploading', true);
            //once the file is ready, start the visualizer
            this.decodeAudio();
        };
      };
      //listen the drag & drop
      dropContainer.addEventListener("dragenter", () => {
          document.getElementById('fileWrapper').style.opacity = 1;
          this.updateInfo('Drop it on the page', true);
      }, false);
      dropContainer.addEventListener("dragover", function(e) {
          e.stopPropagation();
          e.preventDefault();
          //set the drop mode
          e.dataTransfer.dropEffect = 'copy';
      }, false);
      dropContainer.addEventListener("dragleave", () => {
          document.getElementById('fileWrapper').style.opacity = 0.2;
          this.updateInfo(this.info, false);
      }, false);
      dropContainer.addEventListener("drop", (e) => {
          e.stopPropagation();
          e.preventDefault();
          if (this.audio.ctx===null) {return;};
          document.getElementById('fileWrapper').style.opacity = 1;
          this.updateInfo('Uploading', true);
          //get the dropped file
          this.file = e.dataTransfer.files[0];
          if (this.status === 1) {
              document.getElementById('fileWrapper').style.opacity = 1;
              this.forceStop = true;
          };
          this.fileName = this.file.name;
          //once the file is ready, start the visualizer
          this.decodeAudio();
      }, false);
    }

    drawMeter = () => {
      var cwidth = this.c.width;
      var cheight = this.c.height;

      var meterWidthScale = cwidth/this.relativeWidth;

      var array = new Uint8Array(this.audio.analyser.frequencyBinCount);
      this.audio.analyser.getByteFrequencyData(array);
      if (this.status === 0) {
          //fix when some sounds stop and the value is still not back to zero
          for (var i = array.length - 1; i >= 0; i--) {
              array[i] = 0;
          };
          this.allCapsReachBottom = true;
          for (var i = this.capYPositionArray.length - 1; i >= 0; i--) {
            this.allCapsReachBottom = this.allCapsReachBottom && (this.capYPositionArray[i] === 0);
          };
          if (this.allCapsReachBottom) {
              cancelAnimationFrame(this.animationId); //since the sound is stopped and animation finished, stop the requestAnimation to prevent potential memory leak.
              return;
          };
      };
      var step = Math.round((array.length*0.75) / this.meterNum); //sample limited data from the total array
      this.ctx.clearRect(0, 0, cwidth, cheight);
      for (var i = 0; i < this.meterNum; i++) {
          var value = array[i * step];
          if (this.capYPositionArray.length < this.meterNum) {
            this.capYPositionArray.push(value);
          };
          this.capYPositionArray[i] = this.capYPositionArray[i] - 0.5;
          this.ctx.fillStyle = this.capStyle;
          //draw the cap, with transition effect
          var xoffset = (this.meterWidth*meterWidthScale + this.meterGap*meterWidthScale);
          if (value < this.capYPositionArray[i]) {
            this.ctx.fillRect(i * xoffset, cheight - this.capYPositionArray[i], this.meterWidth*meterWidthScale, this.capHeight);
          } else {
            this.ctx.fillRect(i * xoffset, cheight - value, this.meterWidth*meterWidthScale, this.capHeight);
            this.capYPositionArray[i] = value;
          };
          this.ctx.fillStyle = this.gradient; //set the fillStyle to gradient for a better look
          this.ctx.fillRect(i * xoffset /*meterWidth+gap*/ , cheight - value + this.capHeight, this.meterWidth*meterWidthScale, cheight); //the meter
      }
    }

    drawLine = () => {
      var cwidth = this.c.width;
      var cheight = this.c.height;

      var meterWidthScale = cwidth/this.relativeWidth;

      var array = new Uint8Array(this.audio.analyser.frequencyBinCount);
      this.audio.analyser.getByteFrequencyData(array);
      if (this.status === 0) {
          //fix when some sounds stop and the value is still not back to zero
          for (var i = array.length - 1; i >= 0; i--) {
              array[i] = 0;
          };
          this.allCapsReachBottom = true;
          for (var i = this.capYPositionArray.length - 1; i >= 0; i--) {
            this.allCapsReachBottom = this.allCapsReachBottom && (this.capYPositionArray[i] === 0);
          };
          if (this.allCapsReachBottom) {
              cancelAnimationFrame(this.animationId); //since the sound is stopped and animation finished, stop the requestAnimation to prevent potential memory leak.
              return;
          };
      };
      var step = Math.round((array.length*0.75) / this.meterNum); //sample limited data from the total array

      this.ctx.clearRect(0, 0, cwidth, cheight);
      this.ctx.beginPath();
      this.ctx.moveTo(0, cheight - this.capYPositionArray[0]*2);
      for (var i = 0; i < this.meterNum; i++) {
        var value = array[i * step];
        this.capYPositionArray[i]=value;
        var xoffset = (this.meterWidth + this.meterGap)*meterWidthScale;
        this.ctx.lineTo(i*xoffset,cheight - this.capYPositionArray[i]*2);
      }
      this.ctx.strokeStyle = 'red';
      this.ctx.stroke();
    }

    drawCircle = () => { //Based on: https://www.kkhaydarov.com/audio-visualizer/
      
      // find the center of the window
      var center_x = this.c.width * 0.5;
      var center_y = this.c.height * 0.5;
      var radius = 150;
       
      var array = new Uint8Array(this.audio.analyser.frequencyBinCount);
      this.audio.analyser.getByteFrequencyData(array);

      // style the background
      var gradient = this.ctx.createRadialGradient(center_x,center_y,2,center_x,center_y,600+array[100]);
      gradient.addColorStop(0,"blue");
      gradient.addColorStop(0.25,"purple");
      gradient.addColorStop(1,"rgba(255,69,0,0)");
      this.ctx.fillStyle = gradient;
      this.ctx.fillRect(0,0,this.c.width,this.c.height);

      var gradient2 = this.ctx.createRadialGradient(center_x*0.2,center_y*1.7,1,center_x*0.2,center_y*1.7,2+array[2]*0.5);
      gradient2.addColorStop(0,"yellow");
      gradient2.addColorStop(0.2,"greenyellow");
      gradient2.addColorStop(1,"rgba(255,69,0,0)");
      this.ctx.fillStyle = gradient2;
      this.ctx.fillRect(0,0,this.c.width,this.c.height);

      var gradient3 = this.ctx.createRadialGradient(center_x*0.2,center_y*0.3,1,center_x*0.2,center_y*0.3,2+array[200]*0.5);
      gradient3.addColorStop(0,"red");
      gradient3.addColorStop(0.2,"crimson");
      gradient3.addColorStop(1,"rgba(255,69,0,0)");
      this.ctx.fillStyle = gradient3;
      this.ctx.fillRect(0,0,this.c.width,this.c.height);

      var gradient4 = this.ctx.createRadialGradient(center_x*1.8,center_y*0.3,1,center_x*1.8,center_y*0.3,2+array[100]*0.5);
      gradient4.addColorStop(0,"hotpink");
      gradient4.addColorStop(0.2,"magenta");
      gradient4.addColorStop(1,"rgba(255,69,0,0)");
      this.ctx.fillStyle = gradient4;
      this.ctx.fillRect(0,0,this.c.width,this.c.height);

      var gradient5 = this.ctx.createRadialGradient(center_x*1.8,center_y*1.7,1,center_x*1.8,center_y*1.7,2+array[50]*0.5);
      gradient5.addColorStop(0,"deepskyblue");
      gradient5.addColorStop(0.2,"skyblue");
      gradient5.addColorStop(1,"rgba(255,69,0,0)");
      this.ctx.fillStyle = gradient5;
      this.ctx.fillRect(0,0,this.c.width,this.c.height);

      /*
      //draw a circle
      this.ctx.beginPath();
      this.ctx.arc(center_x,center_y,radius,0,2*Math.PI);
      this.ctx.stroke();
      */
      for(var i = 0; i < this.meterNum; i++){
          
          //divide a circle into equal parts
          var rads = Math.PI * 2 / this.meterNum;
          
          var bar_height = array[i];
          
          // set coordinates
          var x = center_x + Math.cos(rads * i) * (radius);
          var y = center_y + Math.sin(rads * i) * (radius);
          var x_end = center_x + Math.cos(rads * i)*(radius + bar_height);
          var y_end = center_y + Math.sin(rads * i)*(radius + bar_height);
          
          //draw a bar
          var lineColor = "rgb(" + array[i] + ", " + array[i] + ", " + 205 + ")";
    
          this.ctx.strokeStyle = lineColor;
          this.ctx.lineWidth = this.capHeight;
          this.ctx.beginPath();
          this.ctx.moveTo(x,y);
          this.ctx.lineTo(x_end,y_end);
          this.ctx.stroke();
      
      }
    } 
 
    draw = () => {
      this.c.width=window.innerWidth;
      if(this.mode == 0){
        this.drawMeter(); 
      }
      else if(this.mode == 1){
        this.drawLine();
      }
      else if(this.mode == 2){
        this.drawCircle();
      }
      setTimeout(()=>{this.animationId = requestAnimationFrame(this.draw)},15);
    }
 }

class hillJS {
  constructor(res=["1400","500"], updateInterval=2000, parentId="main_body", canvasId="hillscanvas", defaultUI=true, canvasmenuId="hillsmenu") {
   this.canvasId = canvasId;
   this.canvasmenuId = canvasmenuId;

   var canvasHTML = '<div id="canvasContainer" class="canvasContainer"> \
      <canvas class="hillcss" id="'+this.canvasId+'" width="'+res[0]+'" height="'+res[1]+'"></canvas> \
      ';

   HEGwebAPI.appendFragment(canvasHTML,parentId);

   this.defaultUI = defaultUI;
   
   if(defaultUI == true){
    this.initUI(parentId);
    this.menu = document.getElementById(this.canvasmenuId);
   }
   
   this.c = document.getElementById(this.canvasId);
   this.ctx = this.c.getContext("2d");
   this.hidden = false;

   this.soundFX = null;
    
   this.gradient = this.ctx.createLinearGradient(0, 0, 0, this.c.height);
   this.gradient.addColorStop(1, 'dodgerblue');
   this.gradient.addColorStop(0.9, 'green');
   this.gradient.addColorStop(0.8, 'springgreen');
   this.gradient.addColorStop(0.65, 'sandybrown')
   this.gradient.addColorStop(0.45, 'slategray')
   this.gradient.addColorStop(0.35, 'silver')
   this.gradient.addColorStop(0.2, 'snow');
   this.gradient.addColorStop(0.1, 'white');
   this.gradient.addColorStop(0.00, 'gold');
   
   this.mode = 1;

   this.updateInterval = updateInterval;
   this.allCapsReachBottom = false;
   this.meterWidth = 12;
   this.meterGap = 2;
   this.hillNum = 150; //count of the meters
   this.capHeight = 2;
   this.capStyle = '#fff';

   this.relativeWidth = this.hillNum*(this.meterWidth+this.meterGap);

   this.updateInterval = updateInterval; //ms between update
   this.hillScore = [...Array(this.hillNum).fill(50)]; //
   this.animationId = null;
   this.draw();

  }

  cancelDraw() {
    if(this.animationId != null){
      cancelAnimationFrame(this.animationId);
      this.animationId = null;
    }
  }

  initUI(parentId){
    var menuHTML = '<div id="'+this.canvasmenuId+'" class="hillapi"> \
    <table class="hilltable"> \
    <tr><td><button class="button" id="hillsRbutton">Reset</button></td></tr> \
    <tr><td><button class="button" id="hillsModebutton">Mode</button></td></tr> \
    <tr><td><button class="button" id="hillsAudbutton">Audio</button></td></tr> \
    <tr><td><input type="number" id="speed" name="speed" placeholder="Update (Sec)"></input></td></tr> \
    <tr><td><button id="hillsSpeedbutton">Set Speed</button></tr></td> \
    </table> \
    </div> \
    <button id="showhide" name="showhide" class="showhide">Hide UI</button>';
      
    HEGwebAPI.appendFragment(menuHTML,parentId);

    document.getElementById("hillsAudbutton").style.opacity = 0.3;

    document.getElementById("hillsRbutton").onclick = () => {
      this.hillScore = [...Array(this.hillNum).fill(50)];
    }
    document.getElementById("hillsModebutton").onclick = () => {
      if(this.mode == 0) { this.mode = 1;}
      else{this.mode = 0;}
    }
    document.getElementById("hillsAudbutton").onclick = () => {
      if(this.soundFX == null){
        this.soundFX = new SoundJS(); //Init on gesture
        document.getElementById("hillsAudbutton").style.opacity = 1.0;
      }
      else{
        if(this.soundFX.gain.gain.value == 0){
          this.soundFX.gain.gain.value = 1;
          document.getElementById("hillsAudbutton").style.opacity = 1.0;
        }
        else {
          this.soundFX.gain.gain.value = 0;
          document.getElementById("hillsAudbutton").style.opacity = 0.3;
        }
      }
    }
    document.getElementById("hillsSpeedbutton").onclick = () => {
      this.updateInterval = document.getElementById("speed").value*1000;
    }

    document.getElementById("startbutton").addEventListener('click',  this.draw, false);
    document.getElementById("stopbutton").addEventListener('click',  this.cancelDraw, false);

    document.getElementById("showhide").onclick = () => {
      if(this.hidden == false) {
        this.hidden = true;
        document.getElementById("showhide").innerHTML = "Show UI";
        document.getElementById(this.canvasmenuId).style.display = "none";
      }
      else{
        this.hidden = false;
        document.getElementById("showhide").innerHTML = "Hide UI";
        document.getElementById(this.canvasmenuId).style.display = "";
      }
    }
  }

  deInit(){
    document.getElementById("startbutton").removeEventListener('click', this.draw);
    document.getElementById("stopbutton").removeEventListener('click', this.cancelDraw);
    if(this.soundFX != null){
      if(this.soundFX.osc[0] != undefined) {
        this.soundFX.osc[0].stop(0);
      }
    }
  }

  onData(score){
    var newscore = this.hillScore[this.hillScore.length - 1]+score*20
    //if(newscore > this.c.height){
      this.hillScore[this.hillScore.length - 1] = newscore;
      if(this.hillScore[this.hillScore.length - 1] < 10) { // minimum score (prevents rendering outside viewport)
        this.hillScore[this.hillScore.length - 1] = 10;
      }
      if(score > 0) {
        this.hillScore[this.hillScore.length - 1] += 0.5;
      }
      if(score < 0) {
        this.hillScore[this.hillScore.length - 1] -= 0.3;
      }
    //}
    //else {
    //  this.hillScore[this.hillScore.length - 1] = this.c.height;
    //}
  }

  draw = () => {
    // Get data interval
    // Create background and bars
    // Change height of bars based on avg or rms. (all at 0 on fresh session)
    // Update last bar for every t time interval based on change
    if(this.soundFX != null){
      if(this.hillScore[this.hillScore.length - 1] > this.hillScore[this.hillScore.length - 2]) {
        this.soundFX.playFreq([650+this.hillScore[this.hillScore.length - 1]], 0.05);
      }
      else if(this.hillScore[this.hillScore.length - 1] < this.hillScore[this.hillScore.length - 2]){
        this.soundFX.playFreq([250+this.hillScore[this.hillScore.length - 1]], 0.05);
      }
    }

    var cwidth = this.c.width;
    var cheight = this.c.height;
    var capYPositionArray = [];
    
    var wscale = cwidth / this.relativeWidth;
    var xoffset = (this.meterWidth+this.meterGap)*wscale;
    var hscale = 1; //Height scalar
    if(this.hillScore[this.hillScore.length-1] > cheight) {hscale = cheight / this.hillScore[this.hillScore.length-1];}
    this.ctx.clearRect(0, 0, cwidth, cheight);
    if(this.mode == 0){ // bars
      for (var i = 0; i < this.hillNum; i++) {
          var value = this.hillScore[i]*hscale;
          if(value < 0){ value = 0;}
          if (capYPositionArray.length < Math.round(this.hillNum)) {
              capYPositionArray.push(value);
          }
          this.ctx.fillStyle = this.capStyle;
          //draw the cap, with transition effect
          if (value < capYPositionArray[i]) {
              this.ctx.fillRect(i * xoffset, (cheight - (--capYPositionArray[i])), this.meterWidth*wscale, this.capHeight);
          } else {
              this.ctx.fillRect(i * xoffset, (cheight - value), this.meterWidth*wscale, this.capHeight);
              capYPositionArray[i] = value;
          }
          this.ctx.fillStyle = this.gradient; 
          this.ctx.fillRect(i * xoffset /*meterWidth+gap*/ , (cheight - value + this.capHeight), this.meterWidth*wscale, cheight);
      }
    }
    if(this.mode == 1){ //gradient
      this.ctx.fillStyle = this.gradient;
      this.ctx.beginPath();
      this.ctx.moveTo(0,cheight - this.hillScore[0])
      for (var i = 0; i < this.hillNum; i++) {
        var value = this.hillScore[i]*hscale;
        if(value < 0){ value = 0; }
        this.ctx.lineTo(i*xoffset, (cheight - value))
        if (i == this.hillNum - 1){      
          this.ctx.lineTo(cwidth,(cheight - value));
        }
      }
      this.ctx.lineTo(cwidth,cheight);
      this.ctx.lineTo(0,cheight);
      this.ctx.closePath()
      this.ctx.fill();
    }
    this.hillScore.shift();
    this.hillScore.push(this.hillScore[this.hillScore.length - 1]);
    setTimeout(() => {this.animationId = requestAnimationFrame(this.draw)}, this.updateInterval);
  }
 }

class textReaderJS {
  constructor(text="this is a test", res=["800","400"], parentId="main_body", canvasId="textcanvas", defaultUI=true, canvasmenuId="textcanvasmenu") {
    this.text = text;
    this.canvasId = canvasId;
    this.canvasmenuId = canvasmenuId;
    this.parentId = parentId;

    var textReaderHTML = "<div id='"+this.canvasId+"container' class='canvasContainer'><canvas id='"+this.canvasId+"' class='textreadercss' width='"+res[0]+"' height='"+res[1]+"'></canvas></div>"

    HEGwebAPI.appendFragment(textReaderHTML, parentId);

    this.defaultUI = defaultUI;
    this.hidden = false;
    if(defaultUI == true) {
      this.text = 'Leap clear of all that is corporeal, and make yourself grown to a like expanse with that greatness which is beyond all measure... rise above all time and become eternal... then you will apprehend God. \
      Think that for you too nothing is impossible; deem that you too are immortal, and that you are able to grasp all things in your thought, to know every craft and science; find your home in the haunts of every living creature; \
      make yourself higher than all heights and lower than all depths; bring together in yourself all opposites of quality, heat and cold, dryness and fluidity; \
      think that you are everywhere at once, on land, at sea, in heaven; think that you are not yet begotten, that you are in the womb, that you are young, that you are old, that you have died, that you are in the world beyond the grave; \
      grasp in your thought all of this at once, all times and places, all substances and qualities and magnitudes together; then you can apprehend God. \
      But if you shut up your soul in your body, and abase yourself, and say “I know nothing, I can do nothing; I am afraid of earth and sea, I cannot mount to heaven; I know not what I was, nor what I shall be,” then what have you to do with God?';

      this.initUI();
    }

    this.c = document.getElementById(this.canvasId);
    this.ctx = this.c.getContext("2d");
    this.pxf = 0.5; //Pixels per frame;
    this.lastpxf = this.pxf; //Store last pxf when paused or whatever
    this.textXPos = 0;
    this.maxXPos = window.innerWidth;
    this.animationId;

    this.draw();
  }

  startpxf = () => {
    this.pxf = this.lastpxf; 
  }

  stoppxf = () => {
    this.lastpxf = this.pxf; this.pxf = 0;
  }

  initUI() {
    var uiHTML = "<div id='"+this.canvasmenuId+"' class='textmenu'> \
    <textarea id='"+this.canvasId+"Textarea'>Breathe in, Breathe out, Breathe in, Breathe out...</textarea><br> \
    <button id='"+this.canvasId+"submittext' class='button'>Submit</button> \
    </div><button id='showhide' name='showhide' class='showhide'>Hide UI</button>";

    HEGwebAPI.appendFragment(uiHTML, this.parentId);

    document.getElementById(this.canvasId+'submittext').onclick = () => {
      this.text = document.getElementById(this.textId+'Textarea').value;
      this.textXPos = 0;
    }

    document.getElementById("showhide").onclick = () => {
      if(this.hidden == false) {
        this.hidden = true;
        document.getElementById("showhide").innerHTML = "Show UI";
        document.getElementById(this.canvasId+'menu').style.display = "none";
      }
      else{
        this.hidden = false;
        document.getElementById("showhide").innerHTML = "Hide UI";
        document.getElementById(this.canvasId+'menu').style.display = "";
      }
    }

    document.getElementById("startbutton").addEventListener('click', this.startpxf, false);
    document.getElementById("stopbutton").addEventListener('click', this.stoppxf, false);

  }

  deInit(){
    document.getElementById("startbutton").removeEventListener('click', this.startpxf);
    document.getElementById("stopbutton").removeEventListener('click', this.stoppxf);
  }

  onData(score) {
    this.pxf += score;
  }

  draw = () => {
    this.maxXPos = this.c.width;

    this.textXPos += this.pxf;

    //draw this.text at correct position, in middle of canvas;
    this.ctx.clearRect(0, 0, this.c.width, this.c.height);

    this.ctx.font = "2em Arial";
    this.ctx.fillStyle = "#ffffff";
    this.ctx.fillText(this.text, this.maxXPos - this.textXPos, this.c.height*0.5);
    setTimeout(()=>{this.animationId = requestAnimationFrame(this.draw);},15); 
  }
 }


 class boidsJS { //Birdoids Swarm AI. https://en.wikipedia.org/wiki/Boids 
    constructor(boidsCount = 200, res=[window.innerWidth,"440"], parentId="main_body", canvasId="boidscanvas", defaultUI=true, canvasmenuId="boidscanvasmenu") {

      this.parentId = parentId;
      this.res = res;
      this.canvasId = canvasId;
      this.defaultUI = defaultUI;
      this.canvasMenuId = canvasmenuId;
      this.animationId = null;

      this.boidsCount = boidsCount;
      this.boidsPos = []; //vec3 list
      this.boidsVel = [];
      
      this.groupingSize = 10; //Max # that a boid will reference.
      this.groupingRadius = 10000; //Max radius for a boid to check for flocking

      this.boidsMul = 1; // Global modifier on boids velocity change for particles. 

      this.dragMul = 0.1;
      this.cohesionMul = 0.01; //Force toward mean position of group
      this.alignmentMul = 0.5; //Force perpendicular to mean direction of group
      this.separationMul = 3; //Force away from other boids group members, multiplied by closeness.
      this.swirlMul = 0.0005; //Positive = Clockwise rotation about an anchor point
      this.attractorMul = 0.003;

      this.useAttractor = true;
      this.useSwirl = true;
      
      this.attractorAnchor = [0.5,0.5,0];
      this.swirlAnchor = [0.5,0.5,0]; //Swirl anchor point
      this.boundingBox; //Bounds boids to 3D box, good for shaping swirls

      //Could add: leaders (negate cohesion and alignment), predators (negate separation), goals (some trig or averaging to bias velocity toward goal post)

      this.lastFrame = 0;
      this.thisFrame = 0;
      this.frameRate = 0;

      this.renderer = new Particles(false, this.boidsCount, this.res, this.parentId, this.canvasId, this.defaultUI, this.canvasmenuId); //Commandeer the particle renderer

      var waitForRenderer = () => { //wait for renderer to load all the particles before beginning the boids algo
        setTimeout(() => {
          if(this.renderer.particles.length == this.renderer.settings.maxParticles){
            this.swirlAnchor = [this.renderer.canvas.width*0.45, this.renderer.canvas.height*0.5, 0];
            this.attractorAnchor = this.swirlAnchor;

            for(var i = 0; i < this.boidsCount; i++){
              this.boidsPos.push([Math.random()*this.renderer.canvas.width,Math.random()*this.renderer.canvas.height,Math.random()]); //Random starting positions;
              this.boidsVel.push([Math.random()*0.01,Math.random()*0.01,Math.random()*0.01]); //Random starting velocities;
            }

            this.boidsPos.forEach((item,idx) => {
                this.renderer.particles[idx].x = item[0];
                this.renderer.particles[idx].y = item[1];
                //console.log(idx);
              });
              
            this.animationId = requestAnimationFrame(this.draw);
          }
          else{
            waitForRenderer();
          }
        },300);
      }
      waitForRenderer();
  }

  calcBoids() { //Run a boids calculation to update velocities
    //Simple recursive boids, does not scale up well without limiting group sizes
    var newVelocities = [];
    //console.time("boid")
    for(var i = 0; i < this.boidsCount; i++) {
      var inRange = []; //indices of in-range boids
      var distances = []; //Distances of in-range boids
      var cohesionVec = this.boidsPos[i]; //Mean position of all boids for cohesion multiplier
      var separationVec = [0,0,0]; //Sum of a-b vectors, weighted by 1/x to make closer boids push harder.
      var alignmentVec = this.boidsVel[i]; //Perpendicular vector from average of boids velocity vectors. Higher velocities have more alignment pull.
      var groupCount = 0;
      nested:
      for(var j = 0; j < this.boidsCount; j++) {

        var randj = Math.floor(Math.random()*this.boidsCount); // Get random index
        if(randj === i) { continue; }

        if(distances.length > this.groupingSize) { break nested; }
        var disttemp = this.distance3D(this.boidsPos[i],this.boidsPos[randj]);
        if(disttemp > this.groupingRadius) { continue; }
        distances.push(disttemp);
        inRange.push(randj);

        cohesionVec   = [cohesionVec[0] + this.boidsPos[randj][0], cohesionVec[1] + this.boidsPos[randj][1], cohesionVec[2] + this.boidsPos[randj][2]];
        
        separationVec = [separationVec[0] + (this.boidsPos[i][0]-this.boidsPos[randj][0])*(1/disttemp), separationVec[1] + this.boidsPos[i][1]-this.boidsPos[randj][1]*(1/disttemp), separationVec[2] + (this.boidsPos[i][2]-this.boidsPos[randj][2] + 1)*(1/disttemp)];
        if((separationVec[0] == Infinity) || (separationVec[0] == -Infinity) || (separationVec[0] > 3) || (separationVec[0] < -3) || (separationVec[1] == Infinity) || (separationVec[1] == -Infinity) || (separationVec[1] > 3) || (separationVec[1] < -3) || (separationVec[2] == Infinity) || (separationVec[2] == -Infinity) || (separationVec[2] > 3) || (separationVec[2] < -3) ) {
          separationVec = [Math.random()*4-2,Math.random()*4-2,Math.random()*4-2]; //Special case for when particles overlap and cause infinities
          //console.log("Infinity!")
        }
        //console.log(separationVec);
        alignmentVec  = [alignmentVec[0] + this.boidsVel[randj][0], alignmentVec[1] + this.boidsVel[randj][1], alignmentVec[2] + this.boidsVel[randj][2]];
        
        groupCount++;
          
      }
      cohesionVec = [this.cohesionMul*(cohesionVec[0]/groupCount -this.boidsPos[i][0] ),this.cohesionMul*(cohesionVec[1]/groupCount-this.boidsPos[i][1]),this.cohesionMul*(cohesionVec[2]/groupCount-this.boidsPos[i][2])];
      alignmentVec = [-(this.alignmentMul*alignmentVec[1]/groupCount),this.alignmentMul*alignmentVec[0]/groupCount,this.alignmentMul*alignmentVec[2]/groupCount];//Use a perpendicular vector [-y,x,z]
      separationVec = [this.separationMul*separationVec[0],this.separationMul*separationVec[1],this.separationMul*separationVec[2]];
      
      var swirlVec = [0,0,0];
      if(this.useSwirl == true){
        swirlVec = [-(this.boidsPos[i][1]-this.swirlAnchor[1])*this.swirlMul,(this.boidsPos[i][0]-this.swirlAnchor[0])*this.swirlMul,(this.boidsPos[i][2]-this.swirlAnchor[2])*this.swirlMul];
      }
      var attractorVec = [0,0,0]
      if(this.useAttractor == true){
        attractorVec = [(this.attractorAnchor[0]-this.boidsPos[i][0])*this.attractorMul,(this.attractorAnchor[1]-this.boidsPos[i][1])*this.attractorMul,(this.attractorAnchor[2]-this.boidsPos[i][2])*this.attractorMul]
      }
      
      //console.log(cohesionVec);
      //console.log(alignmentVec);
      //console.log(separationVec);
      //console.log(swirlVec);

      newVelocities.push([
        this.boidsVel[i][0]*this.dragMul+cohesionVec[0]+alignmentVec[0]+separationVec[0]+swirlVec[0]+attractorVec[0],
        this.boidsVel[i][1]*this.dragMul+cohesionVec[1]+alignmentVec[1]+separationVec[1]+swirlVec[1]+attractorVec[1],
        this.boidsVel[i][2]*this.dragMul+cohesionVec[2]+alignmentVec[2]+separationVec[2]+swirlVec[2]+attractorVec[1]
        ]);
    }
    if(newVelocities.length == this.boidsCount){ // If newVelocities updated completely, else there was likely an error
        //console.log(newVelocities);
        this.boidsVel = newVelocities; //Set new velocities. This will update positions in the draw function which keeps the frame timing
        //console.timeEnd("boid");
        return true;
      }
      else { return false; }
    
  }

  distance3D(a,b) //assumes you're passing two Array(3) i.e. [x,y,z]
  {
    return Math.sqrt(Math.pow(b[0]-a[0],2) + Math.pow(b[1]-a[1],2) + Math.pow(b[2]-a[2],2));
  }

  //returns a shuffled array 
  shuffleArr(arr) {
    var randArr = [];
    while(arr.length > 0) {
      var randIdx = Math.floor(Math.random()*arr.length);
      randArr.push(arr[randIdx]);
    }
    return randArr;
  }

  deInit() {
    cancelAnimationFrame(this.animationId);
  }

  onData(score){
    this.swirlMul += score*0.0003;
    if(this.swirlMul < 0) {
      this.swirlMul = 0;
    }
    else if(this.swirlMul > 0.001){
      this.swirlMul = 0.01;
    }
  }

  draw = () => {
    var success = this.calcBoids();
    if(success == true){
          //Moving anchor
      var anchorTick = performance.now()*0.00005;
      var newAnchor = [Math.sin(anchorTick)*Math.sin(anchorTick)*this.renderer.canvas.width*0.3+this.renderer.canvas.width*0.2, this.renderer.canvas.height*0.3, 0];
    
      this.swirlAnchor = newAnchor;
      this.attractorAnchor = newAnchor;

      this.lastFrame = this.thisFrame;
      this.thisFrame = performance.now();
      this.frameRate = (this.thisFrame - this.lastFrame) * 0.001; //Framerate in seconds
      this.boidsPos.forEach((item,idx) => {
        //this.boidsPos[idx] = [item[0]+(this.boidsVel[idx][0]*this.frameRate),item[1]+(this.boidsVel[idx][1]*this.frameRate),item[2]+(this.boidsVel[idx][2]*this.frameRate)];
        if(idx <= this.renderer.particles.length){
          this.renderer.particles[idx].vx += this.boidsVel[idx][0]*this.frameRate*this.boidsMul;
          this.renderer.particles[idx].vy += this.boidsVel[idx][1]*this.frameRate*this.boidsMul;
          //console.log(this.renderer.particles[idx].vx)
        }
        this.boidsPos[idx][0] = this.renderer.particles[idx].x;
        this.boidsPos[idx][1] = this.renderer.particles[idx].y;
        //console.log(this.renderer.particles[idx].x)
      });
    }

    //Now feed the position data into the visual as a list of vec3 data or update canvas

    setTimeout(()=>{this.animationId = requestAnimationFrame(this.draw)},20);
  }

}

  class Particles { //Adapted from this great tutorial: https://modernweb.com/creating-particles-in-html5-canvas/
    constructor(useDefaultAnim = true, maxParticles = 100, res=[window.innerWidth,"440"], parentId="main_body", canvasId="particlecanvas", defaultUI=true, canvasmenuId="particlecanvas") {

      this.canvasId = canvasId;
      this.parentId = parentId;
      this.defaultUI = defaultUI;
      this.canvasmenuId = canvasmenuId;

      var canvasHTML = '<div id="canvasContainer" class="canvasContainer"> \
      <canvas class="boidscss" id="'+this.canvasId+'" width="'+res[0]+'" height="'+res[1]+'"></canvas> \
      ';

      HEGwebAPI.appendFragment(canvasHTML, this.parentId);
     
      this.canvas = document.getElementById(this.canvasId);
      this.context = this.canvas.getContext("2d");

      this.animationId = null;
      this.lastFrame = 0;
      this.thisFrame = 0;
      this.frameRate = 1;

      this.useDefaultAnim = useDefaultAnim;

      // Inital starting position
      this.posX = 20;
      this.posY = this.canvas.height / 2;

      // No longer setting velocites as they will be random
      // Set up object to contain particles and set some default values
      this.particles = [];
      this.particleIndex = 0;
      this.settings = {
            maxParticles: maxParticles,
            particleSize: 5,
            startingX: this.canvas.width / 2, 
            startingY: this.canvas.height / 4,
            maxSpeed: 3, 
            xBounce: -1,
            yBounce: -1,
            gravity: 0.0,
            maxLife: Infinity,
            groundLevel: this.canvas.height * 0.999,
            leftWall: this.canvas.width * 0.001,
            rightWall: this.canvas.width * 0.999,
            ceilingWall: this.canvas.height * 0.001
          };

      //for default anim
      // To optimise the previous script, generate some pseudo-random angles
      this.seedsX = [];
      this.seedsY = [];
      this.currentAngle = 0;

      if(this.useDefaultAnim == true){
        this.seedAngles();     // Start off with 100 angles ready to go
      }

      this.animationId = requestAnimationFrame(this.draw);
      
    }

    seedAngles() {
      this.seedsX = [];
      this.seedsY = [];
      for (var i = 0; i < this.settings.maxParticles; i++) {
        this.seedsX.push(Math.random() * 20 - 10);
        this.seedsY.push(Math.random() * 30 - 10);
      }
    }

    // Set up a function to create multiple particles
    genParticle() {
      if (this.particleIndex !== this.settings.maxParticles) {
        var newParticle = {};
        
        newParticle.x = this.settings.startingX;
        newParticle.y = this.settings.startingY;
        newParticle.vx = 0;
        newParticle.vy = 0;
        // Establish starting positions and velocities
        if(this.useDefaultAnim == true){
          newParticle.vx = this.seedsX[this.currentAngle];
          newParticle.vy = this.seedsY[this.currentAngle];
          this.currentAngle++;
        }

        // Add new particle to the index
        // Object used as it's simpler to manage that an array
      
        newParticle.id = this.particleIndex;
        newParticle.life = 0;
        newParticle.maxLife = this.settings.maxLife;

        this.particles[this.particleIndex] = newParticle;

        this.particleIndex++;
      } else {
        if(this.useDefaultAnim == true){
          //console.log('Generating more seed angles');
          this.seedAngles();
          this.currentAngle = 0;
        }
        this.particleIndex = 0;
      }
    }

    normalize2D(vec2 = []) {
      var normal = Math.sqrt(Math.pow(vec2[0],2)+Math.pow(vec2[1],2));
      return [vec2[0]/normal,vec2[1]/normal];
    }

    // Keep particles within walls
    updateParticle = (i) => {
      
      this.particles[i].x += this.particles[i].vx+this.particles[i].vx*this.frameRate;
      this.particles[i].y += this.particles[i].vy+this.particles[i].vx*this.frameRate;
    
      if((this.particles[i].vx > this.settings.maxSpeed) || (this.particles[i].vy > this.settings.maxSpeed) || (this.particles[i].vx < -this.settings.maxSpeed) || (this.particles[i].vy < -this.settings.maxSpeed)) {
        var normalized = this.normalize2D([this.particles[i].vx,this.particles[i].vy]);
        this.particles[i].vx = normalized[0]*this.settings.maxSpeed;
        this.particles[i].vy = normalized[1]*this.settings.maxSpeed;
      }
      
      // Give the particle some bounce
      if ((this.particles[i].y + this.settings.particleSize) > this.settings.groundLevel) {
        this.particles[i].vy *= this.settings.yBounce;
        this.particles[i].vx *= -this.settings.xBounce;
        this.particles[i].y = this.settings.groundLevel - this.settings.particleSize;
      }

      // Give the particle some bounce
      if ((this.particles[i].y - this.settings.particleSize) < this.settings.ceilingWall) {
        this.particles[i].vy *= this.settings.yBounce;
        this.particles[i].vx *= -this.settings.xBounce;
        this.particles[i].y = this.settings.ceilingWall + this.settings.particleSize;
      }

      // Determine whether to bounce the particle off a wall
      if (this.particles[i].x - (this.settings.particleSize) <= this.settings.leftWall) {
        this.particles[i].vx *= this.settings.xBounce;
        this.particles[i].x = this.settings.leftWall + (this.settings.particleSize);
      }

      if (this.particles[i].x + (this.settings.particleSize) >= this.settings.rightWall) {
        this.particles[i].vx *= this.settings.xBounce;
        this.particles[i].x = this.settings.rightWall - this.settings.particleSize;
      }

      // Adjust for gravity
      this.particles[i].vy += this.settings.gravity;

      // Age the particle
      this.particles[i].life++;

      // If Particle is old, it goes in the chamber for renewal
      if (this.particles[i].life >= this.particles[i].maxLife) {
        this.particles.splice(i,1);
      }

    }

    deInit() {
     cancelAnimationFrame(this.animationId);
    }

    draw = () => {
      this.lastFrame = this.thisFrame;
      this.thisFrame = performance.now();
      this.frameRate = (this.thisFrame - this.lastFrame) * 0.001; //Framerate in seconds

      this.context.fillStyle = "rgba(10,10,10,0.8)";
      this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);

      // Draw a left, right walls and floor
      this.context.fillStyle = "red";
      this.context.fillRect(0, 0, this.settings.leftWall, this.canvas.height);
      this.context.fillRect(this.settings.rightWall, 0, this.canvas.width, this.canvas.height);
      this.context.fillRect(0, this.settings.groundLevel, this.canvas.width, this.canvas.height);
      this.context.fillRect(0, 0, this.canvas.width, this.settings.ceilingWall);
      
      
      // Draw the particles
      if(this.particles.length < this.settings.maxParticles) {
        for (var i = 0; i < (this.settings.maxParticles - this.particles.length); i++) {
          this.genParticle();
          //console.log(this.particles[i]);
        }
      }


      for (var i in this.particles) {
        this.updateParticle( i );
        // Create the shapes
        //context.fillStyle = "red";
        //context.fillRect(this.x, this.y, settings.particleSize, settings.particleSize);
        this.context.clearRect(this.settings.leftWall, this.settings.groundLevel, this.canvas.width, this.canvas.height);
        this.context.beginPath();
        this.context.fillStyle="rgb("+String(Math.abs(this.particles[i].vx)*75)+","+String(Math.abs(this.particles[i].vx)*25)+","+String(255 - Math.abs(this.particles[i].vx)*75)+")";
        // Draws a circle of radius 20 at the coordinates 100,100 on the canvas
        this.context.arc(this.particles[i].x, this.particles[i].y, this.settings.particleSize, 0, Math.PI*2, true); 
        this.context.closePath();
        this.context.fill();
      }

      setTimeout(() => {this.animationId = requestAnimationFrame(this.draw)},20);
    }
  }


 //Parse Audio file buffers
 class BufferLoader { //From HTML5 Rocks tutorial
   constructor(ctx, urlList, callback){
    this.ctx = ctx;
    this.urlList = urlList;
    this.onload = callback;
    this.bufferList = new Array();
    this.loadCount = 0;
   }

   loadBuffer(url='',index){
    // Load buffer asynchronously
    var request = new XMLHttpRequest();
    request.responseType = "arraybuffer";
    var responseBuf = null;
    
    if((url.indexOf("http://") != -1) || (url.indexOf("file://") != -1)){
        request.open("GET", url, true);
        request.onreadystatechange = () => {
          if(request.readyState === 4){
            if(request.status === 200 || request.status == 0){
              responseBuf = request.response; //Local files work on a webserver with request
            }
          }
        }
      var loader = this;

      request.onload = function() {
        // Asynchronously decode the audio file data in request.response
        loader.ctx.decodeAudioData(
          responseBuf,
          function(buffer) {
            if (!buffer) {
              alert('error decoding file data: ' + url);
              return;
            }
            loader.bufferList[index] = buffer;
            if (++loader.loadCount == loader.urlList.length)
              loader.onload(loader.bufferList);
          },
          function(error) {
            console.error('decodeAudioData error: ', error);
          }
        );
      }
      request.onerror = function() {
        alert('BufferLoader: XHR error');
      }
    
      request.send();
    }
    else{//Local Audio
      //read and decode the file into audio array buffer 
      var loader = this;
      var fr = new FileReader();
      fr.onload = function(e) {
          var fileResult = e.target.result;
          var audioContext = loader.ctx;
          if (audioContext === null) {
              return;
          }
          console.log("Decoding audio...");
          audioContext.decodeAudioData(fileResult, function(buffer) {
            if (!buffer) {
              alert('Error decoding file data: ' + url);
              return;
            }
            else{
              console.log('File decoded successfully!')
            }
            loader.bufferList[index] = buffer;
            if (++loader.loadCount == loader.urlList.length)
              loader.onload(loader.bufferList);
            },
            function(error) {
              console.error('decodeAudioData error: ', error);
            }
          );
      }
      fr.onerror = function(e) {
          console.log(e);
      }
      
      var input = document.createElement('input');
      input.type = 'file';
      input.multiple = true;

      input.onchange = e => {
        fr.readAsArrayBuffer(e.target.files[0]);
        input.value = '';
        }
      input.click();
    }

  }

  load(){
    for (var i = 0; i < this.urlList.length; ++i)
    this.loadBuffer(this.urlList[i], i);
  }
  
}

class SoundJS { //Only one Audio context at a time!
  constructor(){
    window.AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
    
    this.ctx = null;
    try {
      this.ctx = new AudioContext();
    } catch (e) {
      alert("Your browser does not support AudioContext!");
      console.log(e);
    } 
    
    this.sourceList = [];
    
    this.recordedData = [];
    this.recorder = null;
    this.buffer = [];

    this.osc = [];
    this.gain = this.ctx.createGain();
    this.analyser = this.ctx.createAnalyser();
    this.out = this.ctx.destination;
    this.gain.connect(this.analyser)
    this.analyser.connect(this.out);
    
  }

  playFreq(freq=[1000], seconds=0, type='sine', startTime=this.ctx.currentTime){ //Oscillators are single use items. Types: sine, square, sawtooth, triangle, or custom via setPeriodicWave()
    freq.forEach((element)=>{
      var len = this.osc.length;
        this.osc[len] = this.ctx.createOscillator();
        this.osc[len].start();
        this.osc[len].onended = () => {
          this.osc.splice(len,1);
        }
      this.osc[len].type = type;
      this.osc[len].connect(this.gain);
      this.osc[len].frequency.setValueAtTime(element, startTime);
      if(seconds!=0){
        //0 = unlimited 
        this.osc[len].stop(startTime+seconds);
      }
    });
  }

  stopFreq(firstIndex=0, number=1, delay=0){//Stops and removes the selected oscillator(s). Can specify delay.
    for(var i = firstIndex; i<number; i++){
      if(this.osc[oscIndex]){
        this.osc[oscIndex].stop(this.ctx.currentTime+delay);
      }
      else{
        console.log("No oscillator found.")
      }
    }
  }

  finishedLoading = (bufferList) => {
    bufferList.forEach((element) => {
      this.sourceList.push(this.ctx.createBufferSource()); 
      var idx = this.sourceList.length - 1;
      this.sourceList[idx].buffer = element;
      this.sourceList[idx].onended = () => {this.sourceList.splice(idx, 1)};
      this.sourceList[idx].connect(this.gain); //Attach to volume node
    });
  }

  addSounds(urlList=['']){
    var bufferLoader = new BufferLoader(this.ctx, urlList, this.finishedLoading)
    bufferLoader.load();
  }

  playSound(bufferIndex, seconds=0, repeat=false, startTime=this.ctx.currentTime){//Plays sounds loaded in buffer by index. Sound buffers are single use items.
    if(repeat == true){
      this.sourceList[bufferIndex].loop = true;
    }
    
    this.sourceList[bufferIndex].start(startTime);
    if(seconds != 0){
      this.sourceList[bufferIndex].stop(startTime+seconds);
    }
  }

  stopSound(bufferIndex){
    this.sourceList[bufferIndex].stop(0);
  }

  setPlaybackRate(bufferIndex, rate){
    this.sourceList[bufferIndex].playbackRate.value = rate;
  }

  record(name = new Date().toISOString(), args={audio:true, video:false}, type=null, streamElement=null){ // video settings vary e.g. video:{width:{min:1024,ideal:1280,max:1920},height:{min:576,ideal:720,max:1080}}
    /*
    navigator.mediaDevices.enumerateDevices().then((devices) => {
      devices = devices.filter((d) => d.kind === 'audioinput');
      devices.forEach(function(device) {
        let menu = document.getElementById("inputdevices");
        if (device.kind == "audioinput") {
          let item = document.createElement("option");
          item.innerHTML = device.label;
          item.value = device.deviceId;
          menu.appendChild(item);
          }
      });
    }); //Device selection

    navigator.permissions.query({name:'microphone'}).then(function(result) {
      if (result.state == 'granted') {

      } else if (result.state == 'prompt') {

      } else if (result.state == 'denied') {

      }
      result.onchange = function() {

      };
    });
    */
    var supported = null;
    var ext = null;
    var types = type;
    if(types==null){
      if(args.video != false){
        types = [
          'video/webm',
          'video/webm;codecs=vp8',
          'video/webm;codecs=vp9',
          'video/webm;codecs=vp8.0',
          'video/webm;codecs=vp9.0',
          'video/webm;codecs=h264',
          'video/webm;codecs=H264',
          'video/webm;codecs=avc1',
          'video/webm;codecs=vp8,opus',
          'video/WEBM;codecs=VP8,OPUS',
          'video/webm;codecs=vp9,opus',
          'video/webm;codecs=vp8,vp9,opus',
          'video/webm;codecs=h264,opus',
          'video/webm;codecs=h264,vp9,opus',
          'video/x-matroska;codecs=avc1'
        ];
        }
      else if(args.audio == true){
        types = [
          'audio/wav', // might be supported native, otherwise see:
          'audio/mp3', // probably not supported
          'audio/webm',
          'audio/webm;codecs=opus',
          'audio/webm;codecs=pcm',
          'audio/ogg',
          'audio/x-matroska' // probably not supported
        ];
      }
    }

    for(var i=0; i<types.length; i++){
      if(MediaRecorder.isTypeSupported(types[i]) == true){
        supported = types[i];
        console.log("Supported type: ", supported);
        if(types[i].indexOf('webm') != -1){
          ext = '.webm';
        }
        if(types[i].indexOf('ogg') != -1){
          ext = '.ogg';
        }
        if(types[i].indexOf('mp3') != -1){
          ext = '.mp3';
        }
        if(types[i].indexOf('wav') != -1){
          ext = '.wav';
        }
        if(types[i].indexOf('x-matroska') != -1){
          ext = '.mkv';
        }
        break;
      }
    }

    if(supported != null){
      function errfunc(e) {
        console.log(e);
      } 

      navigator.mediaDevices.getUserMedia(args).then((recordingDevice) => { //Get
        console.log("Media stream created.");
        
        if(streamElement != null){ // attach to audio or video element, or Audio(). For canvas, use an AudioContext analyzer.
          streamElement.src = window.URL.createObjectURL(recordingDevice);
        }

        this.recorder = new MediaRecorder(recordingDevice);

        this.recorder.onstop = (e) => {
          console.log("Media recorded, saving...");

          var blob = new Blob(this.recordedData, {
            type: supported
          });

          var url = URL.createObjectURL(blob);
          var a = document.createElement("a");
          document.body.appendChild(a);
          a.style = "display: none";
          a.href = url;
          a.download = name + ext;
          a.click();
          window.URL.revokeObjectURL(url);
        }
        
        this.recorder.ondataavailable = (e) => {
          this.recordedData.push(e.data);
        }

        this.recorder.start(); //Begin recording

      }, errfunc);

    }
    else {
      alert("Cannot record! Check function call settings, ensure browser is compatible.");
    }
  }

  replayRecording(streamElement) { //Replay the currently buffered recording in an acceptable stream element, e.g. attach to audio or video element, or an Audio() class, or a video element. For canvas, use an AudioContext analyzer.
    if(this.recordedData.length > 1){
      this.buffer = new Blob(this.recordedData);
      streamElement.src = window.URL.createObjectURL(buffer);
    }
  }

 }

 
class geolocateJS {
    constructor(){
      if(navigator.geolocation){
        
      }
      else{
        alert("Geolocation not supported in this browser!");
      }

      this.locationData=[];
    }

    showPosition(position){
      //alert("Lat: "+position.coords.latitude+", Lon: "+position.coords.longitude);
      this.locationData.push(new Date().toISOString()+","+position.coords.latitude+","+position.coords.longitude);
    }

    getPosition(){
      navigator.geolocation.getCurrentPosition(this.showPosition);
    }

 }


class bleUtils { //This is formatted for the way the HEG sends/receives information. Other BLE devices will likely need changes to this to be interactive.
   constructor(async = false, serviceUUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e', rxUUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e', txUUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e', defaultUI = true, parentId="main_body" , buttonId = "blebutton"){
    this.serviceUUID = serviceUUID;
    this.rxUUID      = rxUUID; //characteristic that can receive input from this device
    this.txUUID      = txUUID; //characteristic that can transmit input to this device
    this.encoder     = new TextEncoder("utf-8");
    this.decoder     = new TextDecoder("utf-8");

    this.device  = null;
    this.server  = null;
    this.service = null;
    this.rxchar  = null; //receiver on the BLE device (write to this)
    this.txchar  = null; //transmitter on the BLE device (read from this)

    this.parentId = parentId;
    this.buttonId = buttonId;

    this.async = async;

    this.android = navigator.userAgent.toLowerCase().indexOf("android") > -1; //Use fast mode on android (lower MTU throughput)

    this.n; //nsamples

    if(defaultUI = true){
      this.initUI(parentId, buttonId);
    }
    
   }

   initUI(parentId, buttonId) {
    if(this.device != null){
      if (this.device.gatt.connected) {
        this.device.gatt.disconnect();
        console.log("device disconnected")
      }
    }
    var HTMLtoAppend = '<button id="'+buttonId+'">BLE Connect</button>';
    HEGwebAPI.appendFragment(HTMLtoAppend,parentId);
    document.getElementById(buttonId).onclick = () => { 
      if(this.async === false) {
        this.initBLE();
      } 
      else{
        this.initBLEasync();
      } 
    }
   }

   //Typical web BLE calls
   initBLE = (serviceUUID = this.serviceUUID, rxUUID = this.rxUUID, txUUID = this.txUUID) => { //Must be run by button press or user-initiated call
    navigator.bluetooth.requestDevice({   
      acceptAllDevices: true,
      optionalServices: [serviceUUID] 
      })
      .then(device => {
          //document.getElementById("device").innerHTML += device.name+ "/"+ device.id +"/"+ device.gatt.connected+"<br>";
          this.device = device;
          return device.gatt.connect(); //Connect to HEG
      })
      .then(sleeper(100)).then(server => server.getPrimaryService(serviceUUID))
      .then(sleeper(100)).then(service => { 
        this.service = service;
        service.getCharacteristic(rxUUID).then(sleeper(100)).then(tx => {
          this.rxchar = tx;
          return tx.writeValue(this.encoder.encode("t")); // Send command to start HEG automatically (if not already started)
        });
        if(this.android == true){
          service.getCharacteristic(rxUUID).then(sleeper(1000)).then(tx => {
            return tx.writeValue(this.encoder.encode("o")); // Fast output mode for android
          });
        }
        return service.getCharacteristic(txUUID) // Get stream source
      })
      .then(sleeper(1100)).then(characteristic=>{
          this.txchar = characteristic;
          return characteristic.startNotifications(); // Subscribe to stream
      })
      .then(sleeper(100)).then(characteristic => {
          characteristic.addEventListener('characteristicvaluechanged',
                                          this.onNotificationCallback) //Update page with each notification
      }).then(sleeper(100)).then(this.onConnectedCallback())
      .catch(err => {console.error(err);});
      
      function sleeper(ms) {
          return function(x) {
              return new Promise(resolve => setTimeout(() => resolve(x), ms));
          };
      }
   }

   onNotificationCallback = (e) => { //Customize this with the UI (e.g. have it call the handleScore function)
     var val = this.decoder.decode(e.target.value);
     console.log("BLE MSG: ",val);
   }


   onConnectedCallback = () => {
      //Use this to set up the front end UI once connected here
   }

   sendMessage = (msg) => {
     this.rxchar.writeValue(this.encoder.encode(msg));
   }

   //Async solution fix for slower devices (android). This is slower than the other method on PC. Credit Dovydas Stirpeika
   async connectAsync() {
        this.device = await navigator.bluetooth.requestDevice({
            filters: [{ namePrefix: 'HEG' }],
            optionalServices: [this.serviceUUID]
        });

        console.log("BLE Device: ", this.device);
        
        const btServer = await this.device.gatt?.connect();
        if (!btServer) throw 'no connection';
        this.device.addEventListener('gattserverdisconnected', onDisconnected);
        
        this.server = btServer;
        
        const service = await this.server.getPrimaryService(this.serviceUUID);
        
        // Send command to start HEG automatically (if not already started)
        const tx = await service.getCharacteristic(this.rxUUID);
        await tx.writeValue(this.encoder.encode("t"));

        if(this.android == true){
          await tx.writeValue(this.encoder.encode("o"));
        }
        
        this.characteristic = await service.getCharacteristic(this.txUUID);
         this.onConnectedCallback();
        return true;
    }

    disconnect = () => this.server?.disconnect();

    onDisconnected = () => {
      console.log("BLE device disconnected!");
    }

    async readDeviceAsync () {
        if (!this.characteristic) {
            console.log("HEG not connected");
            throw "error";
        }

        // await this.characteristic.startNotifications();
        this.doReadHeg = true;
        
        var data = ""
        while (this.doReadHeg) {
            const val = this.decoder.decode(await this.characteristic.readValue());
            if (val !== this.data) {
                data = val;
                console.log(data);
                //data = data[data.length - 1];
                //const arr = data.replace(/[\n\r]+/g, '')
                this.n += 1;
                this.onReadAsyncCallback(data);
            }
        }
    }

    onReadAsyncCallback = (data) => {
      console.log("BLE Data: ",data)
    }

    stopReadAsync = () => {
        this.doReadHeg = false;
        tx.writeValue(this.encoder.encode("f"));
    }

    spsinterval = () => {
      setTimeout(() => {
        console.log("SPS", this.n + '');
        this.n = 0;
        this.spsinterval();
      }, 1000);
    }

    async initBLEasync() {
      await this.connectAsync();
      this.readDeviceasync();
      this.spsinterval();
    }
      
 }