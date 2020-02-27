const char ws_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
   <head>
      <script type = "text/javascript">
        var ws = new WebSocket('ws://'+window.location.hostname+'/ws'); 
        ws.onopen = function() {
            window.alert("Connected");
         };
 
         ws.onmessage = function(evt) {
            var date = new Date();
            document.getElementById("display").innerHTML  = "Sensor reading: " + evt.data + " at " + date.getFullYear() + "/" + date.getMonth() + "/" + date.getDate() + " " + date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds() + ":" + date.getMilliseconds();
        };
 
      </script>
   </head>
 
   <body>

      <div id="HEGAPI">     
        <form method="post" action="/startHEG" target="dummyframe">
            <button type="submit">Start HEG</button>
        </form>
        <form method="post" action="/stopHEG" target="dummyframe">
            <button type="submit">Stop HEG</button>
        </form>
  
        <form method="post" action="/command" target="dummyframe">
            <input type="text" id="command" name="command"><button type="submit">Send</button>
        </form>
      
      </div>
   
      <div>
         <p id = "display">Sensor not transmitting</p>
      </div>
   </body>

   <iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>
</html>
)=====";
