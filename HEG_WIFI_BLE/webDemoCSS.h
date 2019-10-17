const char webDemoCSS[] PROGMEM = R"=====(
body { background-color: #707070; font-family: Arial, Helvetica, sans-serif; }
  msgDiv { display: none; }
  eventDiv { display: none; }
  input[type=text]{
    border: 2px solid red;
    border-radius: 4px;
    height: 30px;
    padding: 2px;
    font-size: 16px;
  }
  input[type=file], .button {
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
   .dattable {
      position: relative;
      width: 75%;
      min-width: 700px;
      table-layout: fixed;
   }
   th {
      background-color: darkslategray;
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   td {
      background-color: darkslategray;
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   .scoreth { color: honeydew; }
   .hegapi {
      position: absolute;
      width: 275px;
      height: 300px;
      top: 100px;
      left: 0%;
   }
   .startbutton { background-color: #4CAF50; position: absolute; left: 0%; }
   .stopbutton { background-color: #FF0000; position: absolute; left: 120px; }
   .sendcommand{ position: absolute; top: 70px;   }
   .sendbutton{ background-color: #0000FF; }
   .saveLoadBar{ position: absolute; top: 240px; }
   .saveLoadButtons{ background-color: teal; }
   .sensBar { position: absolute; top: 150px; color: white; }
   .label { padding: 4px; color: white; }
   .canvascss {
      position: absolute;
      top:100px;
      left:300px;
   }
   .webglcss {
     position: absolute;
     top: 550px;
     left: 10px;
     width: 75%;
     height: 200px;
     min-width: 400px;
   }
   .scale{
     position: absolute;
     bottom: 0px;
     right: 50px;
     color: white;
   }
   .menudiv{
    position: absolute;
    right: 0px;
    top: 100px;
    color: white;
   }
   .vidapi {
    position: absolute;
    top: 270px;
    right: 0px;
   }
   .vdfade {
    float: right;
    background-color: peachpuff;
   }
   .vdspeed {
    float: right;
    background-color:  palegoldenrod;
   }
   .cvbutton{
    background-color: chartreuse;
   }
   .vdbutton{
    background-color: royalblue;
   }
   .aubutton{
    background-color: tomato;
   }
   .lfbutton{
    background-color: plum;
   }
   .dummy {
      position: absolute;
      top: 130px;
      left:325px;
      width:0;
      height:0;
      border:0; 
      border:none;
   }
)=====";
