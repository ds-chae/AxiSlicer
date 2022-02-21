#include "stdafx.h"

#include "EnumSerial.h"
#include "winsock2.h"
#include "CommAPI.h"

SOCKET clientSocket = INVALID_SOCKET;
HANDLE hCommPort = INVALID_HANDLE_VALUE;

char TxPacket[2048];
int  TxLen = 0;

int commState = CS_READY;

void WritePacket()
{
	if(clientSocket != INVALID_SOCKET)
		send(clientSocket, TxPacket, TxLen, 0);

	if(hCommPort != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(hCommPort, TxPacket, TxLen, &dwio, NULL);
		WriteFile(hCommPort, "", 0, &dwio, NULL);
	}
}

int  sendComm(char* buf, int len)
{
	if(hCommPort == INVALID_HANDLE_VALUE)
		return -1;

	DWORD dwio = 0;
	for(int i = 0; i < len; i++) {
		WriteFile(hCommPort, buf+i, 1, &dwio, NULL);
		if(i != len-1) Sleep(2);
	}

	return len;
}

int  sendCommStr(char* buf)
{
	return sendComm(buf, strlen(buf));
}

#define	STX	0x02
#define	ETX	0x03
#define	DLE	0x0F

void SendCmdPacket(int cmd, char* param)
{
	TxLen = 0;
	TxPacket[TxLen++] = STX;
	int param_len = strlen(param);
	TxLen += sprintf_s(TxPacket+TxLen, sizeof(TxPacket)-TxLen, "%03d", param_len + 1);
	TxPacket[TxLen++] = cmd;
	memcpy_s(TxPacket+TxLen, sizeof(TxPacket)-TxLen, param, param_len);
	TxLen += param_len;
	TxPacket[TxLen++] = ETX;
	WritePacket();
}

#if CMD_API
void SendDatPacket(char* param, int len)
{
	TxLen = 0;
	TxPacket[TxLen++] = STX;
	TxLen += sprintf_s(TxPacket+TxLen, sizeof(TxPacket)-TxLen, "%03d", len + 1);
	TxPacket[TxLen++] = CMD_PUT_DATA;
	for(int i = 0; i < len; i++) {
		if(param[i] == ETX || param[i] == DLE)
			TxPacket[TxLen++] = DLE;
		TxPacket[TxLen++] = param[i];
	}
	TxPacket[TxLen++] = ETX;
	WritePacket();
}
#endif

char RxPacketDat[8192];
int  RxPacketLen = 0;
int  RxPacketSta = 0;

char RxDataPart[8192];
int  RxDataLen = 0;

void GetRxDataPart()
{
	RxDataLen = (RxPacketDat[1]-'0')*100 + (RxPacketDat[2]-'0')*10 + (RxPacketDat[3]-'0') - 1;
	memcpy_s(RxDataPart, sizeof(RxDataPart), RxPacketDat+5, RxDataLen);
	RxDataPart[RxDataLen] = 0;
}

char RecvBuf[16384];
int  RecvNext = 0;
int  RecvCount = 0;

#define	RX_BUFSIZE	8192

int RxHead = 0;
int RxTail = 0;
char RxBuffer[RX_BUFSIZE];

int readRxBuffer(unsigned char* buf, int size)
{
	int i = 0;
	int tmpHead = RxHead;

	while(tmpHead != RxTail && i < size) {
		buf[i++] = RxBuffer[RxTail++];
		RxTail %= RX_BUFSIZE;
	}

	return i;
}

DWORD __stdcall WaitCommThread(LPVOID lpVoid)
{
	DWORD           dwEvent;
	OVERLAPPED      os_overlapped;
	BOOL            bOk = TRUE;
	DWORD           dwRead, dwErrorFlags;  // 읽은 바이트수.

	DWORD rByteLen;
	BYTE check_sum;
	int i;

	int  nStep = 0;

	COMSTAT comstat;
	// Event, OS 설정.
	memset( &os_overlapped, 0, sizeof(OVERLAPPED));
   
	//--> 이벤트 설정..
	if( !(os_overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL)) )
	{
		bOk = FALSE;
	}

