const char connect_page1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h4>Connect to WiFi</h4>
  <form method ='post' action='/doConnect' enctype='multipart/form-data'>
    SSID:<br>
      <input type='text' id='ssid' name='ssid'><br>
    Password:<br>
      <input type='text' id='pw' name='pw'><br>
      <input type='checkbox' id='AP_ONLY' name='AP_ONLY'>Access Point Only<br>
      <input type='checkbox' id'btSwitch' name='btSwitch'>Use Bluetooth<br>
    <input type='submit' id='Connect' value='Connect'>
  </form>
)=====";

const char connect_page2[] PROGMEM = R"=====(
</body>
</html>
)=====";
/* Auto-reconnect?
      <input type='checkbox' id='rc' name='rc'><br>
    Save?
      <input type='checkbox' id='save' name='save'>

      */
