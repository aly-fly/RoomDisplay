#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include "display.h"

// reference: C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\WiFi\examples\WiFiClientBasic\WiFiClientBasic.ino

// Use WiFiClient class to create TCP connections
WiFiClient client;
String TCPresponse;

bool TCPclientConnect(void) {
    if (WifiState != connected) {
        return false;
    }
    
    DisplayText("Client ");
    DisplayText(CLIENT1_HOST);
    DisplayText("\n");

    Serial.print("Connecting to ");
    Serial.println(CLIENT1_HOST);

    if (client.connect(CLIENT1_HOST, CLIENT1_PORT)) {
        Serial.println("Connected.");
        if (!client.connected()) {
            Serial.println("... and connection dropped.");
            return false;
        }
        DisplayText("OK\n");
        // check if server sent any welcome messages
        delay(200);
        String line = client.readString();
        Serial.print("TCP server connect message: ");
        Serial.println(line);
        return true;
    } else {
        DisplayText("FAIL!\n");
        Serial.println("Connection failed.");
        return false;
    }
}

bool TCPclientRequest(const char Text[]) {
    if (!client.connected()) {
        DisplayClear();
        if (!TCPclientConnect()) {
            return false;
        }
    }
    Serial.print("TCP request: ");
    Serial.println(Text);

    // This will send a request to the server
    client.println(Text);  // \n is required at the end

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000)
  {
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0)
  {
    //read back one line from the server
    TCPresponse = client.readStringUntil('\n');
    // clean the received data
    // TCPresponse.trim();    
    TCPresponse.remove(TCPresponse.indexOf(" "));

    Serial.print("TCP reply: ");
    Serial.println(TCPresponse);
    return true;
  }
  else
  {
    Serial.println("TCP client timeout ");
    return false;
  }
}

void TCPclientDisconnect(void) {
    Serial.println("Closing connection.");
    client.stop();
}