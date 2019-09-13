const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

    <script>
        if (!!window.EventSource) {
        var source = new EventSource('/events');

        source.addEventListener('open', function(e) {
            console.log("Events Connected");
        }, false);

        source.addEventListener('error', function(e) {
            if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
            }
        }, false);

        source.addEventListener('message', function(e) {
            document.getElementById("message").innerHTML = e.data;
            console.log("message", e.data);
        }, false);

        source.addEventListener('myevent', function(e) {
            document.getElementById("myevent").innerHTML = e.data;
            console.log("myevent", e.data);
        }, false);

        data = e.data;
        if(typeof(Storage) !=="undefined")
        {window.localStorage.setItem('StateChangerPlugin', data');}
        StateChangerPlugin.message(data);
        }
    </script>

    <div id="message">Not connected</div>
    <div id="myevent">Waiting...</div>
</body>
</html>
)=====";