//--> 이벤트 마스크..
	//if( !SetCommMask( hCOMM, EV_RXCHAR) )
	if( !SetCommMask( hCommPort, EV_RXCHAR) ) {
		bOk = FALSE;
	}

//--> 이벤트나..마스크 설정에 실패함..
	if( !bOk )
	{
		MessageBox(NULL, TEXT("Error while creating ThreadWatchComm"), TEXT("SEE"), MB_OK);
		return FALSE;
	}

	while (hCommPort != INVALID_HANDLE_VALUE)//포트가 연결되면 무한 루프에 들어감
	{
		i = 0;
		check_sum = 0;
 		dwEvent = 0;
		
		// 포트에 읽을 거리가 올때까지 기다린다.
		WaitCommEvent( hCommPort, &dwEvent, NULL);

		//--> 데이터가 수신되었다는 메세지가 발생하면..
        if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
        {
			//System Queue에 도착한 byte수만 미리 읽는다.
			ClearCommError(hCommPort, &dwErrorFlags, &comstat);

			//시스템 큐에서 읽을 거리가 있으면...
			dwRead = comstat.cbInQue;
			
			char RByte[512];
			DWORD rByteLen = 0;
			if(!ReadFile(hCommPort, RByte, sizeof(RByte), &rByteLen, 0))
				break;
			if(dwRead != rByteLen)
				break;
			
			for(int i = 0; i < rByteLen; i++) {
				RxBuffer[RxHead] = RByte[i];
				RxHead = (RxHead+1) % RX_BUFSIZE;
			}
		}

		Sleep(0);	// 받은 데이터를 화면에 보여줄 시간을 벌기 위해.
					// 데이터를 연속으로 받으면 cpu점유율이 100%가 되어 화면에 뿌려주는 작업이 잘 안되고. 결과적으로 
					// 큐 버퍼에 데이터가 쌓이게 됨
   }
   
	CloseHandle( os_overlapped.hEvent);

MessageBox(NULL, TEXT("Comm thread end"), TEXT("Comm notification"), MB_OK);
	return TRUE;
}

char cmdbuf[128];

int SendPosCommand(int hpos, int vpos)
{
	int ret = 0;
	if(hCommPort == INVALID_HANDLE_VALUE)
		return -1;

	DWORD dwio = 0;
	if(vpos != -1) {
		ret += sprintf_s(cmdbuf, 128, "p 0 %d\r", vpos);
		int len = strlen(cmdbuf);
		sendComm(cmdbuf, len);
	}

	if(hpos != -1) {
		ret += sprintf_s(cmdbuf, 128, "p 1 %d\r", hpos);
		int len = strlen(cmdbuf);
		sendComm(cmdbuf, len);
	}

	return ret;
}

int  recvComm(char* buf, int len, int timeout)
{
	DWORD dwErrorFlags = 0;
	COMSTAT comstat;
	
	do {
		ClearCommError(hCommPort, &dwErrorFlags, &comstat);

		//시스템 큐에서 읽을 거리가 있으면...
		DWORD dwRead = comstat.cbInQue;
			
		if(dwRead > len) dwRead = len;
		DWORD dwio = 0;
		if(ReadFile(hCommPort, buf, dwRead, &dwio, NULL)) {
			if(dwio > 0) {
				return dwio;
			}
		}
		if(timeout > 0) {
			timeout -= 10;
			Sleep(10);
		}
	} while(timeout > 0);

	return 0;
}

