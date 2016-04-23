#include <ESP8266WiFi.h>
#include <WiFiConnector.h>
WiFiConnector *wifi;

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>
#include <DHT.h>
#include "declare.h"
#include "utility.c"


#define WIFI_SSID        "Connectify-pc"
#define WIFI_PASSPHARSE  "0816452400"

#include "init_wifi.h"


void init_hardware()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("BEGIN");
}


void showTemp(float temp,float hud) {
// text display tests
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("T=");
  display.print(temp);
  display.println("C");
  display.print("H=");
  display.print(hud);
  display.println("%");
  display.display();
  //display.clearDisplay();
}

void showTemp1(float temp,float hud) {

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("   Temp  ");
  
  display.setTextSize(3);                                 // Size4 = 5 digit , size3 = 7 digits
  //display.setTextColor(BLACK, WHITE);                   // 'inverted' text
  display.setTextColor(WHITE);
  display.setCursor(0,24);
 
  display.print(temp);
  display.println("C"); 
  display.display();
}

void showHud(float temp,float hud) {

  display.clearDisplay();  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(" Humidity ");
   
  display.setTextSize(3);                               // Size4 = 5 digit , size3 = 7 digits
  //display.setTextColor(BLACK, WHITE);                 // 'inverted' text
  display.setTextColor(WHITE);
  display.setCursor(0,24);
 
  display.print(hud);
  display.println("%");  
  display.display();
}

void handleRoot() {
  digitalWrite(BUILTIN_LED, 1);
  server.send(200, "text/plain", "hello from esp8266! 555");
  digitalWrite(BUILTIN_LED, 0);
}

void handleNotFound(){
  digitalWrite(BUILTIN_LED, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(BUILTIN_LED, 0);
}

void uploadThingsSpeak(float d1,float d2) {
  static const char* host = "api.thingspeak.com";
  static const char* apiKey = "XIE7DWLDBZZ79S5Q";

  

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  while(client.available()){
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  
  String url = "/update/";
  //  url += streamId;
  url += "?key=";
  url += apiKey;
  url += "&field1=";
  url += d1;
  url += "&field2=";
  url += d2;

  Serial.print("Requesting URL: ");
  Serial.println(url);


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  while(!client.available()) {
    delay(200); 
  }             

  while(client.available()){
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
               
}

void setup()   {                
  
  uint8_t SMARTCONFIG_PIN = 0;
  init_hardware();
  init_wifi(SMARTCONFIG_PIN);
  
  pinMode(SW_MODE,INPUT);  
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  
  dht.begin();            // dht begin

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x78>>1);
  // init done
  

  // Clear the buffer.
  display.clearDisplay();
    
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setCursor(0,0);
  display.println("  Weather  ");
  
  display.setTextSize(3);                                  //Size4 = 5 digit , size3 = 7 digits
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setTextColor(WHITE);
  display.setCursor(0,18);
  display.println("Station");
    
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setCursor(0,52);
  display.println("Version 0.4");
 
  display.display();
  //delay(2000);
  
  
  // WiFi Connect
  Serial.print("CONNECTING TO ");
  Serial.println(wifi->SSID() + ", " + wifi->psk());
  
  
  display.clearDisplay();
    
    display.setTextSize(2);
    display.setTextColor(WHITE);
    //display.setTextColor(BLACK, WHITE); // 'inverted' text
    display.setCursor(0,0);
    display.println("Connecting");

    

  wifi->on_connecting([&](const void* message)
  {
    char buffer[70];
    //sprintf(buffer, "[%d] connecting -> %s ", wifi->counter, (char*) message);
    //Serial.println(buffer);
        
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    //lp++;
    //if(lp>=20) break;
  });

  
  
  wifi->on_smartconfig_waiting([&](const void* message)
  {
    display.clearDisplay();    
    display.setTextSize(2);       display.setTextColor(WHITE);  
    display.setCursor(0,0);       display.println("Smart");  
    display.setCursor(0,18);      display.println("CONFIG");
    //display.setCursor(0,36);      display.println(WiFi.localIP());
  
    display.display();
       
  });
  
  

  wifi->on_connected([&](const void* message)
  {
    // Print the IP address
    Serial.print("WIFI CONNECTED ");
    Serial.println(WiFi.localIP());
  });

  wifi->connect();
  
  
  //WiFi.begin(ssid, password);
  Serial.println("");
  // Clear the buffer.
  display.clearDisplay();
    
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setCursor(0,0);
  display.println("Connecting");
  

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {    
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    lp++;
    if(lp>=20) break;
  }
  
  
  Serial.println("");
  Serial.print("Connected to ");
  //Serial.println(ssid);
  Serial.print("IP address: ");  Serial.println(WiFi.localIP());
  
  display.clearDisplay();    
  display.setTextSize(2);       display.setTextColor(WHITE);  
  display.setCursor(0,0);       display.println(wifi->SSID());  
  display.setCursor(0,18);      display.println(WiFi.localIP());
  //display.setCursor(0,36);      display.println(WiFi.localIP());
  
  display.display();
  delay(2000);
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);
  
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  
  
}//setup




void loop() {
 
  buttonState = digitalRead(SW_MODE);
  
  if(buttonState == LOW){   
    imode++;
    
    if(imode >= 2) imode = 0; 
    Serial.println("Mode "); 
    Serial.print(imode);
  }
  
 
   server.handleClient(); 
  
   unsigned long currentMillis = millis();
   unsigned long currentMillis_led = millis();
    
  if(currentMillis - previousMillis >= interval) {     
      previousMillis = currentMillis;
    
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius
      float t = dht.readTemperature();
    
       
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,22);
        display.println("Sensor");
        display.println("Error !!!");
        display.display();

        h = 0; t = 0;
        //delay(2000);
        //return;
      }
      
      // show in serial monitor
        Serial.print("Humidity: "); 
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: "); 
        Serial.print(t);
        Serial.print(" *C \n");
      
      if (showState == LOW){
        showState = HIGH; 
        showHud(t,h);        
      }
      else{    
         showState = LOW; 
         showTemp1(t,h);                      // show temp
      }
      
      lp++;      
      if(lp >= 10){
        lp=0;
        uploadThingsSpeak(h,t);        
      }    
  }
  
 
}//Loop




