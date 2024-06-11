
#include "Arduino.h"
#include "utils.h"

void TrimOnlyPrintable (String& Str) {
  Str.trim(); // remove leading and trailing spaces
  for (int16_t i = 0; i < Str.length(); i++)
  {
    char c = Str.charAt(i);
    if ((c < 32) || (c > 126)) {
      Str.remove(i, 1);
      i--;
      }
  }  
}

void TrimNumDot (String& Str) {
  Str.trim(); // remove leading and trailing spaces
  TrimOnlyPrintable(Str);
  for (int16_t i = 0; i < Str.length(); i++)
  {
    char c = Str.charAt(i);
    if (((c < '0') || (c > '9')) && (c != '.')) {
      Str.remove(i, 1);
      i--;
      }
  }  
}

void TrimAlfaNum (String& Str) {
  bool validChar;
  Str.trim(); // remove leading and trailing spaces
  TrimOnlyPrintable(Str);
  for (uint16_t i = 0; i < Str.length(); i++)
  {
    char c = Str.charAt(i);
    validChar = ((c >= '0') && (c <= '9')) ||
                ((c >= 'A') && (c <= 'Z')) ||
                ((c >= 'a') && (c <= 'z'));
    if (!validChar) {
      Str.remove(i, 1);
      i--;
      }
  }  
}

// remove non-printable chars and double spaces
void TrimDoubleSpaces (String& Str) {
  if (Str.length() < 2) return;
  bool Found;
  Str.trim(); // remove leading and trailing spaces
  char c1, c2;
  unsigned int i = 0;
  unsigned int len = Str.length()-1;
  while (i < len) {
    c1 = Str.charAt(i);
    c2 = Str.charAt(i+1);
    Found = ((c1 == ' ') && (c2 == ' '));
    i++;
    if (Found) {
      Str.remove(i, 1);
      i--;
      len--;
      }
  }  
}


String FindJsonParam(String& inStr, String needParam, int& Position)
{
    int indexStart = inStr.indexOf(needParam, Position);
    Position = -1; // status "not found" until found
    if (indexStart > 0) {
        int indexStop = inStr.indexOf(",", indexStart);
        Position = indexStop;
        if (indexStop <= indexStart) {
          Position = -1; // status "not found"
          return "/";
        }
        int CountChar = needParam.length();
        return inStr.substring(indexStart+CountChar+2, indexStop);
    }
    return "/";
}


String FindXMLParam(String& inStr, String needParam, int& Position)
{
    String searchString = "<"+needParam+">";
    int indexStart = inStr.indexOf(searchString, Position);
    Position = -1; // status "not found" until found
    if (indexStart > 0) {
        int indexStop = inStr.indexOf("</"+needParam+">", indexStart);  
        Position = indexStop;
        if (indexStop <= indexStart) {
          Position = -1; // status "not found"
          return "/";
        }
        int CountChar = needParam.length();
        return inStr.substring(indexStart+CountChar+2, indexStop);
    }
    return "/";
}

//**************************************************************************************************
//                                      U T F 8 A S C I I                                          *
//**************************************************************************************************
// UTF8-Decoder: convert UTF8-string to extended ASCII.                                            *
// Convert a single Character from UTF8 to Extended ASCII.                                         *
// Return "0" if a byte has to be ignored.                                                         *
//**************************************************************************************************
char utf8ascii ( char ascii )
{
  static const char lut_C3[] = { "AAAAAAACEEEEIIIIDNOOOOO#0UUUU###"
                                 "aaaaaaaceeeeiiiidnooooo##uuuuyyy" } ; 
  static const char lut_C4[] = { "AaAaAaCcCcCcCcDdDdEeEeEeEeEeGgGg"
                                 "GgGgHhHhIiIiIiIiIiJjJjKkkLlLlLlL" } ;
  static const char lut_C5[] = { "lLlNnNnNnnnnOoOoOoOoRrRrRrSsSsSs"
                                 "SsTtTtTtUuUuUuUuUuUuWwYyYZzZzZzs" } ;

  static char       c1 ;              // Last character buffer
  char              res = '\0' ;      // Result, default 0

  if ( ascii <= 0x7F )                // Standard ASCII-set 0..0x7F handling
  {
    c1 = 0 ;
    res = ascii ;                     // Return unmodified
  }
  else
  {
    switch ( c1 )                     // Conversion depending on first UTF8-character
    {
      case 0xC2: res = '~' ;
        break ;
      case 0xC3: res = lut_C3[ascii - 128] ;
        break ;
      case 0xC4: res = lut_C4[ascii - 128] ;
        break ;
      case 0xC5: res = lut_C5[ascii - 128] ;
        break ;
      case 0x82: if ( ascii == 0xAC )
        {
          res = 'E' ;                 // Special case Euro-symbol
        }
    }
    c1 = ascii ;                      // Remember actual character
  }
  return res ;                        // Otherwise: return zero, if character has to be ignored
}


//**************************************************************************************************
//                                U T F 8 A S C I I _ I P                                          *
//**************************************************************************************************
// In Place conversion UTF8-string to Extended ASCII (ASCII is shorter!).                          *
//**************************************************************************************************
void utf8ascii_ip ( char* s )
{
  int  i, k = 0 ;                     // Indexes for in en out string
  char c ;

  for ( i = 0 ; s[i] ; i++ )          // For every input character
  {
    c = utf8ascii ( s[i] ) ;          // Translate if necessary
    if ( c )                          // Good translation?
    {
      s[k++] = c ;                    // Yes, put in output string
    }
  }
  s[k] = 0 ;                          // Take care of delimeter
}


//**************************************************************************************************
//                                      U T F 8 A S C I I                                          *
//**************************************************************************************************
// Conversion UTF8-String to Extended ASCII String.                                                *
//**************************************************************************************************
String utf8ascii ( const char* s )
{
  int  i ;                            // Index for input string
  char c ;
  String res = "" ;                   // Result string

  for ( i = 0 ; s[i] ; i++ )          // For every input character
  {
    c = utf8ascii ( s[i] ) ;          // Translate if necessary
    if ( c )                          // Good translation?
    {
      res += String ( c ) ;           // Yes, put in output string
    }
  }
  return res ;
}

