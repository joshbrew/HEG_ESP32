const char ws_page1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
   <head>
      <script type = "text/javascript">
        var ws = new WebSocket('ws://)=====";

// ws_page1 + IP + ws_page2
        
const char ws_page2[] PROGMEM = R"=====(/ws'); 
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
      <div>
         <p id = "display">Not connected</p>
      </div>
   </body>
</html>
)=====";
