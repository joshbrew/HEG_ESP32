const char connect_page1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color:#303030;
  color: white;
  font-family: Arial, Helvetica, sans-serif;
}
input[type=text]{
  border: 2px solid lime;
  padding: 2px;
  font-size: 16px;
}
input[type=password]{
  border: 2px solid red;
  padding: 2px;
  font-size: 16px;
}
input[type=submit]{
    border: none;
    border-radius: 12px;
    color: royalblue;
    padding: 12px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    cursor: pointer;
}
.container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
}
#staticoptions{
  opacity: 0.3;
}
#staticextoptions{
  opacity: 0.3
}
</style>
</head>
<body>
 <div id='formContainer' class='container'>
  <h2>Connection Settings</h4>
    <!-- For remote form POSTing, add host IP to front of action path (e.g. <form method='post' action='http://192.168.4.1/doConnect'...) -->
  <form method ='post' action='/doConnect' enctype='multipart/form-data'>
    <input type='radio' id='AP_ONLY' name='choices' value='1'>Reset to Default (OR toggle with "W" via USB Serial or BT)<br>
    <input type='radio' id='btSwitch' name='choices' value='2'>Bluetooth LE (OR toggle with "b" via USB Serial or BT)<br>
    <input type='radio' id='btserSwitch' name='choices' value='5'>Bluetooth Serial (OR toggle with "B" via USB Serial or BT)<br>
    <input type='radio' id='usbonlyswitch' name='choices' value='6'>USB Serial only (OR toggle with "u" via USB Serial or BT)<br>
    <br>
    Connect to Network (WiFi scan results at bottom of page): <br>
    SSID:<br>
      <input type='text' id='ssid' name='ssid'><br>
    Password:<br>
      <input type='password' id='pw' name='pw'><br>
      <input type='radio' id='defaultC' name='choices' value='9' checked>Auto-Connect (find the IP via your router or via USB Serial output on boot)<br>
      <input type='radio' id='suggestIP' name='choices' value = '4'>Suggest Static IP (Return to this page after reconnecting to this device)<br>
      <br>
      <hr>

    Optional Static IP mode: <br><br>
    <div id="staticoptions">
    Static IP (e.g. 192.168.0.199, first 3 numbers must match gateway):<br>
      <input type='text' id='static' name='static'><br>
    Gateway IP (e.g. 192.168.0.1, check router settings):<br>
      <input type='text' id='gateway' name='gateway'><br>
    Subnet Mask (e.g. 255.255.255.0, check router settings):<br>
      <input type='text' id='subnet' name='subnet'></div>
    <input type='radio' id='use_static' name='choices' value='0'>Use Static IP<br>
    <br>
    <hr>
    <div id="staticextoptions">
    For use with Static IP with DNS mode:<br><br>
    Primary DNS:<br>
      <input type='text' id='primary' name='primary'><br>
    Secondary DNS (optional):<br>
      <input type='text' id='secondary' name='secondary'></div>
      <input type='radio' id='use_dns' name='choices' value = '3'>Use Static IP with DNS<br>
    <br>
    <input type='submit' id='Connect' value='Connect'>
   </div>
  </form>
  <div id="scanResults">
  </div>

  <script>
    document.getElementById("use_static").onclick = function() {
      document.getElementById("staticoptions").style.opacity = 1.0;
    }
    document.getElementById("use_dns").onclick = function() {
      document.getElementById("staticoptions").style.opacity = 1.0;
      document.getElementById("staticextoptions").style.opacity = 1.0;
    }
    document.getElementById("defaultC").onclick = function() {
      document.getElementById("staticoptions").style.opacity = 0.3;
      document.getElementById("staticextoptions").style.opacity = 0.3;
    }
  </script>
)=====";

const char connect_page2[] PROGMEM = R"=====(
  <div id="scanResults">
  </div>
</body>
</html>
)=====";
/* Auto-reconnect?
      <input type='checkbox' id='rc' name='rc'><br>
    Save?
      <input type='checkbox' id='save' name='save'>

      */
