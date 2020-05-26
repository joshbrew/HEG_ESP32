const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color: #303030;
  font-family: Console, Lucida, monospace;
  color: white;
}
h1, h3 {
  text-align: center;
}
.hegnav {
  position: relative;
  width:50%;
  margin-left: auto;
  margin-right: auto;
  text-align: center;
}
form {
  font-size: 20px;
  font-weight: bold;
}
.scform {
  border: 2px solid royalblue;
  border-radius: 8px;
  padding: 8px;
}
.cvform{
  border: 2px solid greenyellow;
  border-radius: 8px;
  padding: 8px;
}
.csform{
  border: 2px solid tomato;
  border-radius: 8px;
  padding: 8px;
}
.uform{
  border: 2px solid tomato;
  border-radius: 8px;
  padding: 8px;
}
.hform{
  border: 2px solid rgb(64, 205, 224);
  border-radius: 8px;
  padding: 8px;
}
.button {
  border: none;
  width: 100%;
  border-radius: 12px;
  font-family: Console, Lucida, monospace;
  font-weight: bold;
  color: white;
  padding: 15px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 4px 2px;
  cursor: pointer;
}
.scbutton{
  background-color: gray; 
}
.cvbutton{
  background-color: rgb(26, 211, 57);
}
.cvbutton:hover {
  background-color: green;
}
.csbutton{
  background-color: tomato;
}
.csbutton:hover{
  background-color: red;
}
.hbutton{
  background-color: rgb(64, 205, 224);
}
.hbutton:hover{
  background-color: rgb(47, 85, 255);
}

</style>
</head>
<body>
<h3>ESP32 Web Server</h3>
<hr>
<h1>HEG Alpha Web Nav</h1>
<div id="HEG_NAV" class="hegnav" align="center">
  <form method="get" class="cvform" action="/listen"><button class="button cvbutton" type="submit">HTML5 Web App</button></form><br>
  <form  method="get" class="csform" action="/connect"><button class="button csbutton" type="submit">Connection Settings</button></form><br>
  <form method="get" class="uform" action="/update"><button class="button csbutton" type="submit">Update</button></form><br>
  <form method="get" class="hform" action="/help"><button class="button hbutton" type="submit">Help (WIP)</button></form>
  <!--<form class="scform" method="get" action="/sc">StateChanger Demo (WIP):<button class="button scbutton" type="submit">GO</button></form><br>-->
</div>
<div id="version" style="position:absolute; right:5px; bottom:5px">
  Alpha v0.1.9. May 25, 2020.
</div>

</body>
</html>
)=====";
