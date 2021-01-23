// Custom Scripts and UI setup, feedback modules must be manually linked to session event data (you can mix and match or write your own easily) 
// Advanced Client scripts using external packages

 
// Initialize Session - undefined are default values
var s = new HEGwebAPI('',undefined,undefined,undefined,undefined,false); //HEGduino
//var s = new HEGwebAPI('',["us","Red","IR","Ambient","Ratio","HR","SPO2"],undefined,undefined,undefined,false); // Delobotomizer
//var s = new HEGwebAPI('',["us","lRed","lIR","lRatio","cRed","cIR","cRatio","rRed","rIR","rRatio"],undefined,undefined,undefined,false); //Statechanger
//var s = new HEGwebAPI('',["us","Ratio"],",",0,1,undefined,undefined,false); window.PEANUT = true; //Peanut (USB only)

// Detect that we are not using the default local hosting on the ESP32 so we can grab scripts
if((window.location.hostname !== '192.168.4.1') && (window.location.hostname !== 'esp32.local')) {
    var useAdvanced = true; // Create a global flag to indicate we're capable of using advanced scripts.
  }
  
  // ------------------------------------------------------------------------
  // ------------------------------Tab Modal Code----------------------------
  // ------------------------------------------------------------------------
  
  var switchHTML = '<label class="switch"><input type="checkbox" id="togBtn"><div class="startslider round"></div></label>';
  
  var tabHTML = '<div id="tabContainer"> \
    <button class="tablink" id="modal_opener">Data</button> \
    <button class="tablink" id="modal_opener2">Graph</button> \
    <button class="tablink" id="modal_opener3">Feedback</button> \
    </div> \
    <div id="modal" class="modal" style="display: none"> \
    <div id="overlay" class="overlay"></div> \
      <div class="modal_content databoxmodal"> \
        <h2>Data Options</h2> \
        <div id="dataBox"></div> \
        <button title="Close" id="close_modal" class="close_modal"> \
            <i class="fas fa-times"></i> \
        </button> \
      </div> \
    </div> \
    <div id="modal2" class="modal" style="display: none"> \
    <div id="overlay2" class="overlay"></div> \
      <div id="graphBox" class="modal_content graphboxmodal"> \
        <h2>Graph Options</h2> \
        <button title="Close" id="close_modal2" class="close_modal"> \
            <i class="fas fa-times"></i> \
        </button> \
      </div> \
    </div> \
    <div id="modal3" class="modal" style="display: none"> \
    <div id="overlay3" class="overlay"></div> \
      <div id = "visualBox" class="modal_content feedbackboxmodal"> \
        <h2>Feedback Options</h2> \
        <button title="Close" id="close_modal3" class="close_modal"> \
            <i class="fas fa-times"></i> \
        </button> \
      </div> \
    </div>';
  
  HEGwebAPI.appendFragment(switchHTML, "main_body");
  HEGwebAPI.appendFragment(tabHTML, "main_body");
  
  function attachModalListeners(modalElm, closemodal, overlay) {
    document.getElementById(closemodal).onclick = function() {toggleModal(modalElm, closemodal, overlay)};
    document.getElementById(overlay).onclick = function() {toggleModal(modalElm, closemodal, overlay)};
  }
  
  function detachModalListeners(modalElm, closemodal, overlay) {
    document.getElementById(closemodal).onclick = function() {toggleModal(modalElm, closemodal, overlay)};
    document.getElementById(overlay).onclick = function() {toggleModal(modalelm, closemodal, overlay)};
  }
  
  function toggleModal(modalElm, closemodal, overlay) {
    var currentState = modalElm.style.display;
    // If modal is visible, hide it. Else, display it.
    if (currentState === 'none') {
      modalElm.style.display = 'block';
      attachModalListeners(modalElm, closemodal, overlay);
    } else {
      modalElm.style.display = 'none';  
      detachModalListeners(modalElm, closemodal, overlay);
    }
  }
  
  var modal = document.getElementById('modal');
  var modal2 = document.getElementById('modal2');
  var modal3 = document.getElementById('modal3');
  
  document.getElementById('modal_opener').onclick = function() {toggleModal(modal,'close_modal','overlay')};
  document.getElementById('modal_opener2').onclick = function() {toggleModal(modal2,'close_modal2','overlay2')};
  document.getElementById('modal_opener3').onclick = function() {toggleModal(modal3,'close_modal3','overlay3')};
  
  function toggleHEG(switchElement) {
    if (switchElement.checked) {
      document.getElementById('startbutton').click();
    } else {
      document.getElementById('stopbutton').click();
    }
  }
  document.getElementById("togBtn").onchange = function(){toggleHEG(document.getElementById("togBtn"))};
  
  // ------------------------------------------------------------------------
  // ------------------------------------------------------------------------
  // ------------------------------------------------------------------------
 
  // Initialize Graph
  var g = new graphJS(1155,[255,100,80,1],1.0,[1400,600], "main_body", "g", false); // This could be swapped for a superior graphing package
  
  s.createUI("dataBox");
  g.createUI("graphBox")

  // Feedback
  var c = new circleJS(); // Default animation initialize
  var v = null;
  var a = null;
  var h = null;
  var txt = null;
  
  
  var modeHTML = '<div class="menudiv" id="menudiv"> \
    Modes:<br> \
    <button class="button" id="canvasmode">Circle</button> \
    <button class="button" id="videomode">Video</button> \
    <button class="button" id="audiomode">Audio</button><br> \
    <button class="button" id="hillmode">Hill Climb</button> \
    <button class="button" id="txtmode">Text Reader</button> \
    </div>';

  HEGwebAPI.appendFragment(modeHTML,"visualBox");
  
  document.getElementById("canvasmode").onclick = function() {
    if(c == null){
      deInitMode();
      c = new circleJS();
    }
  }
  
  document.getElementById("videomode").onclick = function() {
    if(v == null){
      deInitMode();
      v = new videoJS();
    }
  }
  
  document.getElementById("audiomode").onclick = function() {
    if(a == null){
      deInitMode();
      a = new audioJS();
    }
  }
  
  document.getElementById("hillmode").onclick = function() {
    if(h == null){
      deInitMode();
      h = new hillJS();
    }
  }
  
  document.getElementById("txtmode").onclick = function() {
    if(txt == null){
      deInitMode();
      txt = new textReaderJS();
    }
  }
  
  // ------------------------------------------------------------------------
  // ------------------------------------------------------------------------
  // ------------------------------------------------------------------------

  if(useAdvanced) { // Setup advanced scripts now that the default app is ready.
    var link = document.createElement("script");
    link.src = "js/threeApp.js"; // Can set this to be a nonlocal link like from cloudflare or a special script with a custom app
    document.head.appendChild(link); // Append script
  
    var threeApp = null;
  
    var threeModeHTML = '<button class="button" id="threemode">ThreeJS</button>';
    HEGwebAPI.appendFragment(threeModeHTML,"menudiv");
  
    document.getElementById("threemode").onclick = function() {
      if(threeApp == null) {
          deInitMode();
          threeApp = new ThreeGlobe();
      }
    }
  
    // var link3 = document.createElement("script");
    // link3.src = "https://cdnjs.cloudflare.com/ajax/libs/viewerjs/1.5.0/viewer.min.js"; // PDF Viewer JS (text scrolling experiment)
    // document.head.appendChild(link3);
  }
  
  // ------------------------------------------------------------------------
  // ------------------------------------------------------------------------
  
  // Customize session functions
  s.handleScore = function() {
    g.clock = this.clock[this.clock.length - 1] - this.startTime;
    if(this.ratio.length > 40){
      if(g.sampleRate == null) {
        if(s.useMs == true){ g.sampleRate = (this.clock[this.clock.length - 1] - this.clock[0]) * 0.001 / this.clock.length; } // Seconds / Sample
        else{ g.sampleRate = (this.clock[this.clock.length - 1] - this.clock[0]) * 0.000001 / this.clock.length; } // Seconds / Sample
      }
      this.smaScore(this.ratio);
      var score = this.smaSlope*this.sensitivity.value*0.01;
      if(c != null){
        c.onData(score);
      }
      if (v != null) {
        v.onData(score);
      }
      if(a != null) {
        a.onData(score);
      }
      if(h != null) {
        h.onData(score);
      }
      if(txt != null) {
        txt.onData(score);
      }
      if(useAdvanced) { // Score handling for advanced scripts
        if(threeApp != null) {
          threeApp.onData(score);
        }
      }
      this.scoreArr.push(this.scoreArr[this.scoreArr.length - 1] + score);
      g.ratio = this.slowSMA;
      g.score = this.scoreArr[this.scoreArr.length - 1];
      g.graphY1.shift();
      g.graphY1.push(this.scoreArr[this.scoreArr.length - 1 - g.xoffset]);
      g.graphY2.shift();
      g.graphY2.push(this.slowSMAarr[this.slowSMAarr.length - 1 - g.xoffset]);
    }
    else {
      // this.smaSlope = this.scoreArr[this.scoreArr.length - 1];
      // g.graphY1.shift();
      // g.graphY1.push(this.smaSlope);
      // this.scoreArr.push(this.smaSlope);
    }
  }
  
  s.endOfEvent = function() {
    if(g.xoffsetSlider.max < this.scoreArr.length){
      if(this.scoreArr.length % 20 == 0) { 
        g.xoffsetSlider.max = this.scoreArr.length - 3; // Need 2 vertices minimum
      }
    }
  }
  
  function deInitMode(){
    if(v != null){
      HEGwebAPI.removeParent(v.vidapiId);  
      HEGwebAPI.removeParent(v.vidContainerId);
      v.deInit();
      v = null;
    }
    if(a != null){
      a.stopAudio();
      a.endAudio(a);
      HEGwebAPI.removeParent(a.audioId);
      HEGwebAPI.removeParent(a.audmenuId);
      a = null;
    }
    if(c != null){
      c.deInit();
      HEGwebAPI.removeParentParent(c.canvasId);
      HEGwebAPI.removeParent(c.canvasmenuId);
      c = null;
    }
    if(h != null){
      h.deInit();
      HEGwebAPI.removeParentParent(h.canvasId);
      HEGwebAPI.removeParent(h.canvasmenuId)
      h = null;
    }
    if(txt != null){
      txt.deInit();
      HEGwebAPI.removeParentParent(txt.canvasId);
      HEGwebAPI.removeParent(txt.canvasmenuId);
      txt = null;
    }
    if(useAdvanced) { // Score handling for advanced scripts
      if(threeApp != null) {
        threeApp.destroyThreeApp();
        threeApp = null;
      }
    }
  }
  

  document.getElementById("resetSession").onclick = () => { // Override default function
    s.resetVars();
    g.resetVars();
  
    if(c != null) {
      deInitMode();
      c = new circleJS();
    }
    if(v != null) {
      deInitMode();
      v = new videoJS();
    }
    if(a != null) {
      deInitMode();
      a = new audioJS();
    }
    if(h != null) {
      deInitMode();
      h = new hillJS();
    }
    if(txt != null) {
      deInitMode();
      txt = new textReaderJS();
    }
    if(useAdvanced){
      if(threeApp != null) {
        deInitMode();
        threeApp = new ThreeGlobe();
      }
    }
  }
  
  
  g.xoffsetSlider.onchange = () => {
     if(g.xoffsetSlider.value > s.scoreArr.length) {
       g.xoffsetSlider.value = s.scoreArr.length - 1;
     }
     g.xoffset = g.xoffsetSlider.value;
     
     if(s.scoreArr.length > g.graphY1.length){ // more data than graph size, so just grab a slice of the graph
      var endIndex = s.scoreArr.length - g.xoffset - 1;
      g.graphY1 = s.scoreArr.slice(endIndex - g.graphY1.length, endIndex); // FIX 
      g.graphY2 = s.ratio.slice(endIndex -g.graphY2.length, endIndex);
     }
     else if (s.scoreArr.length < g.graphY1.length) { // less data than graph size, generate zeroes with data from 0 to offset
      var scoreslice = s.scoreArr.slice(0,s.scoreArr.length - 1 - g.xoffset);
      var ratioslice = s.ratio.slice(0,s.ratio.length - 1 - g.xoffset);
      if(g.graphY1.length == scoreslice){
        g.graphY1 = scoreslice;
        g.graphY2 = ratioslice;
      }
      else{
        g.graphY1 = [...Array(g.VERTEX_LENGTH - scoreslice.length).fill(0), ...scoreslice];
        g.graphY2 = [...Array(g.VERTEX_LENGTH - ratioslice.length).fill(0), ...ratioslice];
      }
     }
  }
  
  g.xscaleSlider.onchange = () => {
    len = g.graphY1.length;
    if(g.xscaleSlider.value < len) { // Remove from front.
      for(var i = 0; i < len - g.xscaleSlider.value; i++){
        g.graphY1.shift();
        g.graphY2.shift();
      }
    }
    if(g.xscaleSlider.value > len) { // Add to front
      for(var i = 0; i < g.xscaleSlider.value - len; i++){
        if(i+len+g.xoffset <= s.scoreArr.length){
          g.graphY1.unshift(s.scoreArr[s.scoreArr.length - ((len+i) + g.xoffset)]);
          g.graphY2.unshift(s.ratio[s.ratio.length - ((len+i) + g.xoffset)]);
        } 
        else{
          g.graphY1.unshift(0);
          g.graphY2.unshift(0);
        }
      }
    }
    g.VERTEX_LENGTH = g.graphY1.length;
  }
  
  document.getElementById("xscalebutton").onclick = () => {
    var len = g.graphY1.length;
    g.xscaleSlider.value = g.nPoints;
    if(g.xscaleSlider.value < len) { // Remove from front.
      for(var i = 0; i < len - g.xscaleSlider.value; i++){
        g.graphY1.shift();
        g.graphY2.shift();
      }
    }
    if(g.xscaleSlider.value > len) { // Add to front
      for(var i = 0; i < g.xscaleSlider.value - len; i++){
        if(g.xscaleSlider.value < s.scoreArr.length){
          g.graphY1.unshift(s.scoreArr[s.scoreArr.length - 1 - ((g.graphY1.length+i) + g.xoffset)]);
          g.graphY2.unshift(s.ratio[s.ratio.length - 1 - ((g.graphY2.length+i) + g.xoffset)]);
        } 
        else{
          g.graphY1.unshift(0);
          g.graphY2.unshift(0);
        }
      }
    }
    g.VERTEX_LENGTH = g.xscaleSlider.value;
  }
  
  document.getElementById("xscaletd").style.display = "none"; // Gonna leave this out for now.
  document.getElementById("xscalebutton").style.display = "none"; // Gonna leave this out for now.
  document.getElementById("xoffsettd").style.display = "none"; // Gonna leave this out for now.
  document.getElementById("xoffsetbutton").style.display = "none"; // Gonna leave this out for now.
  
  // ------------------------------------------------------------------------
  // ----------------------------ToolTips------------------------------------
  // ------------------------------------------------------------------------
  
  function makeTooltip(parentId, position=[100,100], text="Tooltip text") {
    var tooltipHTML = "<div id='"+parentId+"_tooltip' class='tooltip'></div>";
    HEGwebAPI.appendFragment(tooltipHTML, parentId);
    var tooltip = document.getElementById(parentId+"_tooltip");
    tooltip.innerHTML = text;
    var thisParent = document.getElementById(parentId);
    
    // console.log(tooltip);
    tooltip.style.left = position[0] + "px";
    tooltip.style.top = position[1] + "px";
    // console.log(tooltip.style.left);
    // console.log(tooltip.style.top);
    tooltip.style.display = "none";
    
    thisParent.onmouseover = () => {
      tooltip.style.display = "";
    }
    thisParent.onmouseleave = () => {
      tooltip.style.display = "none";
    }
  }
  
  // Menu tabs
  makeTooltip("modal_opener",[150,70],"Session controls, timestamped annotating, save & replay data, host-changing, and an output table");
  makeTooltip("modal_opener2",[10,70],"Graph perspective controls");
  makeTooltip("modal_opener3",[10,90],"Various feedback modes. Change the scoring sensitivity settings in the Data menu to change the reactiveness.");
  
  // Data options
  makeTooltip("commandrow",[10,100],"See documentation for a command list, not all work over WiFi.");
  makeTooltip("sensitivityrow",[300,250],"Controls how reactive the feedback is to ratio changes.");
  makeTooltip("timerow",[10,340],"Press 'Get Time' at any given time in your session then write a note and press 'Annotate' and it will be added to the CSV when you click 'Save CSV'");
  makeTooltip("csvrow",[10,520],"Name your CSV and save it after your session is complete to have a record of your data. Automatically stores in your default Downloads folder.")
  makeTooltip("replaycsv",[10,575],"Replay saved CSV files (in our format) as if they are live sessions. For charting see our Data Charter applet on our repo or website.")
  makeTooltip("hostrow",[10,575],"Connect to your device's WiFi IP manually from here to access the Event Source, it is automatically set when accessing this interface on the device. USB serial connectivity requires our free Chrome Extension. Alternate Bluetooth LE mode or BT serial connectivity also available.")
  
  // Graph options
  makeTooltip("xoffsettd",[10,40],"Scroll back and forth through your data if it is longer than the graph.");
  makeTooltip("xscaletd",[10,80],"Shrink or grow the graph on the x-axis");
  makeTooltip("yoffsettd",[10,110],"Scroll up or down on the y-axis of the graph.");
  makeTooltip("yscaletd",[10,160],"Shrink or grow the graph on the y-axis.");
  makeTooltip("autoscaletd",[10,220],"Uncheck to manually scale the graph on the y-axis.");
  
  // Feedback options
  makeTooltip("canvasmode",[10,10],"Grow the circle and keep it big!");
  makeTooltip("audiomode",[10,10],"Keep the song volume up or at a sweet spot! Use MP3 files.");
  makeTooltip("txtmode",[10,10],"Scroll the text to the right to keep reading!");
  makeTooltip("videomode",[300,10],"Control a video through various means! Use MP4 files.");
  makeTooltip("hillmode",[300,10],"Climb the mountain!");
  
  if(useAdvanced == true){
    makeTooltip("threemode",[300,10],"Turn the Earth! More coming!");
  }

