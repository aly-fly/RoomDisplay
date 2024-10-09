#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include <utils.h>
#include "Clock.h"
#include "eAsistent_https_certificate.h"
#include "display.h"
#include "GlobalVariables.h"

// https://www.easistent.com/urniki/izpis/12296fadef0fe622b9f04637b58002abc872259c/600999/0/0/0/6/9421462

const String eAsistent_URL1 = "https://www.easistent.com/urniki/izpis/12296fadef0fe622b9f04637b58002abc872259c/";
const String eAsistent_URL2 = "/0/0/0/";

unsigned long LastTimeUrnik1Refreshed = 0;
unsigned long LastTimeUrnik2Refreshed = 0;

// Class, day, hour
String Urnik[2][6][10];
String RxBuffer;

bool ReadEAsistentWebsite(int teden, String ucenec) {
  Serial.println("ReadEAsistentWebsite()");
  bool result = false;
  RxBuffer.clear();
  if (!WiFi.isConnected()) {
      return false;
  }

  setClock(); 

  DisplayClear();
  String razred;
  if (ucenec == "9421462") razred = "600999";
  if (ucenec == "9621355") razred = "600887";
  String URL = eAsistent_URL1 + razred + eAsistent_URL2 + '/' + String(teden) + '/' + ucenec;
  DisplayText("Contacting: ");
  DisplayText(URL.c_str());
  DisplayText("\n");
  String sBufOld;

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client -> setCACert(rootCACertificate_eAsistent);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      DisplayText("HTTPS begin\n");
      if (https.begin(*client, URL)) {  // HTTPS
        yield(); // watchdog reset
        Serial.print("[HTTPS] GET...\r\n");
        DisplayText("HTTPS get request: ");
        // start connection and send HTTP header
        int httpCode = https.GET();
        yield(); // watchdog reset
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
          DisplayText(String(httpCode).c_str());
          DisplayText("\n");
  
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
                Serial.println("[HTTPS] Code OK");
                DisplayText("Code OK\n");

// stream data in chunks
                // get length of document (is -1 when Server sends no Content-Length header)
                int DocumentLength = https.getSize();
                Serial.printf("[eAsistent] Document size: %d \r\n", DocumentLength);
                // get tcp stream
                WiFiClient * stream = https.getStreamPtr();
                // jump to the header
                Serial.println("Searching for the header...");
                DisplayText("Searching for the header...\n");
                bool HeaderFound = stream->find("<table");
                Serial.print("Header found: ");
                Serial.println(HeaderFound);
                if (HeaderFound) {
                  RxBuffer.clear();
                  Serial.println("Reading data from server...");
                  DisplayText("Reading data... ");
                  //RxBuffer = stream->readStringReducedSpaces();
                  RxBuffer = stream->readString();
                  Serial.print("Data read (bytes): ");
                  Serial.println(RxBuffer.length());
                  DisplayText(String(RxBuffer.length() / 1024).c_str());
                  DisplayText(" kB\n");
                  TrimDoubleSpaces(RxBuffer);
                  TrimDoubleChars(RxBuffer, TAB);
                  Serial.println("----------------");
                  //Serial.println(RxBuffer);
                  Serial.println("----------------");
                  result = true;
                } // header found
          } // HTTP code > 0
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
          DisplayText("Error: ");
          DisplayText(https.errorToString(httpCode).c_str());
          DisplayText("\n");
        }
  
        https.end();
      } else {
        Serial.println("[HTTPS] Unable to connect");
        DisplayText("Unable to connect.\n");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("Unable to create HTTPS client");
  }

  sBufOld.clear(); // free mem
  if (result){
    Serial.println("Website read OK");
    DisplayText("Website read OK\n", CLGREEN);
  } else {
    Serial.println("Website read FAILED");
    DisplayText("Website read FAILED\n", CLRED);
    delay(2000);
  }
  return result;
}

//##################################################################################################################
//##################################################################################################################
//##################################################################################################################
//##################################################################################################################

