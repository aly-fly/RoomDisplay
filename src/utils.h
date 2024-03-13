#ifndef UTILS_H
#define UTILS_H

void TrimNumDot (String& Str);
void TrimAlfaNum (String& Str);

String FindJsonParam(String& inStr, String needParam, int& Position);

String FindXMLParam(String& inStr, String needParam, int& Position);

char        utf8ascii ( char ascii ) ;                             // Convert UTF to Ascii
void        utf8ascii_ip ( char* s ) ;                             // Convert UTF to Ascii in place
String      utf8ascii ( const char* s ) ;                          // Convert UTF to Ascii as String

#endif
