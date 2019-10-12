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
   .saveLoadBar{ position: absolute; top: 200px; }
   .saveLoadButtons{ background-color: teal; }
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
<script>
  class graphJS {
    constructor(canvasId, nPoints=[1000], color=[0,255,0], res=[1400,400]){
      //WebGL graph based on: https://tinyurl.com/y5roydhe
      //HTML : <canvas id={canvasId}></canvas><canvas id={canvasId+"text"}></canvas>;
      this.gl,
      this.shaderProgram,
      this.vertices,
      this.canvas;

      this.canvasId = canvasId;
      this.textId = canvasId + "text";
      this.color = color;
      this.res = res;
      
      this.ms = [0];
      this.VERTEX_LENGTH = nPoints;
      this.graphY1 = [...Array(nPoints).fill(0)];

      this.VERTEX_SHADER = `
      attribute vec4 coords;
      attribute float pointSize;
      void main(void) {
        gl_Position = coords;
        gl_PointSize = pointSize;
      }
      `;

      this.FRAGMENT_SHADER = `
      precision mediump float;
      uniform vec4 color;
      void main(void) {
        gl_FragColor = color;
      }
      `;

      this.initGL(canvasId);
      this.createShader();
      this.createVertices(color);
      
      this.graphtext = document.getElementById(this.textId).getContext("2d");
      this.graphtext.canvas.width = res[0];
      this.graphtext.canvas.height = res[1];
      this.graphtext.font = "20pt Arial";
      this.graphtext.fillStyle = "#00ff00";
      
      this.draw();

      window.addEventListener('resize', this.setCanvasSize(this.canvasId), false);
    }

    setCanvasSize(canvasId) {
        this.canvas.width = this.res[0]; //Define in CSS
        this.canvas.height = this.res[1];
        this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
    }

    initGL(canvasId) {
        this.canvas = document.querySelector('#'+canvasId);
        this.gl = this.canvas.getContext('webgl');
          this.setCanvasSize(canvasId);
          console.log(this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
          this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
          this.gl.clearColor(0, 0, 0, 1);
    }
      
    makePoints(numPoints) {
        const highestPointNdx = numPoints - 1;
        return Array.from({length: numPoints * 2}, (_, i) => {
          const pointId = i / 2 | 0;
          const lerp0To1 = pointId / highestPointNdx;
          const isY = i % 2;
          return isY
            ? this.graphY1[i]     // Y
            : (lerp0To1 * 4 - 1); // X
        });
    }

    createVertices() {
          this.vertices = this.makePoints(this.VERTEX_LENGTH);
          const buffer = this.gl.createBuffer();
          this.gl.bindBuffer(this.gl.ARRAY_BUFFER, buffer);
          this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.vertices), this.gl.DYNAMIC_DRAW);

          const coords = this.gl.getAttribLocation(this.shaderProgram, 'coords');
          this.gl.vertexAttribPointer(coords, 2, this.gl.FLOAT, false, 0, 0);
          this.gl.enableVertexAttribArray(coords);
          // gl.bindBuffer(gl.ARRAY_BUFFER, null);

          const pointSize = this.gl.getAttribLocation(this.shaderProgram, 'pointSize');
          this.gl.vertexAttrib1f(pointSize, 2);

          const uniformColor = this.gl.getUniformLocation(this.shaderProgram, 'color');
          this.gl.uniform4f(uniformColor, this.normalize(this.color[0]), this.normalize(this.color[1]), this.normalize(this.color[2]), 1);
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

    draw = () => {
      this.gl.bufferSubData(this.gl.ARRAY_BUFFER, 0, new Float32Array(this.vertices));
      this.gl.clear(this.gl.COLOR_BUFFER_BIT);
      this.gl.drawArrays(this.gl.LINE_STRIP, 0, this.VERTEX_LENGTH);

      this.graphtext.clearRect(0, 0, this.canvas.width, this.canvas.height);
      this.graphtext.fillText("t: " + (this.ms[this.ms.length - 1]*0.001).toFixed(2),5,25);
      this.graphtext.fillText("y: " + this.graphY1[this.graphY1.length - 1].toFixed(4),5,50);
      requestAnimationFrame(this.draw);
    }

    normalize(val, max=255, min=0) { return (val - min) / (max - min); }
  }
      
  class HEGwebAPI {
    constructor(parentId="main_body"){
      this.ms=[];
      this.red=[];
      this.ir=[];
      this.ratio=[];
      this.smallSavLay=[];
      this.largeSavLay=[];
      this.adcAvg=[];
      this.ratioSlope=[];
      this.AI=[];

      this.slowSMA = 0;
      this.fastSMA = 0;
      this.smaSlope = 0;
      this.scoreArr = [0];
      this.replay = false;

      this.csvDat = [];
      this.csvIndex = 0;

      this.createHegUI(parentId);

    }

    resetVars() {
        this.ms = [];
        this.red = [];
        this.ir = [];
        this.ratio = [];
        this.smallSavLay = [];
        this.largeSavLay = [];
        this.adcAvg = [];
        this.ratioSlope = [];
        this.AI = [];
        
        this.slowSMA = 0;
        this.fastSMA = 0;
        this.smaSlope = 0;
        this.scoreArr = [0];
        this.replay = false;
    }

    smaScore() {
      var temp = this.largeSavLay.slice(this.largeSavLay.length - 40,this.largeSavLay.length);
      var temp2 = this.largeSavLay.slice(this.largeSavLay.length - 20,this.largeSavLay.length);
      this.slowSMA = temp.reduce((a,b) => a + b, 0) / 40;
      this.fastSMA = temp2.reduce((a,b) => a + b, 0) / 20;
      this.smaSlope = this.fastSMA - this.slowSMA;
      this.scoreArr.push(this.smaSlope);
    }

    saveCSV(){
      var csv = "ms,red,ir,ratio,sSavLay,lSavLay,adcAvg,ratioSlope,AI\n"; //csv header
      for(var i = 0; i<this.ms.length - 1; i++){
        var temp = [this.ms[i],this.red[i],this.ir[i],this.ratio[i],this.smallSavLay[i],this.largeSavLay[i],this.adcAvg[i],this.ratioSlope[i],this.AI[i],].join(',') + "\n";
        csv += temp;
      }
      var hiddenElement = document.createElement('a');
      hiddenElement.href = "data:text/csv;charset=utf-8," + encodeURI(csv);
      hiddenElement.target = "_blank";
      hiddenElement.download = document.getElementById("csvname").value+".csv";
      hiddenElement.click();
    }
 
    replayCSV = () => {
      console.log("Define replay function in script");
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

    //appendId is the element Id you want to append this fragment to
    static appendFragment(HTMLtoAppend, appendId) {

        var fragment = document.createDocumentFragment();
        var newDiv = document.createElement('div');
        newDiv.innerHTML = HTMLtoAppend;
        newDiv.setAttribute("id", appendId + '_child');

        fragment.appendChild(newDiv);

        document.getElementById(appendId).appendChild(fragment);
    }

    //delete selected fragment. Will delete the most recent fragment if Ids are shared.
    static deleteFragment(parentId,fragmentId) {
        var this_fragment = document.getElementById(fragmentId);
        document.getElementById(parentId).removeChild(this_fragment);
    }

    //Separates the appendId from the fragmentId so you can make multiple child threads with different Ids
    static appendFragmentMulti(HTMLtoAppend, appendId, fragmentId) {

        var fragment = document.createDocumentFragment();
        var newDiv = document.createElement('div');
        newDiv.innerHTML = HTMLtoAppend;
        newDiv.setAttribute("id", fragmentId + '_child');

        fragment.appendChild(newDiv);

        document.getElementById(appendId).appendChild(fragment);
    }

    handleData(e){
      console.log("HEGDUINO", e.data);
    }

    createEventListeners() {
      if (!!window.EventSource) {
        var source = new EventSource('/events');

        source.addEventListener('open', function(e) {
            console.log("HEGDUINO", "Events Connected");
            //document.getElementById("message").innerHTML = "Output:";
        }, false);

        source.addEventListener('error', function(e) {
            if (e.target.readyState != EventSource.OPEN) {
              console.log("HEGDUINO", "Events Disconnected");
            }
        }, false);

        source.addEventListener('message', function(e) {
            document.getElementById("message").innerHTML = e.data;
            console.log("HEGDUINO", e.data);
        }, false);

        source.addEventListener('myevent', (e) => {
            this.handleData(e);
        }, false);
      }
    }

    createHegUI(parentId) {
      var hegapiHTML = '<div id="hegapi" class="hegapi"> \
        <form method="post" action="/startHEG" target="dummyframe"><button class="button startbutton" type="submit">Start HEG</button></form> \
        <form method="post" action="/stopHEG" target="dummyframe"><button class="button stopbutton" type="submit">Stop HEG</button></form> \
        <form class="sendcommand" method="post" action="/command" target="dummyframe"><label class="label" for="command">Command:</label><br><input type="text" id="command" name="command"><button class="button sendbutton" type="submit">Send</button></form> \
        <div id="saveLoad" class="saveLoadBar"> \
          <label class="label" for="csvname">Save Session:</label><br><input type="text" id="csvname" name="csvname" placeholder="session_data" required></input> \
          <button class="button saveLoadButtons" id="savecsv">Save CSV</button> \
          <button class="button saveLoadButtons" id="replaycsv">Replay CSV</button> \
        </div> \
        </div> \
        <iframe name="dummyframe" id="dummyframe" class="dummy"></iframe> \
        ';
  
      var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>';
      var containerHTML = '<div id="container"></div>';
      var messageHTML = '<msgDiv id="message">Output:</div>';
      var eventHTML = '<eventDiv id="myevent">Not connected...</eventDiv>';
      var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"><tr><th>ms</th><th>Red</th><th>IR</th><th>Ratio</th><th>sSavLay</th><th>lSavLay</th><th>adcAvg</th><th>rSlope</th><th>A.I.</th><th class="scoreth">SMA1s-2s</th></tr></table></div>';
      var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><th>Awaiting Data...</th></tr></table></div>';

      HEGwebAPI.appendFragment(dataDivHTML,"main_body");
      HEGwebAPI.appendFragment(hegapiHTML,"main_body");
      HEGwebAPI.appendFragment(containerHTML,"dataDiv");
      HEGwebAPI.appendFragment(messageHTML,"container");
      HEGwebAPI.appendFragment(eventHTML,"container");
      HEGwebAPI.appendFragment(tableHeadHTML,"container");
      HEGwebAPI.appendFragment(tableDatHTML,"container");

      document.getElementById("savecsv").onclick = () => {this.saveCSV();}
      document.getElementById("replaycsv").onclick = () => {this.openCSV();}

      this.createEventListeners();
    }

  }

  class circleJS {
    constructor(canvasId){
      this.c = document.getElementById(canvasId);
      this.ctx = this.c.getContext('2d');
       
      this.cWidth = this.c.width;
      this.cHeight = this.c.height;
       
      this.angle = 1.57;

      this.draw();
    }
    
    changeAngle(){
      return;
    }
    
    draw = () => {
        this.ctx.clearRect(0, 0, this.cWidth, this.cHeight);
         
        // color in the background
        this.ctx.fillStyle = "#2751CE";
        this.ctx.fillRect(0, 0, this.cWidth, this.cHeight);
         
        // draw the circle
        this.ctx.beginPath();
         
        var radius = 25 + 175 * Math.abs(Math.cos(this.angle));
        this.ctx.arc(200, 200, radius, 0, Math.PI * 2, false);
        this.ctx.closePath();
         
        // color in the circle
        this.ctx.fillStyle = "#EE1818";
        this.ctx.fill();

        this.changeAngle();

        requestAnimationFrame(this.draw);
    }
  }

  
</script>
</head>
<body id="main_body">
    <script>
      var s = new HEGwebAPI();

      var canvasHTML = '<div id="canvasContainer"><canvas class="canvascss" id="canvas1" height="400px" width="400px"></canvas></div>'
      var shaderHTML = '<div id="shaderContainer"><canvas class="webglcss" id="graph1"></canvas><canvas class="webglcss" id="graph1text"></canvas></div>'

      //Setup page as fragments so updates to elements are asynchronous.
      HEGwebAPI.appendFragment(canvasHTML,"main_body");
      HEGwebAPI.appendFragment(shaderHTML,"main_body");

      var c = new circleJS("canvas1");
      var graph1 = new graphJS("graph1",1500,[255,100,80]);

      c.changeAngle = function() {
        if(((this.angle > 1.57) || (s.smaSlope > 0)) && ((this.angle < 3.14) || (s.smaSlope < 0))) {
          this.angle += s.smaSlope*.1;
        }
        return;
      }

      s.replayCSV = () => {
        if(s.csvIndex == 0){
          s.ms.push(parseInt(s.csvDat[s.csvIndex][0]));
          s.largeSavLay.push(parseFloat(s.csvDat[s.csvIndex][5]))
          
        }
        if(s.csvIndex < s.csvDat.length - 1){
          s.csvIndex++;
          s.ms.push(parseInt(s.csvDat[s.csvIndex][0]));
          s.largeSavLay.push(parseFloat(s.csvDat[s.csvIndex][5]));
          if(s.ms.length >= 2){
            if(s.largeSavLay.length > 40){
              s.smaScore();
              graph1.graphY1.shift();
              graph1.graphY1.push(s.smaSlope);
              graph1.createVertices();
            }
            else {
              s.smaSlope = 0;
              graph1.graphY1.shift();
              graph1.graphY1.push(0);
              graph1.createVertices();
            }
            document.getElementById("dataTable").innerHTML = '<tr><td id="ms">'+s.csvDat[s.csvIndex][0]+'</td><td id="red">'+s.csvDat[s.csvIndex][1]+'</td><td id="ir">'+s.csvDat[s.csvIndex][2]+'</td><td id="ratio">'+s.csvDat[s.csvIndex][3]+'</td><td id="smallSavLay">'+s.csvDat[s.csvIndex][4]+'</td><td id="largeSavLay">'+s.csvDat[s.csvIndex][5]+'</td><td id="adcAvg">'+s.csvDat[s.csvIndex][6]+'</td><td id="ratioSlope">'+s.csvDat[s.csvIndex][7]+'</td><td id="AI">'+s.csvDat[s.csvIndex][8]+'</td><td class="scoreth">'+s.smaSlope.toFixed(4)+'</td></tr>'
            setTimeout(() => {s.replayCSV();},(s.ms[s.csvIndex]-s.ms[s.csvIndex-1])); //Call until end of index. Need to make this async
          }
          else {
            s.smaSlope = 0;
            graph1.graphY1.shift();
            graph1.graphY1.push(0);
            graph1.createVertices();
          }
        }
        else {
          s.resetVars();
          s.csvDat = [];
          s.csvIndex = 0;
        }
    }

    s.handleData = (e) => {
      console.log("HEGDUINO", e.data);
      if(document.getElementById("myevent").innerHTML != e.data){
        document.getElementById("myevent").innerHTML = e.data;
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

          if(s.largeSavLay.length > 40){
            s.smaScore();
            graph1.graphY1.shift();
            graph1.graphY1.push(s.smaSlope);
            graph1.createVertices();
          }
          document.getElementById("dataTable").innerHTML = '<tr><td id="ms">'+s.ms[s.ms.length-1-1]+'</td><td id="red">'+s.red[s.red.length-1-1]+'</td><td id="ir">'+s.ir[s.ir.length-1-1]+'</td><td id="ratio">'+s.ratio[s.ratio.length-1-1]+'</td><td id="smallSavLay">'+s.smallSavLay[s.smallSavLay.length-1-1]+'</td><td id="largeSavLay">'+s.largeSavLay[s.largeSavLay.length-1-1]+'</td><td id="adcAvg">'+s.adcAvg[s.adcAvg.length-1-1]+'</td><td id="ratioSlope">'+s.ratioSlope[s.ratioSlope.length-1-1]+'</td><td id="AI">'+s.AI[s.AI.length-1]+'</td><td class="scoreth">'+s.scoreArr[s.scoreArr.length-1].toFixed(4)+'</td></tr>'
        }
      }
      else if (s.replay == false) {
        s.smaSlope = 0;
        graph1.graphY1.shift();
        graph1.graphY1.push(0);
        graph1.createVertices();
      }
    }
  </script>
   
</body>
</html>
)=====";