void ProcessData (int urnikNr, String sName) {
  int txtEnd_sectionStart, sectionEnd_txtStart, dan, ura;
  String section, txt, txt_utf, celica;
  bool subTable = false;
  int subTableNum = 0;
  bool odpadlaUra;

  txtEnd_sectionStart = 0;
  sectionEnd_txtStart = 0;
  Serial.println();
  Serial.println("-----------------");
  Serial.println("-----------------");
  Serial.println("-----------------");
  RxBuffer.setCharAt(0, '<');  // start with a fake section with random data
  dan = 0;
  ura = 0;
  celica.clear();
  odpadlaUra = false;
  while (txtEnd_sectionStart > -1)
  {
    txt.clear();
    section.clear();
    txtEnd_sectionStart = RxBuffer.indexOf('<', sectionEnd_txtStart);
    if (sectionEnd_txtStart+1 < txtEnd_sectionStart) {
      txt_utf = RxBuffer.substring(sectionEnd_txtStart+1, txtEnd_sectionStart);
      txt = utf8ascii(txt_utf.c_str());
    }
    txt.replace("Zgodovinski", "Zgo.");
    txt.replace("Geografski", "Geo.");
    txt.replace("krozek", "k.");
    txt.replace("Slovenscina", "SL");
    txt.replace("Anglescina", "AN");
    txt.replace("Matematika", "MT");
    txt.replace("Gospodinjstvo", "GS");
    txt.replace("Tehnika in tehnologija", "TT");
    txt.replace("Sport", "SP");

    txt.replace("&nbsp;", " ");
    txt.replace(TAB, SPACE);
    txt.trim(); // remove leading and trailing spaces
    if (txt.length() > 0) {
      txt.concat(' '); // just one space at the end
      TrimDoubleSpaces(txt);
      Serial.println(txt);
      celica.concat(txt);
    }
    // text is now in the cell, do not use txt anymore from here down.

    sectionEnd_txtStart = RxBuffer.indexOf('>', txtEnd_sectionStart);
    if (txtEnd_sectionStart+1 < sectionEnd_txtStart)
      section = RxBuffer.substring(txtEnd_sectionStart+1, sectionEnd_txtStart);
    //Serial.print("(((");
    //Serial.println(section);

    if (section.indexOf("ednevnik_seznam_ur_odpadlo") > 0) {
      odpadlaUra = true;
      Serial.println("ODPADLO");
      }

    if (section.indexOf("table")  == 0) {
      subTable = true; 
      subTableNum++;
      Serial.print("   Sub Table ");
      Serial.println(subTableNum);
      if (subTableNum > 1) {
        // korigiraj odpadle ure
        if (odpadlaUra) {
          int pp = celica.lastIndexOf('&');
          if (pp >= 0) {  // naslednja sekcija iste celice
            celica.remove(pp+1);
            celica.concat(" ODPADLO");
          } else {
            celica = "ODPADLO ";
          }
          odpadlaUra = false;
        }
        celica.concat (" & ");
      }
    }

    if (section.indexOf("/table") == 0) {
      subTable = false; 
      Serial.println("   Sub Table end");
      }

    if (subTable == false) { 
        if (section == "/tr") { // konec vrstice --> vpis podatkov v urnik
          // korigiraj odpadle ure
          if (odpadlaUra) {
            int pp = celica.lastIndexOf('&');
            if (pp >= 0) {  // naslednja sekcija iste celice
              celica.remove(pp+1);
              celica.concat(" ODPADLO");
            } else {
              celica = "ODPADLO ";
            }
            odpadlaUra = false;
          }
        celica.trim(); // remove leading and trailing spaces
        TrimDoubleSpaces(celica);
        TrimNonPrintable(celica);
        if ((dan < 6) && (ura < 10)) Urnik[urnikNr][dan][ura] = celica;
        celica.clear();
        subTableNum = 0;
        ura++;
        dan = 0;
        Serial.println("\n=============================");
      }
      if ((section.indexOf("/td") == 0) || (section.indexOf("/th") == 0)) { // konec celice --> vpis podatkov v urnik
        // korigiraj odpadle ure
        if (odpadlaUra) {
          int pp = celica.lastIndexOf('&');
          if (pp >= 0) {  // naslednja sekcija iste celice
            celica.remove(pp+1);
            celica.concat(" ODPADLO");
          } else {
            celica = "ODPADLO ";
          }
          odpadlaUra = false;
        }
        celica.trim(); // remove leading and trailing spaces
        TrimDoubleSpaces(celica);
        TrimNonPrintable(celica);
        if ((dan < 6) && (ura < 10)) Urnik[urnikNr][dan][ura] = celica;
        celica.clear();
        subTableNum = 0;
        dan++;
        Serial.println("-----------");
      }
    } // sub table
  } // while
  RxBuffer.clear();

  for (dan = 0; dan < 6; dan++) {
    Urnik[urnikNr][dan][0].concat(" " + sName);
  }

  Serial.println("++++++++++++++++++++");
  
  for (ura = 0; ura < 10; ura++) {
    for (dan = 0; dan < 6; dan++) {
      Serial.print(Urnik[urnikNr][dan][ura]);
      Serial.print("  |  ");
    }
    Serial.println();
  }
  Serial.println("######################");
}

