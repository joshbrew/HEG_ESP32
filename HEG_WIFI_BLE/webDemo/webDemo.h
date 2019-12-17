const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<link rel="stylesheet" type="text/css" href="webDemoCSS.css">
<script src="HEGwebAPI.js"></script>
</head>
<body>
  <div id="main_body"></div>
<script> //Rough scripts
var s = new HEGwebAPI();
var g = new graphJS("main_body","g",1500,[255,100,80,1]);
var c = new circleJS("main_body","canvas1"); //Default animation
var v = null;
var a = null;
var h = null;

var useGraph = true;
var useCanvas = true;
var useVideo = false;
var useAudio = false;
var useHills = false;

var modeHTML = '<div class="menudiv" id="menudiv"> \
  Modes:<br> \
  <button class="button cvbutton" id="canvasmode">Canvas</button><button class="button vdbutton" id="videomode">Video</button><button class="button aubutton" id="audiomode">Audio</button><br> \
  <button class="button lfbutton" id="hillmode">Hill Climb</button> \
</div>';

HEGwebAPI.appendFragment(modeHTML,"main_body");

function deInitMode(){
  if(useVideo == true){
    var thisNode = document.getElementById(v.vidapiId);
    thisNode.parentNode.parentNode.removeChild(thisNode.parentNode);
    thisNode = document.getElementById(v.vidContainerId);    
    thisNode.parentNode.parentNode.removeChild(thisNode.parentNode);
    v.deInit();
    useVideo = false;
    v = null;
  }
  if(useAudio == true){
    a.stopAudio();
    a.endAudio(a);
    var thisNode = document.getElementById(a.audioId);
    thisNode.parentNode.parentNode.removeChild(thisNode.parentNode);
    thisNode = document.getElementById(a.audmenuId);
    thisNode.parentNode.parentNode.removeChild(thisNode.parentNode);
    useAudio = false;
    a = null;
  }
  if(useCanvas == true){
    c.deInit();
    c.c.parentNode.parentNode.parentNode.removeChild(c.c.parentNode.parentNode);
    useCanvas = false;
    c = null;
  }
  if(useHills == true){
    h.deInit();
    h.c.parentNode.parentNode.parentNode.removeChild(h.c.parentNode.parentNode);
    h.menu.parentNode.removeChild(h.menu);
    useHills = false;
    h = null;
  }
}

document.getElementById("canvasmode").onclick = function() {
  if(useCanvas == false){
    deInitMode();
    c = new circleJS("main_body","canvas1");
    useCanvas = true;
  }
}

document.getElementById("videomode").onclick = function() {
  if(useVideo == false){
    deInitMode();
    v = new videoJS("main_body");
    useVideo = true
  }
}

document.getElementById("audiomode").onclick = function() {
  if(useAudio == false){
    deInitMode();
    a = new audioJS("main_body");
    useAudio = true; 
  }
}

document.getElementById("hillmode").onclick = function() {
  if(useHills == false){
    deInitMode();
    h = new hillJS("main_body");
    useHills = true;
  }
}

g.xoffsetSlider.onchange = () => {
   if(g.xoffsetSlider.value > s.scoreArr.length) {
     g.xoffsetSlider.value = s.scoreArr.length - 1;
   }
   g.offset = g.xoffsetSlider.value;
   
   if(s.scoreArr.length > g.graphY1.length){ //more data than graph size, so just grab a slice of the graph
    var endIndex = s.scoreArr.length - g.offset;
    g.graphY1 = s.scoreArr.slice(endIndex - g.graphY1.length, endIndex); // FIX 
   }
   else if (s.scoreArr.length < g.graphY1.length) { //less data than graph size, generate zeroes with data from 0 to offset
    var scoreslice = s.scoreArr.slice(0,s.scoreArr.length - g.offset);
    if(g.graphY1.length == scoreslice){
      g.graphY1 = scoreslice;
    }
    else{
      g.graphY1 = [...Array(g.VERTEX_LENGTH - scoreslice.length).fill(0), ...scoreslice];
    }
   }
}

g.xscaleSlider.onchange = () => {
  len = g.graphY1.length;
  if(g.xscaleSlider.value < len) { // Remove from front.
    for(var i = 0; i < len - g.xscaleSlider.value; i++){
      g.graphY1.shift();
    }
  }
  if(g.xscaleSlider.value > len) { // Add to front
    for(var i = 0; i < g.xscaleSlider.value - len; i++){
      if(i+len <= s.scoreArr.length){
        g.graphY1.unshift(s.scoreArr[s.scoreArr.length - (len+i) + g.offset]);
      } 
      else{
        g.graphY1.unshift(0);
      }
    }
  }
  g.VERTEX_LENGTH = g.graphY1.length;
}

document.getElementById("xscalebutton").onclick = () => {
  var len = g.graphY1.length;
  g.xscaleSlider.value = 1000;
  if(g.xscaleSlider.value < len) { // Remove from front.
    for(var i = 0; i < len - g.xscaleSlider.value; i++){
      g.graphY1.shift();
    }
  }
  if(g.xscaleSlider.value > len) { // Add to front
    for(var i = 0; i < g.xscaleSlider.value - len; i++){
      if(g.xscaleSlider.value < s.scoreArr.length){
        g.graphY1.unshift(s.scoreArr[s.scoreArr.length - 1 - g.graphY1.length + g.offset]);
      } 
      else{
        g.graphY1.unshift(0);
      }
    }
  }
  g.VERTEX_LENGTH = g.xscaleSlider.value;
}

