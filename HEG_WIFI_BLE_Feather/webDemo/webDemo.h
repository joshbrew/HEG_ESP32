const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>

<style>
  #gtext {
    z-index: 999;
  }
  h1 {
    font-family: Arial, Helvetica, sans-serif;
  }
  .dummy {
      width:0;
      height:0;
      border:0; 
      border:none;
   }

   .dattable {
      position: relative;
      table-layout: fixed;
      font-family: Console, Lucida, monospace;
   }
   th {
      background-color: rgb(54, 54, 54);
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   td {
      background-color: rgb(54, 54, 54);
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
  .scoreth { color: honeydew; }
    canvas {
      display:inline;
    }
  .canvasContainer {
      width: 100%;
      margin-left: auto;
      margin-right: auto;
      text-align: center;
   }

  .webglcss {
    position: absolute;
    top: 520px;
    left: 0px;
    width: 100%;
    height: 28.57%;
   }

      /**modal styling**/
      .modal {
        position: fixed;
        left: 0;
        top: 0;
        width: 100%;
        height: 100%;
        z-index: 990;

        }

      .modal .overlay {
          position: absolute;
          left: 0;
          top: 0;
          width: 100%;
          height: 100%;
          z-index: 995;
          background: rgba(0, 0, 0, 0);
          
      }

      .modal .modal_content {
          z-index: 999;
          position: absolute;
         
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          max-height: 90%;
          overflow: auto;
          background: rgba(0, 78, 131, 0.637);
          color: white;
          padding: 20px;
          box-shadow: 0 1px 5px rgba(0,0,0,0.7);
          text-align: center;
          border-radius: 4px;
          width: 500px; /* This just a default width */
          transition: all 300ms ease-in-out;
      }

          .modal .modal_content > h2 {
              transition: all 300ms ease-in-out;
              color: white;
              font-size: 1em;
              font-weight: 200;
         
              text-align: center;
          }

          .modal .modal_content .buttons_wrapper {
               transition: all 300ms ease-in-out;
              padding: 20px;
          }

      .modal .close_modal {
          position: absolute;
          right: 10px;
          top: 10px;
          cursor: pointer;
          font-size: 18px;
          opacity: 0.5;
          background: none;
          border: none;
          transition: opacity 0.2s ease;
      }

          .modal .close_modal:hover {
              opacity: 0.9;
          }

          .modal2 {
            position: fixed;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            z-index: 990;

            }

      .modal2 .overlay2 {
          position: absolute;
          left: 0;
          top: 0;
          width: 100%;
          height: 100%;
          z-index: 995;
          background: rgba(0, 0, 0, 0);
          
      }

      .modal2 .modal_content2 {
          z-index: 999;
          position: absolute;
         
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          max-height: 90%;
          overflow: auto;
          background: rgba(0, 78, 131, 0.637);
          color: white;
          padding: 20px;
          box-shadow: 0 1px 5px rgba(0,0,0,0.7);
          text-align: center;
          border-radius: 4px;
          width: 700px;
          transition: all 300ms ease-in-out;
      }

          .modal2 .modal_content2 > h2 {
              transition: all 300ms ease-in-out;
              color: white;
              font-size: 1em;
              font-weight: 200;
         
              text-align: center;
          }

          .modal2 .modal_content2 .buttons_wrapper {
               transition: all 300ms ease-in-out;
              padding: 20px;
          }

      .modal2 .close_modal2 {
          position: absolute;
          right: 10px;
          top: 10px;
          cursor: pointer;
          font-size: 18px;
          opacity: 0.5;
          background: none;
          border: none;
          transition: opacity 0.2s ease;
      }

          .modal2 .close_modal2:hover {
              opacity: 0.9;
          }

          .modal3 {
            position: fixed;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            z-index: 990;

            }

      .modal3 .overlay3 {
          position: absolute;
          left: 0;
          top: 0;
          width: 100%;
          height: 100%;
          z-index: 995;
          background: rgba(0, 0, 0, 0);
          
      }

      .modal3 .modal_content3 {
          z-index: 999;
          position: absolute;
         
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          max-height: 90%;
          overflow: auto;
          background: rgba(0, 78, 131, 0.637);
          color: white;
          padding: 20px;
          box-shadow: 0 1px 5px rgba(0,0,0,0.7);
          text-align: center;
          border-radius: 4px;
          width: 500px; /* This just a default width */
          transition: all 300ms ease-in-out;
      }

          .modal3 .modal_content3 > h3 {
              transition: all 300ms ease-in-out;
              color: white;
              font-size: 1em;
              font-weight: 200;
         
              text-align: center;
          }

          .modal3 .modal_content3 .buttons_wrapper {
               transition: all 300ms ease-in-out;
              padding: 20px;
          }

      .modal3 .close_modal3 {
          position: absolute;
          right: 10px;
          top: 10px;
          cursor: pointer;
          font-size: 18px;
          opacity: 0.5;
          background: none;
          border: none;
          transition: opacity 0.2s ease;
      }

          .modal3 .close_modal3:hover {
              opacity: 0.9;
          }


  /**Toggle Switch**/
  .switch {
      position: absolute;
      right: 16px;
      top: 95px;
      display: inline-block;
      width: 90px;
      height: 34px;
      z-index: 990;
  }

      .switch input {
          display: none;
      }

  .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ca2222;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 34px;
  }

      .slider:before {
          position: absolute;
          content: "";
          height: 26px;
          width: 26px;
          left: 4px;
          bottom: 4px;
          background-color: white;
          -webkit-transition: .4s;
          transition: .4s;
          border-radius: 50%;
      }

  input:checked + .slider {
      background-color: #2ab934;
  }

  input:focus + .slider {
      box-shadow: 0 0 1px #2196F3;
  }

  input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -us-transform: translateX(26px);
      transform: translateX(55px);
  }

  /*------ ADDED CSS ---------*/
  .slider:after {
      content: 'Off';
      color: white;
      display: block;
      position: absolute;
      transform: translate(-50%,-50%);
      top: 50%;
      left: 50%;
      font-size: 10px;
      font-family: Verdana, sans-serif;
  }

  input:checked + .slider:after {
      content: 'ON';
  }


  * {
      box-sizing: border-box
  }

  /* Set height of body and the document to 100% */
  body, html {
  width:  100%;
  height: 100%;
  margin: 0;
      background-color: black;
  }

  /* Style tab links */
  
  .tablink {
      font-family: 'Arial', serif;
      font-weight: bold;
      color: white;
      float: left;
      border: 1px solid rgb(85, 85, 85);
      outline: none;
      cursor: pointer;
      padding: 10px;
      font-size: 1.3em;
      width: 33.33%;
      background-color: black;
  }

      .tablink:hover {
          background-color: rgba(0, 189, 157, 0.644);
      }


    .tabcontent {
        color: white;
        display: none;
        height: 100%;
        font-family: 'Ibarra Real Nova', serif;
    }

  .button {
      font-family: Console, Lucida, monospace;
      color: black;
      float: left;
      border: 1px solid gray;
      outline: none;
      cursor: pointer;
      padding: 14px 16px;
      font-size: 1.6em;
      width: 50%;
      background-color: white;
  }

      .button:hover {
          background-color: rgba(0, 189, 157, 0.644);
      }


  input[type=text] {
      width: 50%;
      font-size: 1.5em;
      padding: 14px 16px;
      border: none;
      background-color: rgb(221, 210, 210);
      color: black;
      font-family: 'Ibarra Real Nova', serif;
  }

  .header {
      width: 50%;
      color: white;
      font-size: .5em;
      text-align: left;
      font-family: 'Ibarra Real Nova', serif;
  }
