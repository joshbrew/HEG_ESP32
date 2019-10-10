const char connect_page1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color:#707070;
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
</style>
</head>
<body>
 <div id='formContainer' class='container'>
  <h2>Connect to WiFi</h4>
  <form method ='post' action='/doConnect' enctype='multipart/form-data'>
    SSID:<br>
      <input type='text' id='ssid' name='ssid'><br>
    Password:<br>
      <input type='password' id='pw' name='pw'><br>
    For use with Static IP: <br>
    Static IP (e.g. 192.168.0.199):<br>
      <input type='text' id='static' name='static'><br>
    Gateway IP (e.g. 192.168.0.1):<br>
      <input type='text' id='gateway' name='gateway'><br>
    Subnet Mask (e.g. 255.255.255.0):<br>
      <input type='text' id='subnet' name='subnet'><br>
      <input type='radio' id='use_static' name='choices' value='0'>Use Static IP<br>
      <input type='radio' id='AP_ONLY' name='choices' value='1'>Access Point Only<br>
      <input type='radio' id='btSwitch' name='choices' value='2'>Use Bluetooth<br>
    <input type='submit' id='Connect' value='Connect'>
   </div>
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
