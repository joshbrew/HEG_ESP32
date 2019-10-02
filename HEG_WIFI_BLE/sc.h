const char sc_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
</style>
</head>
<body id="main_body">

    <div id="HEGAPI">     
      <form id="startForm" method="post" action="/startHEG" target="dummyframe">
          <button id="start" type="submit">Start HEG</button>
      </form>
      <form id="stopForm" method="post" action="/stopHEG" target="dummyframe">
          <button id="stop" type="submit">Stop HEG</button>
      </form>

      <form method="post" action="/command" target="dummyframe">
          Command:<br>
          <input type="text" id="command" name="command"><button type="submit">Send</button>
      </form>
    </div>

    <script>

        var data;
        function getMessage(){
           return data;
        }

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
                //document.getElementById("message").innerHTML = e.data;
                console.log("HEGDUINO", e.data);

                 <!-- Android interface -->
                 data = e.data;
                 if (typeof(Storage) !== "undefined"){window.localStorage.setItem('StateChangerPlugin', data);}  
                 if (typeof(StateChangerPlugin) !== "undefined"){StateChangerPlugin.message(data);} 
                <!-- Android interface -->
                <!-- Web interface -->
                  var onRead = new CustomEvent('on_read', { detail: {data: data} });
                  window.parent.dispatchEvent(onRead); 
                  window.parent.postMessage(data, "*");
                <!-- Web interface -->
            }, false);

            source.addEventListener('myevent', function(e) {
                //document.getElementById("myevent").innerHTML = e.data;
                console.log("myevent", e.data);
                
            }, false);

            function startHEG(){
              document.getElementById('startForm').submit();
            }

            function stopHEG(){
              document.getElementById('stopForm').submit();
            }

            //data = e.data;
            //if(typeof(Storage) !=="undefined") 
            //{window.localStorage.setItem('StateChangerPlugin', data);}
            //StateChangerPlugin.message(data);
            
        }
      
  </script>

  <iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>
   
</body>
</html>
)=====";
