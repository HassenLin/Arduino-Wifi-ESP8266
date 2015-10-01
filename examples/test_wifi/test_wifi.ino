#include <Wire.h>
#include <ScrollLCD_I2C.h>
//#define CDC_Debug
#include "ESP8266.h"
#include "dht11.h"
#define SSID        "DSC_ADSL_TPLINK_ATC"
#define PASSWORD    "22988120"

ScrollLCD_I2C lcd(0x27, 16, 2); // set the LCD address to 0x3F for a 20 chars and 4


ESP8266 wifi(Serial);
dht11 DHT11;

void setup() {
  lcd.begin();
  lcd.print("Init");
  lcd.setCursor(0, 0);
  wifi.begin();
  wifi.setSoftAPParam("ESP8266", "01234567");
  if (wifi.setOprToStationSoftAP()) {
    wifi.setDHCP(0, 1);
    if (wifi.joinAP(SSID, PASSWORD)) {
      lcd.print("IP:");
      lcd.print(wifi.getLocalIP().c_str());
      wifi.enableMUX();
      wifi.startTCPServer(80);
      wifi.setTCPServerTimeout(10);
    } else {
      lcd.print("Join AP failure");
    }
  } else {
    lcd.print("station sap err");
  }
}


void loop() {

  uint8_t buffer[128] = {0};
  uint8_t mux_id;
  uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
  if (len > 0) {
    lcd.clear();
    lcd.print("Status:[");
    lcd.print(wifi.getIPStatus().c_str());
    lcd.print("]");

    lcd.print("Received from :");
    lcd.print(mux_id);
    lcd.print("[");
    for (uint32_t i = 0; i < len; i++) {
      lcd.print((char)buffer[i]);
    }
    lcd.print("]\r\n");
    WiFi_Print client(&wifi, mux_id);
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
      lcd.print("release tcp ");
      lcd.print(mux_id);
      lcd.println(" ok");
    } else {
      lcd.print("release tcp");
      lcd.print(mux_id);
      lcd.println(" err");
    }

    lcd.print("Status:[");
    lcd.print(wifi.getIPStatus().c_str());
    lcd.println("]");
  }
  delay(10);
}
