const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color: #707070;
  font-family: Console, Lucida, monospace;
  color: white;
}
h1, h3 {
  text-align: center;
}
hegnav {
  position: relative;
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
  border: 2px solid lime;
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
  border: 2px solid turquoise;
  border-radius: 8px;
  padding: 8px;
}
.button {
  border: none;
  border-radius: 12px;
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
  background-color: lime;
}
.csbutton{
  background-color: tomato;
}
.hbutton{
  background-color: turquoise;
}

</style>
</head>
<body>
<h3>ESP32 web server demo</h3>
<h1>HEG Alpha Web Nav</h1>
<div id="HEG_NAV" class="hegnav" align="center">
  <form method="get" class="cvform" action="/listen">HTML5 Web Demo: <button class="button cvbutton" type="submit">GO</button></form><br>
  <form  method="get" class="csform" action="/connect">Connection Settings: <button class="button csbutton" type="submit">GO</button></form><br>
  <form method="get" class="uform" action="/update">Update: <button class="button csbutton" type="submit">GO</button></form><br>
  <form method="get" class="hform" action="/help">Help (WIP): <button class="button hbutton" type="submit">GO</button></form>
  <!--<form class="scform" method="get" action="/sc">StateChanger Demo (WIP):<button class="button scbutton" type="submit">GO</button></form><br>-->
</div>
<div id="version" style="position:absolute; right:5px; bottom:5px">
  Alpha v0.1.2. Jan 31, 2019.
</div>

</body>
</html>
)=====";
