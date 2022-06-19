/**************************************************************************************************************************************
  EEERover Starter Example

  Based on AdvancedWebServer.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc

  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Forked and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Forked and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license

  Copyright (c) 2015, Majenko Technologies
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of Majenko Technologies nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************************************************************************/
#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <WiFiWebServer.h>


const char ssid[] = "iPhone Ezra";
const char pass[] = "12345678";


const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

//Webpage to return when root is requested
const char webpage[] = \
                       "<html><head><style>\
.btn {background-color: inherit;padding: 14px 28px;font-size: 16px;}\
.btn:hover {background: #eee;}\
</style></head>\
<body>\
<button class=\"btn\" onclick=\"ledOn()\">LED On</button>\
<button class=\"btn\" onclick=\"ledOff()\">LED Off</button>\
<button class=\"btn\" onclick=\"fetchData()\">Fetch Sensor Data</button>\
<br>LED STATE: <span id=\"ledState\">OFF</span>\
<br>ROCK DETECTED: <span id=\"detectionState\">Nothing Detected</span>\
<br>DETECTION HISTORY: <span id=\"history\">None</span>\
<br>Sensor Reading: <span id=\"sensorData\">None</span>\
</body>\
<script>\
var xhttp = new XMLHttpRequest();\
xhttp.onreadystatechange = function() {\
if (this.readyState == 4 && this.status == 200) {\
myObj = JSON.parse(this.responseText);\
document.getElementById(\"ledState\").innerHTML = myObj.ledState;\
document.getElementById(\"detectionState\").innerHTML = myObj.rockDetected;\
document.getElementById(\"sensorData\").innerHTML = myObj.sensorData;\
}\
};\
function ledOn() {xhttp.open(\"GET\", \"/on\"); xhttp.send();}\
function ledOff() {xhttp.open(\"GET\", \"/off\"); xhttp.send();}\
function fetchData() {xhttp.open(\"GET\", \"/fetching data\"); xhttp.send();}\
function reset() {xhttp.open(\"GET\", \"/reset\"); xhttp.send();}\
</script></html>";

WiFiWebServer server(80);


String Infrared_freq = "0";
String Radio_freq = "0";
String Acoustic_freq = "0";
String Magnetic_volt = "0";

// UltraSound variable initialisation
const int pulsePin = 6; // Input signal connected to D6 of Metro M0 Express
int pulseHigh; // Integer variable to capture High time of the incoming pulse
int pulseLow; // Integer variable to capture Low time of the incoming pulse
float pulseTotal; // Float variable to capture Total time of the incoming pulse
float acoustic_frequency = 0; // Calculated Frequency

//Magnetic Sensor Pin
int magnetic_value = 0;

//Radio Sensor variable initialisation
const int Radio_Pin = 11;
const int Radio_Interrupt_Pin = digitalPinToInterrupt(Radio_Pin);
float radio_frequency = 0;
unsigned long radio_newData = 0;
unsigned long radio_oldData = 0;



//Infrared Sensor variable initialisation
const int Infrared_Pin = 12;
const int Infrared_Interrupt_Pin = digitalPinToInterrupt(Infrared_Pin);
float infrared_frequency = 0;
unsigned long infrared_newData = 0;
unsigned long infrared_oldData = 0;


// MOTORS:
// pin 2 and 7 -> DIR signal pins (left, right)
// pin 3 and 9 -> PWM (left, right)
int leftMotor_dir = 2;
int rightMotor_dir = 8;
int leftMotor_en = 3;
int rightMotor_en = 9; // prev 4


//Return the web page
void handleRoot()
{
  server.send(200, F("text/html"), webpage);
  Serial.println("\nHandle Root");
}

//Switch LED on and acknowledge
void ledON()
{
  digitalWrite(LED_BUILTIN, 1);
  String message = "{\"ledState\":\"ON\", \"sensorData_infra\":" + Infrared_freq + ", \"sensorData_radio\":" + Radio_freq + ", \"sensorData_acoustic\":" + Acoustic_freq + ",\"sensorData_magnet\":" + Magnetic_volt + "}";
  server.send(200, "text/plain", message);
}

//Switch LED on and acknowledge
void ledOFF()
{
  digitalWrite(LED_BUILTIN, 0);
  String message = "{\"ledState\":\"OFF\", \"sensorData_infra\":" + Infrared_freq + ", \"sensorData_radio\":" + Radio_freq + ", \"sensorData_acoustic\":" + Acoustic_freq + ",\"sensorData_magnet\":" + Magnetic_volt + "}";
  server.send(200, "text/plain", message);
}

void fetchData()
{
  String message = "{\"ledState\":\"ON\", \"sensorData_infra\":" + Infrared_freq + ", \"sensorData_radio\":" + Radio_freq + ", \"sensorData_acoustic\":" + Acoustic_freq + ",\"sensorData_magnet\":" + Magnetic_volt + "}";
  server.send(200, "text/plain", message);
  //Serial.println(message);
}

void radioPwmRegist(){        //interrupt service routine
  radio_oldData = radio_newData;
  radio_newData = micros();
}


void infraredPwmRegist(){        //interrupt service routine
  infrared_oldData = infrared_newData;
  infrared_newData = micros();
}

void radiofrequency() {
    detachInterrupt(Radio_Interrupt_Pin);                 //stop measurements and process first
    radio_frequency = 1000000 / (radio_newData - radio_oldData);
    Radio_freq = String(radio_frequency);
    attachInterrupt(Radio_Interrupt_Pin, radioPwmRegist, FALLING);                        //start measuring again
}

