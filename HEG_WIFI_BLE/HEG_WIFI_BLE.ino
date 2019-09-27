//Requires these arduino libraries: Arduino ESP32 latest dev build via git, AsyncTCP, ESPAsyncWebServer, SavLay Filter, Arduino ADS1X15, and the different build options provided.
//By Joshua Brewster
//Contributors: Diego Schmaedech and many others indirectly. 

#include "WiFi_API.h" // WiFi settings and macros. HEG and BLE libraries linked through here.
unsigned long eventMillis = 0;

//===============================================================
// Setup
//===============================================================

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  setupHEG();
  delay(100);
  startADS();
  delay(100);
  EEPROM.begin(512);
  if(EEPROM.read(0) != 1){
    EEPROM.end();
    setupWiFi();
  }
  else {
    EEPROM.end();
    setupBLE();
  }
}


void loop(void){
  checkInput();
  HEG_core_loop();
  delayMicroseconds(1800);

  if(currentMillis - eventMillis >= 50){
    eventMillis = currentMillis;

    events.send(output.c_str(),"myevent",millis());
  }
}


/* Spaghetti
 * 
 * else{
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      Serial.print("Connecting to ");
      Serial.println(ssid);
    
      //Wait for WiFi to connect
      int wait = 0;
      while((WiFi.waitForConnectResult() != WL_CONNECTED)){
        if(wait >= 1){ break;}      
        Serial.print("...");
        wait++;
        delay(100);
      }
      if(WiFi.waitForConnectResult() == WL_CONNECTED){
        //If connection successful show IP address in serial monitor
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());  //IP address assigned to your ESP
      }
      else{
        WiFi.disconnect(true);
        Serial.println("Connection Failed.");
        Serial.print("Attempted at SSID: ");
        Serial.println(ssid);
      }
     }
 * 
 * 
 * 
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')"
    "}"
    "}"
"</script>"
 * 
 * 
 * ---xTaskCreate EXAMPLE---
 * 
 * void Task(void*param){
 *  while(taskComplete == false){
 *    //do things
 *    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // WAIT/YIELD FOR OTHER TASKS
 *  }
 * }
 * 
 * BaseType_t xReturned;
 * TaskHandle_t xHandle;
 * 
 * setup(){
 *  xTaskCreate(Task,"Task",16384,NULL,1,xHandle);
 * }
 * loop(){
 *  if(taskComplete){
 *    vTaskDelete(xHandle);
 *  }
 * }
 */
