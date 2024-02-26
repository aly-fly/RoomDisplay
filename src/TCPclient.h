#ifndef TCPCLIENT_H
#define TCPCLIENT_H

bool TCPclientConnect(void);
bool TCPclientRequest(const char Text[]);
void TCPclientDisconnect(void);

extern String TCPresponse;

#endif // TCPCLIENT_H