</style>

<script src="HEGwebAPI.js"></script>
</head>
<body>
  <title>HEG Interface</title>
  <div class="header">
      <h1>HEG ALPHA Ver 0.0.2</h1>
  </div>
  <label class="switch"><input onchange="toggleHEG(this)" type="checkbox" id="togBtn"><div class="slider round"></div></label>


  <div id="tabContainer">
      <button class="tablink" id="modal_opener">Data</button>
      <button class="tablink" id="modal_opener2">Graph</button>
      <button class="tablink" id="modal_opener3">Feedback</button>
  </div>
  <div class="modal" style="display: none">
      <div class="overlay"></div>
      <div class="modal_content">
          <h2>Data Options</h2>
          <div id="dataBox"></div>
          <button title="Close" class="close_modal">
              <i class="fas fa-times"></i>
          </button>
      </div>
  </div>

  <div class="modal2" style="display: none">
      <div class="overlay2"></div>
      <div id="graphBox" class="modal_content2">
          <h2>Graph Options</h2>
          <button title="Close" class="close_modal2">
              <i class="fas fa-times"></i>
          </button>
      </div>
  </div>

  <div class="modal3" style="display: none">
      <div class="overlay3"></div>
      <div id = "visualBox" class="modal_content3">
          <h2>Feedback Options</h2>
          <button title="Close" class="close_modal3">
              <i class="fas fa-times"></i>
          </button>
      </div>
  </div>
  <div id="main_body"></div>






  <script>


    var btn = document.getElementById('modal_opener');
    var btn2 = document.getElementById('modal_opener2');
    var btn3 = document.getElementById('modal_opener3');
    
    var modal = document.querySelector('.modal');
    var modal2 = document.querySelector('.modal2')
    var modal3 = document.querySelector('.modal3')
    
    function attachModalListeners(modalElm) {
      modalElm.querySelector('.close_modal').addEventListener('click', toggleModal);
      modalElm.querySelector('.overlay').addEventListener('click', toggleModal);
    }
    
    function detachModalListeners(modalElm) {
      modalElm.querySelector('.close_modal').removeEventListener('click', toggleModal);
      modalElm.querySelector('.overlay').removeEventListener('click', toggleModal);
    }
    
    function toggleModal() {
      var currentState = modal.style.display;
    
      // If modal is visible, hide it. Else, display it.
      if (currentState === 'none') {
        modal.style.display = 'block';
        attachModalListeners(modal);
      } else {
        modal.style.display = 'none';
        detachModalListeners(modal);  
      }
    }
    
    btn.addEventListener('click', toggleModal);
    
    function attachModalListeners2(modalElm) {
      modalElm.querySelector('.close_modal2').addEventListener('click', toggleModal2);
      modalElm.querySelector('.overlay2').addEventListener('click', toggleModal2);
    }
    
    function detachModalListeners2(modalElm) {
      modalElm.querySelector('.close_modal2').removeEventListener('click', toggleModal2);
      modalElm.querySelector('.overlay2').removeEventListener('click', toggleModal2);
    }
    
    function toggleModal2() {
      var currentState = modal2.style.display;
    
      // If modal is visible, hide it. Else, display it.
      if (currentState === 'none') {
        modal2.style.display = 'block';
        attachModalListeners2(modal2);
      } else {
        modal2.style.display = 'none';
        detachModalListeners2(modal2);  
      }
    }
    
    btn2.addEventListener('click', toggleModal2);
    
    function attachModalListeners3(modalElm) {
      modalElm.querySelector('.close_modal3').addEventListener('click', toggleModal3);
      modalElm.querySelector('.overlay3').addEventListener('click', toggleModal3);
    }
    
    function detachModalListeners3(modalElm) {
      modalElm.querySelector('.close_modal3').removeEventListener('click', toggleModal3);
      modalElm.querySelector('.overlay3').removeEventListener('click', toggleModal3);
    }
    
    function toggleModal3() {
      var currentState = modal3.style.display;
    
      // If modal is visible, hide it. Else, display it.
      if (currentState === 'none') {
        modal3.style.display = 'block';
        attachModalListeners3(modal3);
      } else {
        modal3.style.display = 'none';
        detachModalListeners3(modal3);  
      }
    }
    
    btn3.addEventListener('click', toggleModal3);
    
    function toggleHEG(switchElement) {
  if (switchElement.checked) {
    document.getElementById('startbutton').click();
  } else {
    document.getElementById('stopbutton').click();
  }
}


