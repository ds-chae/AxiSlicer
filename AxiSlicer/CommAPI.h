extern SOCKET clientSocket;
extern HANDLE hCommPort;

void WritePacket();

#ifndef STX
#define	STX	0x02
#endif
#define	ETX	0x03
#define	DLE	0x0F

extern int commState;

#define	CS_READY		0
#define	CS_IL_ISSUED	1
#define	CS_IL_START		2
#define	CS_AL_ISSUED	3
#define	CS_AL_START		4
#define	CS_IN_YMODEM	5

void SendCmdPacket(int cmd, char* param);
void SendDatPacket(char* param, int len);
void GetRxDataPart();
int  recvComm(char* buf, int len, int timeout);
int  sendComm(char* buf, int len);
int  sendCommStr(char* buf);
void RecvRepPacket();
int IsCommConnected();
void CloseCommPort();
int ConnectTcpPort(char* ip, int port, TCHAR* msg);
int ConnectCommPort(LPCTSTR port, int ispeed, TCHAR* msg);
void CloseCommPort();
int SendPosCommand(int hpos, int vpos);
int readRxBuffer(unsigned char* buf, int size);