void RecvRepPacket()
{
	if(RecvNext >= RecvCount) {
		RecvNext = 0;
		if(clientSocket != INVALID_SOCKET)
			RecvCount = recv(clientSocket, RecvBuf, sizeof(RecvBuf), 0);
		if(hCommPort != INVALID_HANDLE_VALUE)
			RecvCount = recvComm(RecvBuf, sizeof(RecvBuf), 10000);

		if(RecvCount <= 0) {
			// Error occurred
			RxPacketLen = 0;
			return;
		}
	}

	while(RecvNext < RecvCount) {
		char c = RecvBuf[RecvNext++];
		switch(RxPacketSta){
		case 0 :
			if(c == STX) {
				RxPacketLen = 0;
				RxPacketDat[RxPacketLen++] = c;
				RxPacketSta = 1;
			}
			break;
		case 1 :
			if(c == DLE) {
				RxPacketSta = 2;
			} else {
				if(RxPacketLen < sizeof(RxPacketDat)) {
					RxPacketDat[RxPacketLen++] = c;
					if(c == ETX) {
						RxPacketDat[RxPacketLen] = 0;
						RxPacketSta = 0;
						return;
					}
				}
			}
			break;
		case 2 :
			if(RxPacketLen < sizeof(RxPacketDat))
				RxPacketDat[RxPacketLen++] = c;
			RxPacketSta = 1;
		}
	}
}

int IsCommConnected()
{
	if(clientSocket != INVALID_SOCKET)
		return 1;

	if(hCommPort != INVALID_HANDLE_VALUE)
		return 1;

	return 0;
}

int ConnectTcpPort(char* ip, int port, TCHAR* msg)
{
	CloseCommPort();

	struct sockaddr_in  serv_addr;          

	memset(&serv_addr, 0, sizeof(serv_addr));          
	serv_addr.sin_family = PF_INET;          
	serv_addr.sin_addr.s_addr = inet_addr(ip);          
	serv_addr.sin_port = htons(port);          /* open a tcp socket*/         
	clientSocket = socket(PF_INET, SOCK_STREAM,0);
	if(clientSocket == INVALID_SOCKET) {
		lstrcpy(msg, TEXT("socket creation error\n"));
		return 0;
	}
	//	printf(" socket opened successfully. socket num is %d\n", x);
	/* connect to  the server */          
	if (connect(clientSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
		lstrcpy(msg, TEXT("can't connect to the server\n"));
		return 0;
	}

	return 1;
}

int ConnectCommPort(LPCTSTR port, int ispeed, TCHAR* msg)
{
	CloseCommPort();

	switch(ispeed) {
	case 115200 :
		ispeed = CBR_115200;
		break;
	case 19200 :
		ispeed = CBR_19200;
	}

//	hCommPort = OpenST150SerialPort();
	hCommPort = CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(hCommPort == INVALID_HANDLE_VALUE) {
		DWORD Err = GetLastError();
		wsprintf(msg, TEXT("TVBoy Comm Port failed : %08x"), Err);
		return 0;
	}
		
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength  = sizeof (dcb);
	if(GetCommState(hCommPort, &dcb)) {
		dcb.BaudRate  = ispeed;
		dcb.fBinary   = TRUE;
		dcb.fParity   = FALSE;
		dcb.fDtrControl  = FALSE;
		dcb.fRtsControl  = FALSE;
		dcb.fNull   = FALSE;
		dcb.StopBits  = ONESTOPBIT;
		dcb.Parity   = NOPARITY;
		dcb.ByteSize  = 8;
#if 1
		if (!SetCommState (hCommPort, &dcb)) {
			DWORD Err = GetLastError();
			wsprintf(msg, TEXT("SetCommState failed : %08x"), Err);
			CloseHandle(hCommPort);
			hCommPort = INVALID_HANDLE_VALUE;
			return 0;
		}
#endif
	} else {
/*
		DWORD Err = GetLastError();
		wsprintf(msg, L"GetCommState failed : %08x", Err);
		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
		return 0;
*/
	}

	DWORD threadid = 0;
//	CreateThread(NULL, 0, WaitCommThread, NULL, 0, &threadid);

	return 1;
}

void CloseCommPort()
{
	if(clientSocket != INVALID_SOCKET) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	if(hCommPort != INVALID_HANDLE_VALUE) {
		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
	}
}
