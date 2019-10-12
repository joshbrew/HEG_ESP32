const char video_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
    background-color: #707070; font-family: Arial, Helvetica, sans-serif;
  }
  msgDiv {
    color: white;
  }
  eventDiv {
      color: white;
  }

  input[type=text]{
    border: 2px solid red;
    border-radius: 4px;
    height: 30px;
    padding: 2px;
    font-size: 16px;
  }

  canvas {
    z-index: 2;
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
   .scoreth {
      color: honeydew;
   }
   .hegapi {
      position: absolute;
      width: 275px;
      height: 150px;
      top: 130px;
      left: 0%;
   }
   .startbutton {
      background-color: #4CAF50; /* Green */
      position: absolute;
      left: 0%;
   }
   .stopbutton {
      background-color: #FF0000;
      position: absolute;
      left: 120px;
   }
   .sendcommand{
      position: absolute;
      top: 50%;
   }
   .sendbutton{
      background-color: #0000FF; 
   }
   .label { padding: 4px; color: white; }
   .canvascss {
      position: absolute;
      top:130px;
      left:300px;
   }
   .dummy {
      position: absolute;
      top: 135px;
      left:325px;
      width:0;
      height:0;
      border:0; 
      border:none;
   }
</style>
</head>
<body id="main_body">
    <iframe class="dummy" name="dummyframe" id="dummyframe"></iframe>
    <script>
         var ms = [];
         var red = [];
         var ir = [];
         var ratio = [];
         var smallSavLay = [];
         var largeSavLay = [];
         var adcAvg = [];
         var ratioSlope = [0];
         var AI = [];

         var slowSMA = 0;
         var fastSMA = 0;
         var smaSlope = 0;
         var scoreArr = [0];

         var playRate = 1;
         var alpha = 0;
         var useAlpha = true;
         var useRate = true;
         var enableControls = false;
         
         //appendId is the element Id you want to append this fragment to
         function appendFragment(HTMLtoAppend, appendId) {
            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", appendId + '_child');
            fragment.appendChild(newDiv);
            document.getElementById(appendId).appendChild(fragment);
         }

         //delete selected fragment. Will delete the most recent fragment if Ids are shared.
         function deleteFragment(parentId,fragmentId) {
            this_fragment = document.getElementById(fragmentId);
            document.getElementById(parentId).removeChild(this_fragment);
         }

         //Separates the appendId from the fragmentId so you can make multiple child threads with different Ids
         function appendFragmentMulti(HTMLtoAppend, appendId, fragmentId) {
            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", fragmentId + '_child');
            fragment.appendChild(newDiv);
            document.getElementById(appendId).appendChild(fragment);
         }
 
         var hegapiHTML = '<div id="hegapi" class="hegapi"> \
          <form method="post" action="/startHEG" target="dummyframe"><button id="start" class="button startbutton" type="submit">Start HEG</button></form> \
          <form method="post" action="/stopHEG" target="dummyframe"><button id="stop" class="button stopbutton" type="submit">Stop HEG</button></form> \
          <form class="sendcommand" method="post" action="/command" target="dummyframe"><label class="label" for="command">Command:</label><br><input type="text" id="command" name="command"><button class="button sendbutton" type="submit">Send</button></form> \
          </div>';
    
         var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>';
         var videoapiHTML = '<div id="vidapi" style="position:absolute; top:500px;"> \
          <input class="button" id="fs" name="fs" type="file" accept="video/*"/><br> \
          <button class="button" id="useAlpha" name="useAlpha">Fade</button> \
          <button class="button" id="useRate" name="useRate">Speed</button> \
          </div>';
         var videoHTML = '<video id="vid" height="480px" width="640px" class="canvascss" src="https://vjs.zencdn.net/v/oceans.mp4" type="video/mp4" autoplay loop muted></video><canvas class="canvascss" id="vidcanvas"></canvas>';
         var containerHTML = '<div id="container"></div>';
         var messageHTML = '<msgDiv id="message">Output:</div>';
         var eventHTML = '<eventDiv id="myevent">Not connected...</eventDiv>';
         var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"><tr><th>ms</th><th>Red</th><th>IR</th><th>Ratio</th><th>sSavLay</th><th>lSavLay</th><th>adcAvg</th><th>rSlope</th><th>A.I.</th><th class="scoreth">SMA1s-2s</th></tr></table></div>';
         var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><th>Awaiting Data...</th></tr></table></div>';

         //Setup page as fragments so updates to elements are asynchronous.
         appendFragment(dataDivHTML,"main_body");
         appendFragment(videoapiHTML,"main_body");
         appendFragment(videoHTML,"main_body");
         appendFragment(hegapiHTML,"main_body");
         appendFragment(containerHTML,"dataDiv");
         appendFragment(messageHTML,"container");
         appendFragment(eventHTML,"container");
         appendFragment(tableHeadHTML,"container");
         appendFragment(tableDatHTML,"container");

         var vidQuery = document.getElementById("vid");
         var c = document.getElementById("vidcanvas");
         c.width = vidQuery.width;
         c.height = vidQuery.height;
         var gl = c.getContext("webgl");
         gl.clearColor(0,0,0.1,0);
         gl.clear(gl.COLOR_BUFFER_BIT);

         document.getElementById("useAlpha").onclick = function(){
          if(useAlpha == true){
            useAlpha = false;
            alpha = 0;
          }
          else{ useAlpha = true; }
         }

         document.getElementById("useRate").onclick = function() {
          if(useRate == true){
            useRate = false;
            playRate = 1;
            vidQuery.playbackRate = 1;
          }
          else{ useRate = true; }
         }

         document.getElementById("start").onclick = function(){
          if(playRate < 0.1){ vidQuery.playbackRate = 0; }
          else{ vidQuery.playbackRate = playRate; }
         }
         document.getElementById("stop").onclick = function(){vidQuery.playbackRate = 0;}

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

            source.addEventListener('myevent', function(e) {
                console.log("myevent", e.data);
                if(document.getElementById("myevent").innerHTML != e.data){
                  document.getElementById("myevent").innerHTML = e.data;
                  if(e.data.includes("|")) {
                      var dataArray = e.data.split("|");
                      ms.push(parseInt(dataArray[0]));
                      red.push(parseInt(dataArray[1]));
                      ir.push(parseInt(dataArray[2]));
                      ratio.push(parseFloat(dataArray[3]));
                      smallSavLay.push(parseFloat(dataArray[4]));
                      largeSavLay.push(parseFloat(dataArray[5]));
                      adcAvg.push(parseInt(dataArray[6]));
                      ratioSlope.push(parseFloat(dataArray[7]));
                      AI.push(parseFloat(dataArray[8]));
  
                      if(largeSavLay.length-1 > 40){
                        var temp = largeSavLay.slice(largeSavLay.length - 40,largeSavLay.length);
                        var temp2 = largeSavLay.slice(largeSavLay.length - 20,largeSavLay.length);
                        slowSMA = temp.reduce((a,b) => a + b, 0) / 40;
                        fastSMA = temp2.reduce((a,b) => a + b, 0) / 20;
                        smaSlope = fastSMA - slowSMA;
                        if(useAlpha == true) {
                          if(((alpha < 0.8) || (smaSlope > 0)) && ((alpha > 0)||(smaSlope < 0))){
                            if(alpha - smaSlope < 0){
                              alpha = 0;
                            }
                            else if(alpha - smaSlope > 0.8){
                              alpha = 0.8;
                            }
                            else{
                              alpha -= smaSlope;
                            }
                          }
                        }
                        if(useRate == true){
                          if(((vidQuery.playbackRate < 3) || (smaSlope < 0)) && ((vidQuery.playbackRate > 0) || (smaSlope > 0)))
                          { 
                            playRate = vidQuery.playbackRate + smaSlope*0.5;
                            if((playRate < 0.05) && (playRate > 0)){
                              vidQuery.playbackRate = 0;
                            }
                            else if(playRate < 0) {
                              vidQuery.currentTime += smaSlope;
                            }
                            else if((playRate > 0.05) && (playRate < 0.1)){
                              vidQuery.playbackRate = 0.5;
                            }
                            else{
                              vidQuery.playbackRate = playRate;
                            }
                          }
                        }
                        scoreArr.push(smaSlope);
                      }
                      document.getElementById("dataTable").innerHTML = '<tr><td id="ms">'+ms[ms.length-1-1]+'</td><td id="red">'+red[red.length-1-1]+'</td><td id="ir">'+ir[ir.length-1-1]+'</td><td id="ratio">'+ratio[ratio.length-1-1]+'</td><td id="smallSavLay">'+smallSavLay[smallSavLay.length-1-1]+'</td><td id="largeSavLay">'+largeSavLay[largeSavLay.length-1-1]+'</td><td id="adcAvg">'+adcAvg[adcAvg.length-1-1]+'</td><td id="ratioSlope">'+ratioSlope[ratioSlope.length-1-1]+'</td><td id="AI">'+AI[AI.length-1]+'</td><td class="scoreth">'+scoreArr[scoreArr.length-1].toFixed(4)+'</td></tr>'
                  }
                }
            }, false);
        }

      (function localFileVideoPlayer() {
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
      })()
      
      function animateRect() {
          var gl = c.getContext("webgl");
          gl.clearColor(0,0,0.1,alpha);
          gl.clear(gl.COLOR_BUFFER_BIT);
          requestAnimationFrame(animateRect);
      }
      animateRect();
      
  </script>
   
</body>
</html>
)=====";


//Using SPIFFS:
//<script src="jquery-3.4.1.min.js"></script> 
