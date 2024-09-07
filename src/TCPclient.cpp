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
    Serial.println("TCPclientConnect()");
    bool result = false;
    if (!WiFi.isConnected()) {
        return false;
    }
    
    DisplayText("TCP socket: ");
    DisplayText(CLIENT1_HOST);
    DisplayText("\n");

    Serial.print("Connecting to ");
    Serial.println(CLIENT1_HOST);

    if (client.connect(CLIENT1_HOST, CLIENT1_PORT)) {
        Serial.println("Connected.");
        delay(300);
        if (!client.connected()) {
            DisplayText("CONN. DROPPED!\n", CLRED);
            Serial.println("... and connection dropped.");
            delay (1500);
            return false;
        }
        DisplayText("OK\n", CLGREEN);
        // check if server sent any welcome messages
        delay(300);
        String line = client.readString();
        Serial.print("TCP server connect message: ");
        Serial.println(line);
        result = true;
    } else {
        DisplayText("FAIL!\n", CLRED);
        delay(2000);
        Serial.println("Connection failed.");
        result = false;
    }
    delay (1000);
    return result;
}

bool TCPclientRequest(const char Text[]) {
    Serial.println("TCPclientRequest()");
    if (!client.connected()) {
        //DisplayClear();
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