//##################################################################################################################
//##################################################################################################################
//##################################################################################################################
//##################################################################################################################

/*
void eAsistentInit(void) {
  int urnikNr, dan, ura;  
  for (urnikNr = 0; urnikNr < 2; urnikNr++) 
    for (dan = 0; dan < 6; dan++) 
      for (ura = 0; ura < 10; ura++) 
        Urnik[urnikNr][dan][ura].reserve(25);
}
*/

void GetEAsistent(void) {
  Serial.println("GetEAsistent()");

  if (GetCurrentTime()) {
  } else {
    Serial.println("Error getting current time!");
  }

  // calculate week number
  struct tm sStartTime;
  time_t StartTime, CurrTime;
  time_t TdiffSec;
  int currentWeek;
  // 1.9.2024 = sunday
  sStartTime.tm_year = 2024 - 1900;
  sStartTime.tm_mon = 9 - 1;
  sStartTime.tm_mday = 1;
  sStartTime.tm_hour = 0;
  sStartTime.tm_min = 0;
  StartTime = mktime(&sStartTime);
  time(&CurrTime);
  TdiffSec = time_t(difftime(CurrTime, StartTime));
  Serial.print("Seconds elapsed from school year start: ");
  Serial.println(TdiffSec);
  currentWeek = TdiffSec / (7 * 24 * 60 * 60);
  currentWeek++; // starts with 1
  Serial.print("Current week from school year start: ");
  Serial.println(currentWeek);


  if ((millis() < (LastTimeUrnik1Refreshed + 2*60*60*1000)) && (LastTimeUrnik1Refreshed != 0)) {  // check server every 2 hours
    Serial.println("Urnik 1: Data is valid.");
  } else {
    if (ReadEAsistentWebsite(currentWeek, "9621355")) {
      ProcessData(0, "TINKARA");
      LastTimeUrnik1Refreshed = millis();
    }
  }

  if ((millis() < (LastTimeUrnik2Refreshed + 2*60*60*1000)) && (LastTimeUrnik2Refreshed != 0)) {  // check server every 2 hours
    Serial.println("Urnik 2: Data is valid.");
  } else {
    if (ReadEAsistentWebsite(currentWeek, "9421462")) {
      ProcessData(1, "MARCEL");
      LastTimeUrnik2Refreshed = millis();
    }
  }
}








void DrawEAsistent(int urnikNr) {
  Serial.println("DrawEAsistent()");
  DisplayClear();
  if (GetCurrentTime()) {
    Serial.print("Today = ");
    Serial.println(DAYSF[CurrentWeekday-1]);
    Serial.print("Hour = ");
    Serial.println(CurrentHour);
  } else {
    Serial.println("Error getting current time!");
  }
  int dayToShow = CurrentWeekday;

  if (CurrentWeekday < 6) { // work day 
    if ((CurrentHour > 16) && (CurrentWeekday < 4)) {
    // show next day
      dayToShow++;
      Serial.println("day++");
    }
  }

  if (CurrentWeekday > 5) { // weekend
    dayToShow = 1; // monday
  }

  uint16_t color;
  // process data for that day
  if (dayToShow > 0) {
    for (int i = 0; i < 10; i++)
    {
      if ((i == 0) && (urnikNr == 0)) color = CLPINK; else 
      if ((i == 0) && (urnikNr == 1)) color = CLGREEN; else 
      if (Urnik[urnikNr][dayToShow][i].indexOf("ODPADLO") >= 0) color = CLRED; else
      if (i == 1) color = CLORANGE; else
      if (i % 2) color = CLLIGHTCYAN; else
        color = CLWHITE;
      DisplayText(Urnik[urnikNr][dayToShow][i].c_str(), 20, 0, i * 21, color, false);
    }
  }
    delay(1500);
}

