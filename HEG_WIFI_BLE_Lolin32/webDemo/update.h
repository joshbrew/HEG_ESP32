const char update_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color: #303030;
  color: white;
  font-family: Arial, Helvetica, sans-serif;
}
.container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
}
a {
  color: lightblue;
}
a:visited {
  color: lightblue;
}
a:hover {
  color: lightgreen;
}

progress {
  width: 300px;
}

</style>
</head>
<body>
  <div id="container" class="container">
    <h4>Upload compiled sketch .bin file</h4>
    <form method='POST' id="doUpdate" action='/doUpdate' enctype='multipart/form-data'>
      First: <input type='file' id="file" name='update'>
    </form><br><table style="transform: translateX(-98px);"><tr><td>Then: </td><td><button id="submit">Upload</button></td></tr></table><br>
    Device response: <div id="message"></div>
    <progress id="progressbar" max="100"></progress>
    <br><br>Latest firmware can be found at <br>
    <a href="https://github.com/moothyknight/HEG_ESP32">https://github.com/moothyknight/HEG_ESP32</a>
  <div>

  <script>

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
            document.getElementById("progressbar").value = e.data;
            console.log("HEGDUINO", e.data);
        }, false);
      }

      document.getElementById("submit").onclick = function() {
        if(document.getElementById("file").value == ""){

        }
        else{
          document.getElementById("doUpdate").submit();
        }
      }

  </script>
</body>
</html>
)=====";
