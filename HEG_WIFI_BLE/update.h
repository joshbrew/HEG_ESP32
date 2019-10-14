const char update_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color: #707070;
  color: white;
  font-family: Arial, Helvetica, sans-serif;
}
.container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  min-heigh: 100vh;
}
</style>
</head>
<body>
  <div id="container" class="container">
    <h4>Upload compiled sketch .bin file</h4>
    <form method='POST' action='/doUpdate' enctype='multipart/form-data'>
      <input type='file' name='update'><input type='submit' value='Update'>
    </form>
    Device response: <div id="message"></div>
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
            console.log("HEGDUINO", e.data);
        }, false);

        source.addEventListener('heg', (e) => {
            this.handleData(e);
        }, false);
      }

  </script>
</body>
</html>
)=====";