//------------------------------------------------------------------------
//------------------------Bluetooth LE Additions--------------------------
//------------------------------------------------------------------------

var ble = new bleUtils(false)
ble.onNotificationCallback = (e) => {

  //console.log(e.target.readValue());
  var line = ble.decoder.decode(e.target.value);
  if(ble.android == true){
    line = Date.now()+"|"+line;
  }
  
  //pass to data handler
  if(line.split(s.delimiter).length == s.header.length) { //Most likely a data line based on our stream header formatting
    s.handleEventData(line); 
    //console.log("Passing BLE Data...", Date.now())
  }
  else {
    console.log("BLE MSG: ", line);
  }
}

ble.onReadAsyncCallback = (data) => {

  var line = data;
  if(ble.android == true){
    line = Date.now()+"|"+line;
  }

   //pass to data handler
   if(line.split(s.delimiter).length == s.header.length) { //Most likely a data line based on our stream header formatting
    s.handleEventData(line); 
    //console.log("Passing BLE Data...", Date.now())
  }
  else {
    console.log("BLE MSG: ", line);
  }
}

ble.onConnectedCallback = () => {
  
  s.removeEventListeners();
  if(ble.android === true){
    s.header=["ms","Red","IR","Ratio"];
    s.updateStreamHeader();
    s.useMs = true;
    g.useMs = true;
  }
  document.getElementById("startbutton").onclick = () => {
    ble.sendMessage('t');
  }
  document.getElementById("stopbutton").onclick = () => {
    ble.sendMessage('f');
  }
  document.getElementById("sendbutton").onclick = () => {
    ble.sendMessage(document.getElementById('command').value);
  }
}

