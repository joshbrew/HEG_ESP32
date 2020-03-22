const char HEGwebAPI[] PROGMEM = R"=====(
class HEGwebAPI {
  constructor(host='', defaultUI=true, parentId="main_body"){
    
    this.startTime=0;
    this.us=[];
    this.red=[];
    this.ir=[];
    this.ratio=[];
    this.ambient=[];
    this.velAvg=[];
    this.accelAvg=[];

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

    this.sensitivity = null;
    
    window.requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.msRequestAnimationFrame;
    window.cancelAnimationFrame = window.cancelAnimationFrame || window.webkitCancelAnimationFrame || window.mozCancelAnimationFrame || window.msCancelAnimationFrame;
    if(defaultUI==true){
      this.parentId = parentId;
      this.createUI(parentId);
    }
    /*
    window.addEventListener('message', e => {
      if(e.origin.startsWith(window.location.hostname)) {
        
      }
      else {
        this.handleEventData(e); 
      }
    }); //Generic event listener for postMessage events
    */
    this.createEventListeners(host);
  }

  resetVars() {
    this.startTime = 0;
    this.us = [];
    this.red = [];
    this.ir = [];
    this.ratio = [];
    this.ambient = [];
    this.velAvg = [];
    this.accelAvg = [];
    
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

  static sma(arr, window) {
    var temp = [];
    for(var i = 0; i < arr.length; i++) {
      if((i == 0)) {
        temp.push(arr[0]);
      }
      else if(i < window) {
        var arrslice = arr.slice(0,i+1);
        temp.push(arrslice.reduce((previous,current) => current += previous ) / (i+1));
      }
      else {
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

  saveCSV(){
    var csv = "us,Red,IR,Ratio,ambient,Vel,Accel,Notes\n"; //csv header
    for(var i = 0; i<this.us.length - 1; i++) {
      if(this.noteIndex.indexOf(i) != -1) {
        var temp = [this.us[i],this.red[i],this.ir[i],this.ratio[i],this.ambient[i],this.velAvg[i],this.accelAvg[i],[this.noteText[this.noteIndex.indexOf(i)]]].join(',') + "\n";
      }
      else{
        var temp = [this.us[i],this.red[i],this.ir[i],this.ratio[i],this.ambient[i],this.velAvg[i],this.accelAvg[i]].join(',') + "\n";
      }
      csv += temp;
    }
    var hiddenElement = document.createElement('a');
    hiddenElement.href = "data:text/csv;charset=utf-8," + encodeURI(csv);
    hiddenElement.target = "_blank";
    if(document.getElementById("csvname").value != ""){
      hiddenElement.download = document.getElementById("csvname").value+".csv";
    }
    else{
      hiddenElement.download = "session_data.csv";
    }
    hiddenElement.click();
  }

  handleScore() {
    //Define in-script
  }

  replayCSV() {
    if(this.csvIndex < 2){
      if(this.startTime == 0) { this.startTime = this.csvDat[this.csvIndex][0]}
      this.us.push(parseInt(this.csvDat[this.csvIndex][0]));
      this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
      this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
      this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
      this.ambient.push(parseInt(this.csvDat[this.csvIndex][4]));
      this.velAvg.push(parseFloat(this.csvDat[this.csvIndex][5]));
      this.accelAvg.push(parseFloat(this.csvDat[this.csvIndex][6]));
    }
    this.csvIndex++;
    if(this.csvIndex < this.csvDat.length - 1){
      if(this.startTime == 0) { this.startTime = this.csvDat[this.csvIndex][0]}
      this.us.push(parseInt(this.csvDat[this.csvIndex][0]));
      this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
      this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
      this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
      this.ambient.push(parseInt(this.csvDat[this.csvIndex][4]));
      this.velAvg.push(parseFloat(this.csvDat[this.csvIndex][5]));
      this.accelAvg.push(parseFloat(this.csvDat[this.csvIndex][6]));
      if(this.us.length >= 2){
        this.handleScore();
      }
    }
    else {
      this.replay = false;
      this.csvDat = [];
      this.csvIndex = 0;
    }
    this.endOfEvent();
    setTimeout(() => {this.replayCSV();},(this.us[this.csvIndex]-this.us[this.csvIndex-1])*0.001); //Call until end of index.
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

  handleEventData(data){
    console.log("HEGDUINO", data);
    if(document.getElementById("heg").innerHTML != data){  //on new output
      document.getElementById("heg").innerHTML = data;
      var onRead = new CustomEvent('on_read', { detail: {data: data} });
      window.parent.dispatchEvent(onRead); 
      window.parent.postMessage(data, "*");
      if(data.includes("|")) {
        var dataArray = data.split("|");
        var thisRatio = parseFloat(dataArray[3]);
        if((thisRatio > 0)){ //&& (((this.us.length > 2) && ((thisRatio / this.ratio[this.ratio.length - 1] > 0.6) && (thisRatio/this.ratio[this.ratio.length - 1] < 1.4))) || (this.us.length < 2))) { // Add error filtering here.
          if(this.startTime == 0) { this.startTime = parseInt(dataArray[0])}
          this.us.push(parseInt(dataArray[0]));
          this.red.push(parseInt(dataArray[1]));
          this.ir.push(parseInt(dataArray[2]));
          this.ratio.push(parseFloat(dataArray[3]));
          this.ambient.push(parseInt(dataArray[4]));
          this.velAvg.push(parseFloat(dataArray[5]));
          this.accelAvg.push(parseFloat(dataArray[6]));

          if(this.us.length > 5) { // SMA filtering for ratio
            var temp = HEGwebAPI.sma(this.ratio.slice(this.ratio.length - 7, this.ratio.length-1),5);
            if((this.ratio[this.ratio.length - 1] < temp[4] * 0.7) || (this.ratio[this.ratio.length - 1] > temp[4] * 1.3)) {
              this.ratio[this.ratio.length - 1] = this.ratio[this.ratio.length - 2]; // Roll the ratio back if outside margin 
              this.red[this.red.length - 1] = this.red[this.red.length - 2]; // roll back the other values too for good measure
              this.ir[this.ir.length - 1] = this.ir[this.ir.length - 2];
              this.ambient[this.ambient.length - 1] = this.ambient[this.ambient.length - 2];
              this.velAvg[this.velAvg.length - 1] = this.velAvg[this.velAvg.length - 2];
              this.accelAvg[this.accelAvg.length - 1] = this.accelAvg[this.accelAvg.length - 2];
            } 
          }
          //handle new data
          this.handleScore();
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

  openEvent(e){
    console.log("HEGDUINO", "Events Connected");
    //document.getElementById("message").innerHTML = "Output:";
  }

  errorEvent(e){
    if (e.target.readyState != EventSource.OPEN) {
      console.log("HEGDUINO", "Events Disconnected");
    }
  }

  messageEvent(e){
    console.log("HEGDUINO", e.data);
    //document.getElementById("message").innerHTML = e.data;
  }

  hegEvent = (e) => {
    this.handleEventData(e.data);
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
 
  updateTable(){
    document.getElementById("dataTable").innerHTML = '<tr><td id="us">'+this.us[this.us.length-1]+'</td><td id="red">'+this.red[this.red.length-1]+'</td><td id="ir">'+this.ir[this.ir.length-1]+'</td><td id="ratio">'+this.ratio[this.ratio.length-1]+'</td><td id="ambient">'+this.ambient[this.ambient.length-1]+'</td><td id="Vel">'+this.velAvg[this.velAvg.length-1]+'</td><td id="Accel">'+this.accelAvg[this.accelAvg.length-1]+'</td><td class="scoreth">'+this.scoreArr[this.scoreArr.length-1].toFixed(4)+'</td></tr>';
  }

 

  createUI(parentId) {
    var hegapiHTML = '<div id="hegapi" class="hegapi"> \
      <table> \
      <tr><td><button id="startbutton" class="button startbutton">Start HEG</button></td> \
        <td><button id="stopbutton" class="button stopbutton" type="submit">Stop HEG</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td><input type="text" id="command" name="command" placeholder="Command"></td><td><button id="sendbutton" class="button sendbutton">Send</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2"><button class="button" id="resetSession" name="resetSession">Reset Session</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2" id="sensitivityLabel">Scoring Sensitivity</td></tr> \
      <tr><td><button class="button" id="reset_s">Default</button></td> \
        <td>Sensitivity: <span id="sensitivityVal">1.00</span><br><input type="range" class="slider" id="sensitivity" min="1" max="1000" value="100"></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td><div id="timestamp">Get Current Time</div></td><td><button id="getTime" class="button">Get Time</button></td></tr> \
      <tr><td colspan="2"><textarea id="noteText" placeholder="Point of Interest"></textarea></td></tr>\
      <tr><td colspan="2"><button id="saveNote" class="button">Annotate</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr>\
      <tr><td><input type="text" id="csvname" name="csvname" placeholder="session_data"></input></td> \
        <td><button class="button saveLoadButtons" id="savecsv">Save CSV</button></td></tr> \
      <tr><td colspan="2"><button class="button saveLoadButtons" id="replaycsv">Replay CSV</button></td></tr> \
      <tr><td colspan="2"><hr></td></tr> \
      <tr><td colspan="2" id="hostlabel">Host</td></tr> \
      <tr><td><input type="text" id="hostname" name="hostname" placeholder="http://192.168.4.1"></input></td><td><button id="submithost" class="button">Connect</button></td></tr>  \
      <tr><td colspan="2"><hr></td></tr> \
      </table></div> \
      <iframe name="dummyframe" id="dummyframe" class="dummy"></iframe> \
      ';

    var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>';
    var containerHTML = '<div id="container"></div>';
    var messageHTML = '<msgDiv id="message">Output:</div>';
    var eventHTML = '<eventDiv id="heg">Not connected...</eventDiv>';
    var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"><tr><th>us</th><th>Red</th><th>IR</th><th>Ratio</th><th>ambient</th><th>Vel</th><th>Accel</th><th class="scoreth">SMA Score</th></tr></table></div>';
    var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><th>Awaiting Data...</th></tr></table></div>';

    HEGwebAPI.appendFragment(dataDivHTML,parentId);
    HEGwebAPI.appendFragment(hegapiHTML, "dataDiv");
    HEGwebAPI.appendFragment(containerHTML, "dataDiv");
    HEGwebAPI.appendFragment(messageHTML,"container");
    HEGwebAPI.appendFragment(eventHTML,"container");
    HEGwebAPI.appendFragment(tableHeadHTML,"container");
    HEGwebAPI.appendFragment(tableDatHTML,"container");

    document.getElementById("getTime").onclick = () => {
      this.curIndex = this.us.length - 1;
      document.getElementById("timestamp").innerHTML = (this.us[this.us.length - 1] * 0.000001).toFixed(2) + "s";
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
    this.us = 0;
    this.ratio = 0;
    this.score = 0;
    this.viewing = 0;

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
    this.us = 0;
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
      <tr><td>X Offset:<br><input type="range" class="slider" id="xoffset" min=0 max=1000 value=0></td><td><button id="xoffsetbutton" class="button">Reset</button></td></tr> \
      <tr><td>X Scale:<br><input type="range" class="slider" id="xscale" min=10 max='+(this.VERTEX_LENGTH * 5).toFixed(0)+' value='+this.VERTEX_LENGTH.toFixed(0)+'></td><td><button id="xscalebutton" class="button">Reset</button></td></tr> \
      <tr><td>Y Offset:<br><input type="range" class="slider" id="yoffset" min=0 max=10000 value=5000></td><td><button id="yoffsetbutton" class="button">Reset</button></td></tr> \
      <tr><td>Y Scale:<br><input type="range" class="slider" id="yscale" min=1 max=400 value=200></td><td><button id="yscalebutton" class="button">Reset</button></td></tr> \
      </table><br> \
      <table id="graphViewTable"><tr><td>View:</td><td><form name="graphform">Score<input type="radio" name="graphview" value="0" checked>Ratio<input type="radio" name="graphview" value="1"></form></td></tr></table> \
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
      const pointId = i / 2 | 0;
      const lerp0To1 = pointId / highestPointNdx;
      const isY = i % 2;
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

    var seconds = Math.floor(this.us*0.000001);
    var minutes = Math.floor(seconds*0.01667);
    seconds = seconds - minutes * 60
    if(seconds < 10){seconds = "0"+seconds}
    if(this.viewing == 0) {
      this.graphtext.fillStyle = "#00ff00";
      this.graphtext.fillText("|  Time (s): " + minutes + ":" + seconds,this.graphtext.canvas.width - 300,50);
      this.graphtext.fillText("|  Ratio: " + this.ratio.toFixed(2), this.graphtext.canvas.width - 500,50);
      this.graphtext.fillStyle = "#99ffbb";
      this.graphtext.fillText("    Score: " + this.graphY1[this.graphY1.length - 1].toFixed(2),this.graphtext.canvas.width - 720,50);
    }
    if(this.viewing == 1) {
      this.graphtext.fillStyle = "#00ff00";
      this.graphtext.fillText("|  Time (s): " + (this.us*0.000001).toFixed(2),this.graphtext.canvas.width - 300,50);
      this.graphtext.fillText("    Score: " + this.graphY1[this.graphY1.length - 1].toFixed(2) + "  |",this.graphtext.canvas.width - 720,50);
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
  constructor(bgColor="#34baeb", cColor="#ff3a17", res=[window.innerWidth,"440"], parentId="main_body", canvasId="circlecanvas", defaultUI=true){
    
    this.createCanvas(parentId, canvasId, res);
    this.c = document.getElementById(canvasId);
    this.ctx = this.c.getContext('2d');
     
    this.defaultUI = defaultUI
    //if(defaultUI == true){    }
 
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

  deInit() {
    cancelAnimationFrame(this.animationId);
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
       
      // draw the circle
      this.ctx.beginPath();

      if(((this.angle > 1.57) || (this.angleChange > 0)) && ((this.angle < 3.14) || (this.angleChange < 0))){ //generalize
          this.angle += this.angleChange*0.1;
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
      constructor(res=["700","440"], vidapiId="vidapi", vidContainerId="vidbox", defaultUI=true, parentId="main_body"){
        this.playRate = 1;
        this.alpha = 0;
        this.volume = 0.5;
        this.useAlpha = true;
        this.useRate = true;
        this.useVolume = true;
        this.enableControls = false;
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
         else{ this.useRate = true; this.playRate = 1; document.getElementById("useRate").style.opacity = "1.0";}
        }

        document.getElementById("useVol").onclick = () => {
         if(this.useVolume == true){
           this.vidQuery.muted = true;
           this.useVolume = false;
           this.volume = 0;
           this.vidQuery.volume = 0;
           document.getElementById("useVol").style.opacity = "0.3";
         }
         else{ 
          this.useVolume = true; 
          this.vidQuery.muted = false; 
          this.volume = 0.5; 
          this.vidQuery.volume = 0.5;
          document.getElementById("useVol").style.opacity = "1.0";
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
          <tr><td><button class="button vdvol" id="useVol" name="useVol">Volume</button></td></tr></div> \
          </table> \
        </div>';
       HEGwebAPI.appendFragment(videoapiHTML, parentId);

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
       if(this.useVolume == true){
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
 
 class audioJS { //Modified from: https://codepen.io/jackfuchs/pen/yOqzEW
  constructor(res=["800","400"], audioId="audio", audmenuId="audmenu", defaultUI=true, parentId="main_body") {
    this.audioId = audioId;
    this.audmenuId = audmenuId;
    
    var visualizerHTML = '<div id="'+this.audioId+'" class="visualizerDiv" class-"canvasContainer"> \
    <canvas class="audiocanvas" id="'+this.audioId+'canvas" width="'+res[0]+'" height="'+res[1]+'"></canvas> \
      </div> \
    ';

    HEGwebAPI.appendFragment(visualizerHTML, parentId);

    this.c = document.getElementById(this.audioId+"canvas");
    this.ctx = this.c.getContext("2d");
    this.gradient = this.ctx.createLinearGradient(0, 0, 0, 300);
    this.gradient.addColorStop(1, 'springgreen');
    this.gradient.addColorStop(0.75, 'yellow');
    this.gradient.addColorStop(0, 'red');

    this.defaultUI = defaultUI;
    this.hidden = false;
    if(defaultUI==true) {
      this.initUI(parentId);
    }

    this.maxVol = 1;
    this.file = null; //the current file
    this.fileName = null; //the current file name
    this.audioContext = null;
    this.source = null; //the audio source
    this.analyser = null;
    this.gainNode = null;
    this.info = document.getElementById('fileinfo').innerHTML; //this used to upgrade the UI information
    this.menu = document.getElementById(this.audmenuId);
    this.infoUpdateId = null; //to sotore the setTimeout ID and clear the interval
    this.animationId = null;
    this.status = 0; //flag for sound is playing 1 or stopped 0
    this.forceStop = false;
    this.allCapsReachBottom = false;


    this.meterWidth = 14; //width of the meters in the spectrum
    this.gap = 2; //gap between meters
    this.capHeight = 2;
    this.capStyle = '#fff';
    this.meterNum = 60; //count of the meters
    
    this.init();
  }

  stopAudio(){
    //stop the previous sound if any
    if (this.animationId !== null) {
        cancelAnimationFrame(this.animationId);
    }
    if (this.source !== null) {
        this.source.stop(0);
    }
  }

  createVisualizer(audioContext, buffer){
      var audioBufferSourceNode = audioContext.createBufferSource();
      this.analyser = audioContext.createAnalyser();
      this.gainNode = audioContext.createGain();
      var that = this;
      //connect the source to the modifier nodes
      audioBufferSourceNode.connect(this.gainNode);
      this.gainNode.connect(this.analyser);
      //connect the last node to the destination(the speaker), or we won't hear the sound
      this.analyser.connect(audioContext.destination);
      //then assign the buffer to the buffer source node
      audioBufferSourceNode.buffer = buffer;
      //play the source
      if (!audioBufferSourceNode.start) {
          audioBufferSourceNode.start = audioBufferSourceNode.noteOn //in old browsers use noteOn method
          audioBufferSourceNode.stop = audioBufferSourceNode.noteOff //in old browsers use noteOff method
      };
      this.stopAudio();
      audioBufferSourceNode.start(0);
      this.gainNode.gain.setValueAtTime(this.maxVol, this.audioContext.currentTime);
      this.status = 1;
      this.source = audioBufferSourceNode;
      audioBufferSourceNode.onended = function() {
          that.endAudio(that);
      };
      this.updateInfo('Playing ' + this.fileName, false);
      this.info = 'Playing ' + this.fileName;
      document.getElementById('fileWrapper').style.opacity = 0.2;
      this.draw(this.analyser);
  }

  initUI(parentId){
      var audiomenuHTML = '<div id="'+this.audmenuId+'"> \
        <table id="audtable" class="audtable">\
          <tr><td>Feedback: </td></tr> \
          <tr><td><button id="useVol" name="useVol" class="button">Volume</button></td></tr> \
        </table>\
        <input type="range" class="slider volSlider" id="volSlider" name="volSlider" min="0" max="100" value="100"> \
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
          if(this.gainNode != null) {
            this.gainNode.gain.setValueAtTime(this.maxVol, this.audioContext.currentTime);
          }
          document.getElementById("useVol").style.opacity = "0.3";
        }
      }

      document.getElementById("volSlider").oninput = () => {
        this.maxVol = document.getElementById("volSlider").value * 0.01;
        if(this.gainNode != null) {
          this.gainNode.gain.setValueAtTime(this.maxVol, this.audioContext.currentTime);
        }
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
      var that = this;
      var file = this.file;
      var fr = new FileReader();
      fr.onload = function(e) {
          var fileResult = e.target.result;
          var audioContext = that.audioContext;
          if (audioContext === null) {
              return;
          };
          that.updateInfo('Decoding the audio', true);
          audioContext.decodeAudioData(fileResult, function(buffer) {
              that.updateInfo('Decode successful, starting the visualizer', true);
              that.createVisualizer(audioContext, buffer);
          }, function(e) {
              that.updateInfo('!Fail to decode the file', false);
              console.log(e);
          });
      };
      fr.onerror = function(e) {
          that.updateInfo('!Fail to read the file', false);
          console.log(e);
      };
      //assign the file to the reader
      this.updateInfo('Starting read the file', true);
      fr.readAsArrayBuffer(file);
  }

  onData(score){
    if(this.useVol == true) {
      var newVol = this.gainNode.gain.value + score;
      if(newVol > this.maxVol){
        newVol = this.maxVol;
      }
      if(newVol < 0){
        newVol = 0;
      }
      if(this.defaultUI == true) {
        document.getElementById("volSlider").value = newVol * 100;
      }
      this.gainNode.gain.setValueAtTime(newVol, this.audioContext.currentTime);
    }
  }

  endAudio(instance){
    if (this.forceStop) {
      this.forceStop = false;
      this.status = 1;
      return;
    };
    this.status = 0;
    var text = 'HTML5 Audio API showcase | An Audio Visualizer';
    document.getElementById('fileWrapper').style.opacity = 1;
    document.getElementById('fileinfo').innerHTML = text;
    instance.info = text;
    document.getElementById('uploadedFile').value = '';
  }

  updateInfo(text, processing) {
    var infoBar = document.getElementById('fileinfo'),
    dots = '...',
    i = 0,
    that = this;
    infoBar.innerHTML = text + dots.substring(0, i++);
    if (this.infoUpdateId !== null) {
        clearTimeout(this.infoUpdateId);
    };
    if (processing) {
        //animate dots at the end of the info text
        var animateDot = function() {
            if (i > 3) {
                i = 0
            };
            infoBar.innerHTML = text + dots.substring(0, i++);
            that.infoUpdateId = setTimeout(animateDot, 250);
        }
        this.infoUpdateId = setTimeout(animateDot, 250);
    };
  }

  init(){
      window.AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
      try {
          this.audioContext = new AudioContext();
      } catch (e) {
          this.updateInfo('!Your browser does not support AudioContext', false);
          console.log(e);
      } 
      var that = this;
      var audioInput = document.getElementById('uploadedFile');
      var dropContainer = document.getElementById(this.audioId+"canvas");
      //listen the file upload
      audioInput.onchange = function() {
        if (that.audioContext===null) {return;};
        
        //the if statement fixes the file selction cancle, because the onchange will trigger even the file selection been canceled
        if (audioInput.files.length !== 0) {
            //only process the first file
            that.file = audioInput.files[0];
            that.fileName = that.file.name;
            if (that.status === 1) {
                //the sound is still playing but we upload another file, so set the forceStop flag to true
                that.forceStop = true;
            };
            document.getElementById('fileWrapper').style.opacity = 1;
            that.updateInfo('Uploading', true);
            //once the file is ready,start the visualizer
            that.decodeAudio();
        };
      };
      //listen the drag & drop
      dropContainer.addEventListener("dragenter", function() {
          document.getElementById('fileWrapper').style.opacity = 1;
          that.updateInfo('Drop it on the page', true);
      }, false);
      dropContainer.addEventListener("dragover", function(e) {
          e.stopPropagation();
          e.preventDefault();
          //set the drop mode
          e.dataTransfer.dropEffect = 'copy';
      }, false);
      dropContainer.addEventListener("dragleave", function() {
          document.getElementById('fileWrapper').style.opacity = 0.2;
          that.updateInfo(that.info, false);
      }, false);
      dropContainer.addEventListener("drop", function(e) {
          e.stopPropagation();
          e.preventDefault();
          if (that.audioContext===null) {return;};
          document.getElementById('fileWrapper').style.opacity = 1;
          that.updateInfo('Uploading', true);
          //get the dropped file
          that.file = e.dataTransfer.files[0];
          if (that.status === 1) {
              document.getElementById('fileWrapper').style.opacity = 1;
              that.forceStop = true;
          };
          that.fileName = that.file.name;
          //once the file is ready,start the visualizer
          that.decodeAudio();
      }, false);
    }

    draw = (analyser) => {
      var that = this;
      var cwidth = this.c.width;
      var cheight = this.c.height - 2;
      var capYPositionArray = []; ////store the vertical position of the caps for the previous frame
      var drawMeter = function() {
          var array = new Uint8Array(analyser.frequencyBinCount);
          analyser.getByteFrequencyData(array);
          if (that.status === 0) {
              //fix when some sounds and the value still not back to zero
              for (var i = array.length - 1; i >= 0; i--) {
                  array[i] = 0;
              };
              that.allCapsReachBottom = true;
              for (var i = capYPositionArray.length - 1; i >= 0; i--) {
                  that.allCapsReachBottom = that.allCapsReachBottom && (capYPositionArray[i] === 0);
              };
              if (that.allCapsReachBottom) {
                  cancelAnimationFrame(that.animationId); //since the sound is stopped and animation finished, stop the requestAnimation to prevent potential memory leak,THIS IS VERY IMPORTANT!
                  return;
              };
          };
          var step = Math.round(array.length / that.meterNum); //sample limited data from the total array
          that.ctx.clearRect(0, 0, cwidth, cheight);
          for (var i = 0; i < that.meterNum * 0.85; i++) {
              var value = array[i * step];
              if (capYPositionArray.length < Math.round(that.meterNum)) {
                  capYPositionArray.push(value);
              };
              that.ctx.fillStyle = that.capStyle;
              //draw the cap, with transition effect
              var xoffset = that.meterWidth + that.gap;
              if (value < capYPositionArray[i]) {
                  that.ctx.fillRect(i * xoffset, cheight - (--capYPositionArray[i]), that.meterWidth, that.capHeight);
              } else {
                  that.ctx.fillRect(i * xoffset, cheight - value, that.meterWidth, that.capHeight);
                  capYPositionArray[i] = value;
              };
              that.ctx.fillStyle = that.gradient; //set the fillStyle to gradient for a better look
              that.ctx.fillRect(i * xoffset /*meterWidth+gap*/ , cheight - value + that.capHeight, that.meterWidth, cheight); //the meter
          }
          setTimeout(()=>{that.animationId = requestAnimationFrame(drawMeter);},15); 
      }
      setTimeout(()=>{this.animationId = requestAnimationFrame(drawMeter);},15); 
    }
 }

 class hillJS {
  constructor(res=["700","350"], updateInterval=2000, hillsId="hillscanvas", hillsmenuId="hillsmenu", defaultUI=true, parentId="main_body") {
   this.hillsId = hillsId;
   this.hillsmenuId = hillsmenuId;

   var canvasHTML = '<div id="canvasContainer" class="canvasContainer"> \
      <canvas class="hillcss" id="'+this.hillsId+'" width="'+res[0]+'" height="'+res[1]+'"></canvas> \
      ';

   HEGwebAPI.appendFragment(canvasHTML,parentId);

   this.defaultUI = defaultUI;
   if(defaultUI == true){
    this.initUI(parentId);
   }
   
   this.c = document.getElementById(this.hillsId);
   this.ctx = this.c.getContext("2d");
   this.menu = document.getElementById(this.hillsmenuId);
    
   this.gradient = this.ctx.createLinearGradient(0, 0, 0, 300);
   this.gradient.addColorStop(1, 'springgreen');
   this.gradient.addColorStop(0.75, 'sandybrown');
   this.gradient.addColorStop(0, '#CDE4FB');
   
   this.updateInterval = updateInterval;
   this.allCapsReachBottom = false;
   this.meterWidth = 12;
   this.gap = 2;
   this.capHeight = 2;
   this.capStyle = '#fff';
   this.hillNum = 50; //count of the meters
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
    var menuHTML = '<div id="'+this.hillsmenuId+'" class="hillapi"> \
    <button class="button" id="hillsRbutton">Reset</button> \
    </div>';
      
    HEGwebAPI.appendFragment(menuHTML,parentId);

    document.getElementById("hillsRbutton").onclick = () => {
      this.hillScore = [...Array(this.hillNum).fill(50)];
    }
    document.getElementById("startbutton").addEventListener('click',  this.draw, false);
    document.getElementById("stopbutton").addEventListener('click',  this.cancelDraw, false);
  }

  deInit(){
    document.getElementById("startbutton").removeEventListener('click', this.draw);
    document.getElementById("stopbutton").removeEventListener('click', this.cancelDraw);
  }

  onData(score){
    if(score < this.c.height){
      this.hillScore[this.hillScore.length - 1] += score*20;
      if(this.hillScore[this.hillScore.length - 1] < 10) { // minimum score (prevents rendering outside viewport)
        this.hillScore[this.hillScore.length - 1] = 10;
      }
      if(score > 0) {
        this.hillScore[this.hillScore.length - 1] += 0.5;
      }
      if(score < 0) {
        this.hillScore[this.hillScore.length - 1] -= 0.5;
      }
    }
    else {
      this.hillScore[this.hillScore.length - 1] = this.c.height;
    }
  }

  draw = () => {
    // Get data interval
    // Create background and bars
    // Change height of bars based on avg or rms. (all at 0 on fresh session)
    // Update last bar for every t time interval based on change
    var cwidth = this.c.width;
    var cheight = this.c.height - 2;
    var capYPositionArray = [];
    this.ctx.clearRect(0, 0, cwidth, cheight);
    for (var i = 0; i < this.hillNum; i++) {
        var value = this.hillScore[i];
        if(value < 0){ value = 0;}
        if (capYPositionArray.length < Math.round(this.hillNum)) {
            capYPositionArray.push(value);
        };
        this.ctx.fillStyle = this.capStyle;
        //draw the cap, with transition effect
        var xoffset = this.meterWidth + this.gap;
        if (value < capYPositionArray[i]) {
            this.ctx.fillRect(i * xoffset, cheight - (--capYPositionArray[i]), this.meterWidth, this.capHeight);
        } else {
            this.ctx.fillRect(i * xoffset, cheight - value, this.meterWidth, this.capHeight);
            capYPositionArray[i] = value;
        };
        this.ctx.fillStyle = this.gradient; 
        this.ctx.fillRect(i * xoffset /*meterWidth+gap*/ , cheight - value + this.capHeight, this.meterWidth, cheight);
    }
    this.hillScore.shift();
    this.hillScore.push(this.hillScore[this.hillScore.length - 1]);
    setTimeout(() => {this.animationId = requestAnimationFrame(this.draw)}, this.updateInterval);
  }
 }

 class textReaderJS {
  constructor(text="this is a test", res=["800","400"], textId="textcanvas", parentId="main_body", defaultUI=true) {
    this.text = text;
    this.textId = textId;
    this.parentId = parentId;

    var textReaderHTML = "<div id='"+this.textId+"container' class='canvasContainer'><canvas id='"+this.textId+"' class='textreadercss' width='"+res[0]+"' height='"+res[1]+"'></canvas></div>"

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

    this.c = document.getElementById(this.textId);
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
    var uiHTML = "<div id='"+this.textId+"menu' class='textmenu'> \
    <textarea id='"+this.textId+"Textarea'>Breathe in, Breathe out, Breathe in, Breathe out...</textarea><br> \
    <button id='"+this.textId+"submittext' class='button'>Submit</button> \
    </div><button id='showhide' name='showhide' class='showhide'>Hide UI</button>";

    HEGwebAPI.appendFragment(uiHTML, this.parentId);

    document.getElementById(this.textId+'submittext').onclick = () => {
      this.text = document.getElementById(this.textId+'Textarea').value;
      this.textXPos = 0;
    }

    document.getElementById("showhide").onclick = () => {
      if(this.hidden == false) {
        this.hidden = true;
        document.getElementById("showhide").innerHTML = "Show UI";
        document.getElementById(this.textId+'menu').style.display = "none";
      }
      else{
        this.hidden = false;
        document.getElementById("showhide").innerHTML = "Hide UI";
        document.getElementById(this.textId+'menu').style.display = "";
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
)=====";
