const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
      background-color: gray
  }
  eventDiv {
      color: chartreuse
  }
  scoreDiv {
      color: tomato
  }
</style>
</head>
<body id="main_body">

    <div id="HEGAPI">     
      <form method="post" action="/startHEG" target="dummyframe">
          <button type="submit">Start HEG</button>
      </form>
      <form method="post" action="/stopHEG" target="dummyframe">
          <button type="submit">Stop HEG</button>
      </form>
    
    </div>

    <div id="canvasContainer">
        <canvas id="myCanvas" height="400px" width="400px"></canvas>
    </div>

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

        var containerHTML = '<div id="container"></div>';
        var messageHTML = '<div id="message">Not connected</div>';
        var eventHTML = '<eventDiv id="myevent">Waiting...</eventDiv>';
        var scoreHTML = '<scoreDiv id="ScoreHTML">Getting score...</scoreDiv>';

        appendFragment(containerHTML,"HEGAPI");
        appendFragment(messageHTML,"container");
        appendFragment(eventHTML,"container");
        appendFragment(scoreHTML,"container");

        //var data;
        //function getMessage(){
        //    return data;
        //}

        if (!!window.EventSource) {
            var source = new EventSource('/events');

            source.addEventListener('open', function(e) {
                console.log("HEGDUINO", "Events Connected");
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
                document.getElementById("myevent").innerHTML = e.data;
                console.log("myevent", e.data);
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
                    document.getElementById("ScoreHTML").innerHTML = largeSavLay[largeSavLay.length-1];//slowFastScore[slowFastScore.length - 1]
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
           
          var radius = 25 + 150 * Math.abs(Math.cos(angle));
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