void magneticvoltage() {
  pinMode(A0, INPUT);
  magnetic_value = analogRead(A0);
  Magnetic_volt = String(magnetic_value);
}

void acousticfrequency() {
  pulseHigh = pulseIn(pulsePin, HIGH, 100);
  pulseLow = pulseIn(pulsePin, LOW, 100);
  pulseTotal = pulseHigh + pulseLow; // Time period of the pulse in microseconds
  if (pulseTotal == 0.00) {
    acoustic_frequency = 404;
    Acoustic_freq = String(acoustic_frequency); //Setting the acoustic frequency equal to 404 --> No pulse found
  } else {
    acoustic_frequency = 1000000 / pulseTotal;
    Acoustic_freq = String(acoustic_frequency); //Setting the acoustic frequency equal to the recovered sensor data
  }
}

void infraredfrequency() {
     detachInterrupt(Infrared_Interrupt_Pin);                 //stop measurements and process first 
     infrared_frequency = 1000000/(infrared_newData - infrared_oldData);
     Infrared_freq = String(infrared_frequency);
     attachInterrupt(Infrared_Interrupt_Pin, infraredPwmRegist, FALLING);  //start measuring again      
}


void reset() {
  acoustic_frequency = 0;
  magnetic_value = 0;
  Infrared_freq = "0";
  Radio_freq = "0";
  Acoustic_freq = "0";
  Magnetic_volt = "240";
  
}



void handleMove() {
  // speed values 0 to 100
  String leftSpeed_str = "";
  String rightSpeed_str = "";

  // default direction for both motors is forwards (0)
  int leftDir = 0;
  int rightDir = 0;

  // get arguemnts from URI:
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "leftSpeed") {
      leftSpeed_str = server.arg(i);
    } else if (server.argName(i) == "rightSpeed") {
      rightSpeed_str = server.arg(i);
    } else {
      server.send(404, F("text/plain"), F("Invalid Arguments"));
    }
  }

  if (leftSpeed_str[0] == '-') {
    // if left motor speed is negative, set dir to reverse:
    leftDir = 1;
    // remove negative symbol:
    leftSpeed_str.remove(0, 1);
  }
  if (rightSpeed_str[0] == '-') {
    rightDir = 1;
    rightSpeed_str.remove(0, 1);
  }
  // set left and right motor direction pin out to value dir:
  digitalWrite(leftMotor_dir, leftDir);
  digitalWrite(rightMotor_dir, rightDir);


  int leftSpeed_int = leftSpeed_str.toInt();
  int rightSpeed_int = rightSpeed_str.toInt();

  //Serial.println("\nArg = " + movSpeed_str);
  analogWrite(leftMotor_en, leftSpeed_int * 2.55);
  analogWrite(rightMotor_en, rightSpeed_int * 2.55);
  //String client_message = F(("\nLeft Speed = ",leftSpeed_str,"\nRight Speed = ",rightSpeed_str));
  String client_message = F(("\nMoving... "));
  server.send(200, F("text/plain"), client_message);

}

//Generate a 404 response with details of the failed request
void handleNotFound()
{
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
}

void setup()
{


  // Initialise with LED Off
  digitalWrite(LED_BUILTIN, 0);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // setup DIR, PWM pins for left and right motors:
  pinMode(leftMotor_dir, OUTPUT);
  pinMode(rightMotor_dir, OUTPUT);
  pinMode(leftMotor_en, OUTPUT);
  pinMode(rightMotor_en, OUTPUT);

  Serial.begin(9600);

  //Wait 10s for the serial connection before proceeding
  //This ensures you can see messages from startup() on the monitor
  //Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);

  Serial.println(F("\nStarting Web Server"));

  //Check WiFi shield is present
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("WiFi shield not present"));
    while (true);
  }

  //Configure the static IP address if group number is set
  if (groupNumber)
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  // attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  //Register the callbacks to respond to HTTP requests
  server.on(F("/"), handleRoot);
  server.on(F("/on"), ledON);
  server.on(F("/off"), ledOFF);
  server.on(F("/move"), handleMove);
  server.on(F("/getData"), fetchData);
  server.on(F("/reset"), reset);

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.print(F("HTTP server started @ "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));

  // Turn LED On once connected to network and webserver on
  digitalWrite(LED_BUILTIN, 1);
  //Setting up acoustic pulseIn
  pinMode(pulsePin, INPUT);
  //Setting up magnetic input pin
  pinMode(A0, INPUT);


  //Radio Wave Frequency
  pinMode(Radio_Pin, INPUT);
  attachInterrupt(Radio_Interrupt_Pin, radioPwmRegist, FALLING);

  //Infrared Wave Frequency
  pinMode(Infrared_Pin, INPUT);
  attachInterrupt(Infrared_Interrupt_Pin, infraredPwmRegist, FALLING);
}

//Call the server polling function in the main loop
void loop()
{

  server.handleClient();
  radiofrequency();
  infraredfrequency();
  acousticfrequency();
  magneticvoltage();


  // if wifi shield powered on but not connected to network, retry
  //Serial.println(WiFi.status());
  if (WiFi.status() == WL_DISCONNECTED)
  {
    // wait 10 secs before retrying
    delay(5000);
    digitalWrite(LED_BUILTIN, 0);
    WiFi.begin(ssid, pass);
    server.begin();
    Serial.print(F("HTTP server re-started @ "));
    Serial.println(static_cast<IPAddress>(WiFi.localIP()));
    Serial.print('.');

  }


}