ble.onDisconnected = () => {
  if(ble.android == true){
    s.header=["us","Red","IR","Ratio","Ambient","drdt","ddrdt"]; //try to reset the header in case of reconnecting through a different protocol
    s.updateStreamHeader();
    s.useMs = false;
    g.useMs = false;
  }
  console.log("BLE Device disconnected!");
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//----------------------Chrome Extension Additions------------------------
//------------------------------------------------------------------------
/*
var serialHTML = '<div id="serialContainer" class="serialContainer"><h3>Serial Devices:</h3><div id="serialmenu" class="serialmenu"></div></div>';
HEGwebAPI.appendFragment(serialHTML,"main_body");

var serialMonitor = new chromeSerial();
serialMonitor.finalCallback = () => { //Set this so USB devices bind to the interface once connected.
  s.removeEventListeners();

  document.getElementById("startbutton").onclick = () => {
    serialMonitor.sendMessage('t');
  }
  document.getElementById("stopbutton").onclick = () => {
    serialMonitor.sendMessage('f');
  }
  document.getElementById("sendbutton").onclick = () => {
    serialMonitor.sendMessage(document.getElementById('command').value);
  }

  if(window.PEANUT){
    serialMonitor.sendMessage("protocol 3");
    serialMonitor.onReadLine = (line) => {
      console.log(line);
      //var timeus = Date.now() * 1000;
      //s.handleEventData(timeus+","+line);
    }
  }
  else{
    serialMonitor.onReadLine = (line) => { //Connect the serial monitor data to the session handler
      //pass to data handler
      if(line.split(s.delimiter).length == s.header.length) { //Most likely a data line based on our stream header formatting
        s.handleEventData(line); 
        //console.log("Passing Serial Data...", Date.now())
      }
      else{
        console.log("RECEIVED: ", line);
      }
    }
  }
}

makeTooltip("serialContainer",[-220,10],"Click 'Get' to get available Serial devices and 'Set' to pair it with the interface. Right click and press 'Inspect' to see debug output in the Console");
*/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

