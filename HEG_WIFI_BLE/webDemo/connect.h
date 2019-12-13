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
      <input type='password' id='pw' name='pw'><br><br>
    Optional Static IP mode: <br>
    Static IP (e.g. 192.168.0.199, first 3 numbers must match gateway):<br>
      <input type='text' id='static' name='static'><br>
    Gateway IP (e.g. 192.168.0.1, check your router settings):<br>
      <input type='text' id='gateway' name='gateway'><br>
    Subnet Mask (e.g. 255.255.255.0):<br>
      <input type='text' id='subnet' name='subnet'><br>
    For use with Static IP with DNS mode:<br>
    Primary DNS:<br>
      <input type='text' id='primary' name='primary'><br>
    Secondary DNS (optional):<br>
      <input type='text' id='secondary' name='secondary'><br>
      <input type='radio' id='defaultC' name='choices' value='9' checked>Default Connect<br>
      <input type='radio' id='use_static' name='choices' value='0'>Use Static IP<br>
      <input type='radio' id='use_dns' name='choices' value = '3'>Use Static IP with DNS<br>
      <input type='radio' id='suggestIP' name='choices' value = '4'>Suggest Static IP<br>
      <input type='radio' id='AP_ONLY' name='choices' value='1'>Access Point Only<br>
      <input type='radio' id='btSwitch' name='choices' value='2'>Use Bluetooth<br>
    <input type='submit' id='Connect' value='Connect'>
   </div>
  </form>
  <div id="scanResults">
)=====";

const char connect_page2[] PROGMEM = R"=====(
  </div>
</body>
</html>
)=====";
/* Auto-reconnect?
      <input type='checkbox' id='rc' name='rc'><br>
    Save?
      <input type='checkbox' id='save' name='save'>

      */
