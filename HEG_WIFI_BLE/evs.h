const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
    background-color: #707070;
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
   .hegapi {
      position: absolute;
      width: 275px;
      height: 150px;
      top: 105px;
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
   .canvascss {
      position: absolute;
      top:110px;
      left:300px;
   }
</style>
</head>
<body id="main_body">
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
        var slowFastScore = [100];
        var slowSMAArray = [];

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

        function slowFastSMAScore(dataArray) {
            var dy = 0;
            var slowSMA = 0;
            var fastSMA = 0;
            var score = 0;
            if(dataArray.length > 20) {
                for(var i=dataArray.length-20;i<dataArray.length-1;i++) {
                    slowSMA += dataArray[i];
                    if (i > dataArray.length - 6) {
                        fastSMA += dataArray[i];
                    }
                }
                slowSMA = slowSMA / 20;
                slowSMAArray.push(slowSMA);

                fastSMA = fastSMA / 5;
                dy = fastSMA - slowSMAArray[slowSMAArray.length - 1];
                score = slowFastScore[slowFastScore.length - 1] + dy;
                slowFastScore.push(score);
            }
        }

        var hegapiHTML = '<div id="HEGAPI" class="hegapi"> \
          <form method="post" action="/startHEG" target="dummyframe"><button class="button startbutton" type="submit">Start HEG</button></form> \
          <form method="post" action="/stopHEG" target="dummyframe"><button class="button stopbutton" type="submit">Stop HEG</button></form> \
          <form class="sendcommand" method="post" action="/command" target="dummyframe"><label class="label" for="command">Command:</label><br><input type="text" id="command" name="command"><button class="button sendbutton" type="submit">Send</button></form> \
          </div>';
    
        var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>'

        var canvasHTML = '<div id="canvasContainer"><canvas class="canvascss" id="myCanvas" height="400px" width="400px"></canvas></div>'

        var containerHTML = '<div id="container"></div>';
        var messageHTML = '<msgDiv id="message">Output:</div>';
        var eventHTML = '<eventDiv id="myevent">Not connected...</eventDiv>';
        var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"><tr><th>ms</th><th>Red</th><th>IR</th><th>Ratio</th><th>sSavLay</th><th>lSavLay</th><th>adcAvg</th><th>rSlope</th><th>A.I.</th></tr></table></div>';
        var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><th>Getting Score...</th></tr></table></div>';

        //Setup page as fragments so updates to elements are asynchronous.
        appendFragment(dataDivHTML,"main_body");
        appendFragment(canvasHTML,"main_body");
        appendFragment(hegapiHTML,"main_body");
        appendFragment(containerHTML,"dataDiv");
        appendFragment(messageHTML,"container");
        appendFragment(eventHTML,"container");
        appendFragment(tableHeadHTML,"container");
        appendFragment(tableDatHTML,"container");

        //var data;
        //function getMessage(){
        //    return data;
        //}

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

                <!-- Android interface -->
                 // data = e.data;
                 // if (typeof(Storage) !== "undefined"){window.localStorage.setItem('StateChangerPlugin', data);}  
                 // if (typeof(StateChangerPlugin) !== "undefined"){StateChangerPlugin.message(data);} 
                <!-- Android interface -->
                <!-- Web interface -->
                  // var onRead = new CustomEvent('on_read', { detail: {data: data} });
                  // window.parent.dispatchEvent(onRead); 
                  //window.parent.postMessage(data, "*");
                <!-- Web interface -->
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
  
                      //slowFastSMAScore(ratio);
                      document.getElementById("dataTable").innerHTML = '<tr><td id="ms">'+ms[ms.length-1]+'</td><td id="red">'+red[red.length-1]+'</td><td id="ir">'+ir[ir.length-1]+'</td><td id="ratio">'+ratio[ratio.length-1]+'</td><td id="smallSavLay">'+smallSavLay[smallSavLay.length-1]+'</td><td id="largeSavLay">'+largeSavLay[largeSavLay.length-1]+'</td><td id="adcAvg">'+adcAvg[adcAvg.length-1]+'</td><td class="scoreth" id="ratioSlope">'+ratioSlope[ratioSlope.length-1]+'</td><td id="AI">'+AI[AI.length-1]+'</td></tr>'
                  }
                }
            }, false);

            //data = e.data;
            //if(typeof(Storage) !=="undefined") 
            //{window.localStorage.setItem('StateChangerPlugin', data);}
            //StateChangerPlugin.message(data);
            
        }

      var mainCanvas = document.getElementById("myCanvas");
      var mainContext = mainCanvas.getContext('2d');
       
      var canvasWidth = mainCanvas.width;
      var canvasHeight = mainCanvas.height;
       
      var angle = 1.57;
       
      var requestAnimationFrame = window.requestAnimationFrame || 
                                  window.mozRequestAnimationFrame || 
                                  window.webkitRequestAnimationFrame || 
                                  window.msRequestAnimationFrame;
       
      function drawCircle() {
          mainContext.clearRect(0, 0, canvasWidth, canvasHeight);
           
          // color in the background
          mainContext.fillStyle = "#2751CE";
          mainContext.fillRect(0, 0, canvasWidth, canvasHeight);
           
          // draw the circle
          mainContext.beginPath();
           
          var radius = 25 + 175 * Math.abs(Math.cos(angle));
          mainContext.arc(200, 200, radius, 0, Math.PI * 2, false);
          mainContext.closePath();
           
          // color in the circle
          mainContext.fillStyle = "#EE1818";
          mainContext.fill();

          angleChange = ratioSlope[ratioSlope.length-1]
          if(((angle > 1.57) || (angleChange > 0)) && ((angle < 3.14) || (angleChange < 0))) {
            angle += angleChange*0.2;
          }
          requestAnimationFrame(drawCircle);
      }
      drawCircle();
      
  </script>

  <iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>
   
</body>
</html>
)=====";
