const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body { background-color: #707070; font-family: Arial, Helvetica, sans-serif; }
  msgDiv { color: white; }
  eventDiv { color: white; }
  input[type=text]{
    border: 2px solid red;
    border-radius: 4px;
    height: 30px;
    padding: 2px;
    font-size: 16px;
  }
  .button {
    border: none;
    border-radius: 12px;
    color: white;
    padding: 15px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    cursor: pointer;
   }
   .dattable {
      position: relative;
      width: 75%;
      min-width: 700px;
      table-layout: fixed;
   }
   th {
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   td {
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   .scoreth { color: honeydew; }
   .hegapi {
      position: absolute;
      width: 275px;
      height: 300px;
      top: 130px;
      left: 0%;
   }
   .startbutton { background-color: #4CAF50; position: absolute; left: 0%; }
   .stopbutton { background-color: #FF0000; position: absolute; left: 120px; }
   .sendcommand{ position: absolute; top: 70px;   }
   .sendbutton{ background-color: #0000FF; }
   .saveLoadBar{ position: absolute; top: 240px; }
   .saveLoadButtons{ background-color: teal; }
   .sensBar { position: absolute; top: 150px; color: white; }
   .label { padding: 4px; color: white; }
   .canvascss {
      position: absolute;
      top:130px;
      left:300px;
   }
   .webglcss {
     position: absolute;
     top: 550px;
     left: 10px;
     width: 75%;
     height: 200px;
     min-width: 400px;
   }
   .scale{
     position: absolute;
     bottom: 0px;
     right: 50px;
     color: white;
   }
   .menudiv{
    position: absolute;
    right: 0px;
    top: 130px;
   }
   .cvbutton{
    background-color: chartreuse;
   }
   .vdbutton{
    background-color: royalblue;
   }
   .aubutton{
    background-color: tomato;
   }
   .dummy {
      position: absolute;
      top: 130px;
      left:325px;
      width:0;
      height:0;
      border:0; 
      border:none;
   }
</style>
<script src="HEGwebAPI.js"></script>
</head>
<body id="main_body">
    <script>
      var s = new HEGwebAPI();

      var v = null;
      var c = new circleJS("main_body","canvas1");
      var g = new graphJS("main_body","g",1500,[255,100,80,1]);

      var useCanvas = true;
      var useVideo = false;
      var useAudio = false;
      var useGraph = true;

      var modeHTML = '<div class="menudiv" id="menudiv"> \
        Modes:<br> \
        <button class="button cvbutton" id="canvasmode">Canvas</button><button class="button vdbutton" id="videomode">Video</button><button class="button aubutton" id="audiomode">Audio</button> \
      </div>';

      HEGwebAPI.appendFragment(modeHTML,"main_body");

      document.getElementById("canvasmode").onclick = function() {
        if(useVideo == true){
          var thisNode = document.getElementById(v.vidId);
          thisNode.parentNode.removeChild(thisNode.parentNode);
        }
        v.deInit();
        v = null;
        c = new circleJS("main_body","canvas1");
        useVideo = false;
      }

      document.getElementById("videomode").onclick = function() {
        if(useCanvas == true){
          c.c.parentNode.parentNode.removeChild(c.c.parentNode);
        }
        c = null;
        v = new videoJS("main_body");
        useCanvas = false;
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
          this.ms.push(parseInt(this.csvDat[this.csvIndex][0]));
          this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
          this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
          this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
          this.smallSavLay.push(parseFloat(this.csvDat[this.csvIndex][4]));
          this.largeSavLay.push(parseFloat(this.csvDat[this.csvIndex][5]));
          this.adcAvg.push(parseInt(this.csvDat[this.csvIndex][6]));
          this.ratioSlope.push(parseFloat(this.csvDat[this.csvIndex][7]));
          this.AI.push(parseFloat(this.csvDat[this.csvIndex][8]));
        }
        this.csvIndex++;
        if(this.csvIndex < this.csvDat.length - 1){
          this.ms.push(parseInt(this.csvDat[this.csvIndex][0]));
          this.red.push(parseInt(this.csvDat[this.csvIndex][1]));
          this.ir.push(parseInt(this.csvDat[this.csvIndex][2]));
          this.ratio.push(parseFloat(this.csvDat[this.csvIndex][3]));
          this.smallSavLay.push(parseFloat(this.csvDat[this.csvIndex][4]));
          this.largeSavLay.push(parseFloat(this.csvDat[this.csvIndex][5]));
          this.adcAvg.push(parseInt(this.csvDat[this.csvIndex][6]));
          this.ratioSlope.push(parseFloat(this.csvDat[this.csvIndex][7]));
          this.AI.push(parseFloat(this.csvDat[this.csvIndex][8]));
          g.ms = this.ms;
          if(this.ms.length >= 2){
            if(this.largeSavLay.length > 40){
              this.smaScore();
              if(useCanvas == true){
                c.angleChange = this.smaSlope*this.sensitivity.value*0.01;
              }
              else if (useVideo == true) {
                v.onData(this.smaSlope*this.sensitivity.value*0.01);
              }
              g.graphY1.shift();
              g.graphY1.push(s.scoreArr[this.scoreArr.length - 1 - g.offset]);
            }
            else {
              this.smaSlope = 0;
              if(useCanvas == true){
                c.angleChange = 0;
              }
              g.graphY1.shift();
              g.graphY1.push(0);
              this.scoreArr.push(0);
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
        console.log("myevent", e.data);
        if(document.getElementById("heg").innerHTML != e.data){  //on new output
          document.getElementById("heg").innerHTML = e.data;
          if(e.data.includes("|")) {
            var dataArray = e.data.split("|");
            s.ms.push(parseInt(dataArray[0]));
            s.red.push(parseInt(dataArray[1]));
            s.ir.push(parseInt(dataArray[2]));
            s.ratio.push(parseFloat(dataArray[3]));
            s.smallSavLay.push(parseFloat(dataArray[4]));
            s.largeSavLay.push(parseFloat(dataArray[5]));
            s.adcAvg.push(parseInt(dataArray[6]));
            s.ratioSlope.push(parseFloat(dataArray[7]));
            s.AI.push(parseFloat(dataArray[8]));
            g.ms = s.ms;
            //handle new data
            if(s.largeSavLay.length-1 > 40){
              s.smaScore();
              if(useCanvas == true){
                c.angleChange = s.smaSlope*s.sensitivity.value*0.01
              }
              else if(useVideo == true){
                v.onData(s.smaSlope*s.sensitivity.value*0.01);
              }
              s.scoreArr.push(s.smaSlope);
              g.graphY1.shift();
              g.graphY1.push(s.scoreArr[s.scoreArr.length - 1 - g.offset]);
            }
            s.updateTable();  
          }
        }
        //handle if data not changed
        else if (s.replay == false) {
          s.smaSlope = 0;
          if(useCanvas == true){
            c.angleChange = 0;
          }
          g.graphY1.shift();
          g.graphY1.push(0);
          s.scoreArr.push(0);
        }
        if(g.xoffsetSlider.max < s.scoreArr.length){
          if(s.scoreArr.length % 20 == 0) { 
            g.xoffsetSlider.max = s.scoreArr.length - 1;
          }
      }
    }
    
    s.handleData = (e) => {
      handleEventData(e);
    }

  </script>
   
</body>
</html>
)=====";