s.replayCSV = function() { //REDO IN GENERALIZED FORMAT
  if(this.csvIndex < 2){
    if(useVideo == true){
      v.playRate = 1;
      v.alpha = 0;
    }
    if(useCanvas == true){
      c.angle = 1.57;
    }
    this.ms.push(parseInt(this.csvDat[this.csvIndex][0]));
    this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
    this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
    this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
    this.adcAvg.push(parseInt(this.csvDat[this.csvIndex][4]));
    this.ratioSlope.push(parseFloat(this.csvDat[this.csvIndex][5]));
    this.AI.push(parseFloat(this.csvDat[this.csvIndex][6]));
  }
  this.csvIndex++;
  if(this.csvIndex < this.csvDat.length - 1){
    this.ms.push(parseInt(this.csvDat[this.csvIndex][0]));
    this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
    this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
    this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
    this.adcAvg.push(parseInt(this.csvDat[this.csvIndex][4]));
    this.ratioSlope.push(parseFloat(this.csvDat[this.csvIndex][5]));
    this.AI.push(parseFloat(this.csvDat[this.csvIndex][6]));
    parent.postMessage( s.ratio[s.ratio.length-1], "*");
    g.ms = this.ms;
    if(this.ms.length >= 2){
      if(this.ratio.length > 40){
        s.smaScore(s.ratio);
        var score = this.smaSlope*this.sensitivity.value*0.01;
        if(useCanvas == true){
          c.onData(score);
        }
        if (useVideo == true) {
          v.onData(score);
        }
        if(useAudio == true) {
          a.onData(score);
        }
        if(useHills == true) {
          h.onData(score*10);
        }
        s.scoreArr.push(s.scoreArr[s.scoreArr.length - 1] + score);
        g.graphY1.shift();
        g.graphY1.push(s.scoreArr[this.scoreArr.length - 1 - g.offset]);
      }
      else {
        //this.smaSlope = this.scoreArr[this.scoreArr.length - 1];
        //g.graphY1.shift();
        //g.graphY1.push(this.smaSlope);
        //this.scoreArr.push(this.smaSlope);
      }
      this.updateTable();
    }
  }
  else {
    this.replay = false;
    this.csvDat = [];
    this.csvIndex = 0;
  }
  if(g.xoffsetSlider.max < this.scoreArr.length){
    if(this.scoreArr.length % 20 == 0) { //only update every 20 samples
      g.xoffsetSlider.max = this.scoreArr.length - 1;
    }
  }
  setTimeout(() => {this.replayCSV();},(this.ms[this.csvIndex]-this.ms[this.csvIndex-1])); //Call until end of index.
}

var handleEventData = (e) => { //REDO THESE ONES IN A GENERALIZED WAY
  console.log("event", e.data);
  if(document.getElementById("heg").innerHTML != e.data){  //on new output
    document.getElementById("heg").innerHTML = e.data; // Use stored variable for this instead to save memory
    if(e.data.includes("|")) {
      var dataArray = e.data.split("|");
      if(parseFloat(dataArray[3]) > 0){ // If ratio slope is within a normal range (in case of errors)
        s.ms.push(parseInt(dataArray[0]));
        s.red.push(parseInt(dataArray[1]));
        s.ir.push(parseInt(dataArray[2]));
        s.ratio.push(parseFloat(dataArray[3]));
        s.adcAvg.push(parseInt(dataArray[4]));
        s.ratioSlope.push(parseFloat(dataArray[5]));
        s.AI.push(parseFloat(dataArray[6]));
        parent.postMessage( s.ratio[s.ratio.length-1], "*");
        g.ms = s.ms;
        //handle new data
        if(s.ratio.length-1 > 40){
          s.smaScore(s.ratio);
          var score = s.smaSlope*s.sensitivity.value*0.01;
          if(useCanvas == true){
            c.onData(score);
          }
          if (useVideo == true) {
            v.onData(score);
          }
          if(useAudio == true) {
            a.onData(score);
          }
          if(useHills == true) {
            h.onData(score*10);
          }
          s.scoreArr.push(s.scoreArr[s.scoreArr.length - 1] + score);
          g.graphY1.shift();
          g.graphY1.push(s.scoreArr[s.scoreArr.length - 1 - g.offset]);
        }
        s.updateTable(); 
      } 
    }
  }
  //handle if data not changed
  else if (s.replay == false) {
    //s.smaSlope = s.scoreArr[s.scoreArr.length - 1];
    //g.graphY1.shift();
    //g.graphY1.push(s.scoreArr[s.scoreArr.length - 1 - g.offset]);
    //s.scoreArr.push(s.smaSlope);
  }
  if(g.xoffsetSlider.max < s.scoreArr.length){
    if(s.scoreArr.length % 20 == 0) { 
      g.xoffsetSlider.max = s.scoreArr.length - 1;
    }
  }
}

s.handleData = (e) => { //Set event data handler for this page.
handleEventData(e);
}
</script>
</body>
</html>
)=====";
