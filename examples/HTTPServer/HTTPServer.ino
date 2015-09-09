//#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#define CDC_Debug
#include <SoftwareSerial.h>
#include "ESP8266.h"
#include "dht11.h"
#define SSID        "DSC_ADSL_TPLINK_ATC"
#define PASSWORD    "22988120"

#define rxPin 7
#define txPin 8

//LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x3F for a 20 chars and 4


SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
ESP8266 wifi(mySerial,9600);
dht11 DHT11;
#ifdef CDC_Debug
  #define std_print(x)  Serial.print(x)
  #define std_println(x) Serial.println(x)
#else
  #define std_print(x) 
  #define std_println(x)
#endif

void setup() {
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  


  //lcd.begin();
  //lcd.backlight();
#ifdef CDC_Debug  
  Serial.begin(115200);
  while(!Serial);
#endif  
  
  std_print("setup begin\r\n");
  
  mySerial.write("AT+CIOBAUD=57600\r\n");
  mySerial.end();
  mySerial.begin(57600);
  
  std_println("FW Version:");
  std_println(wifi.getVersion().c_str());
  if (wifi.setOprToStationSoftAP()) {
    std_print("to station + softap ok\r\n");
  } else {
    std_print("to station + softap err\r\n");
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    std_print("Join AP success\r\n");
    std_print("IP: ");
    std_println(wifi.getLocalIP().c_str());
  } else {
    std_print("Join AP failure\r\n");
  }

  if (wifi.enableMUX()) {
    std_print("multiple ok\r\n");
  } else {
    std_print("multiple err\r\n");
  }

  if (wifi.startTCPServer(80)) {
    std_print("start tcp server ok\r\n");
  } else {
    std_print("start tcp server err\r\n");
  }

  if (wifi.setTCPServerTimeout(10)) {
    std_print("set tcp server timout 10 seconds\r\n");
  } else {
    std_print("set tcp server timout err\r\n");
  }

  std_print("setup end\r\n");
}


void loop() {

  uint8_t buffer[128] = {0};
  uint8_t mux_id;
  uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
  if (len > 0) {
    std_print("Status:[");
    std_print(wifi.getIPStatus().c_str());
    std_println("]");

    std_print("Received from :");
    std_print(mux_id);
    std_print("[");
    for (uint32_t i = 0; i < len; i++) {
      std_print((char)buffer[i]);
    }
    std_print("]\r\n");
    WiFi_Print client(wifi,mux_id);
         client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html><body>");
          DHT11.read(2);
          client.print(DHT11.temperature);
          client.print("'C<br>");
          client.print(DHT11.humidity);
          client.print("%");
/*
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          */
          client.println("</body></html>");
    if (wifi.releaseTCP(mux_id)) {
      std_print("release tcp ");
      std_print(mux_id);
      std_println(" ok");
    } else {
      std_print("release tcp");
      std_print(mux_id);
      std_println(" err");
    }

    std_print("Status:[");
    std_print(wifi.getIPStatus().c_str());
    std_println("]");
  }
  delay(10);
}