</script>

<script> //Rough Draft scripts

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

HEGwebAPI.appendFragment(modeHTML,"visualBox");

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
    this.us.push(parseInt(this.csvDat[this.csvIndex][0]));
    this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
    this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
    this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
    this.ambient.push(parseInt(this.csvDat[this.csvIndex][4]));
    this.velAvg.push(parseFloat(this.csvDat[this.csvIndex][5]));
    this.accelAvg.push(parseFloat(this.csvDat[this.csvIndex][6]));
    parent.postMessage( s.ratio[s.ratio.length-1], "*");
    g.us = this.us[this.us.length - 1];
    if(this.us.length >= 2){
      if(this.ratio.length > 40){
        s.smaScore(s.ratio);
        g.ratio = s.slowSMA;
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
        g.graphY1.push(s.slowSMAarr[this.slowSMAarr.length - 1 - g.offset]);
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
  setTimeout(() => {this.replayCSV();},(this.us[this.csvIndex]-this.us[this.csvIndex-1])*0.001); //Call until end of index.
}

var handleEventData = (e) => { //REDO THESE ONES IN A GENERALIZED WAY
  console.log("event", e.data);
  if(document.getElementById("heg").innerHTML != e.data){  //on new output
    document.getElementById("heg").innerHTML = e.data; // Use stored variable for this instead to save memory
    if(e.data.includes("|")) {
      var dataArray = e.data.split("|");
      if(parseFloat(dataArray[3]) > 0){ // If ratio slope is within a normal range (in case of errors)
        s.us.push(parseInt(dataArray[0]));
        s.red.push(parseInt(dataArray[1]));
        s.ir.push(parseInt(dataArray[2]));
        s.ratio.push(parseFloat(dataArray[3]));
        s.ambient.push(parseInt(dataArray[4]));
        s.velAvg.push(parseFloat(dataArray[5]));
        s.accelAvg.push(parseFloat(dataArray[6]));
        parent.postMessage( s.ratio[s.ratio.length-1], "*");
        g.us = s.us[s.us.length - 1];
        //handle new data
        if(s.ratio.length-1 > 40){
          s.smaScore(s.ratio);
          g.ratio = s.slowSMA;
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
