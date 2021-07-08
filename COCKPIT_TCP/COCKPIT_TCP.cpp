
#include<iostream>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#include<thread>

using namespace std;
void initialization();
void listenRecevie(SOCKET socketServer);
void processConnectMessage();
void processStatusMessage(char recevieBuffer[], int index);
void processResultMessage(char recevieBuffer[], int index);
void processChangeProgramMessage(char recevieBuffer[], int index);
void processGetProgramIDMessage(char recevieBuffer[], int index);

bool m_bStartRecevieSocket;
bool m_bOverHeat;
bool m_bSystemError;
bool m_bInputStatus;
bool m_bBusy;
bool m_bReport;
bool m_bSimulationMode;
bool m_bRunningMode;
bool m_bOverTriggered;
bool m_bWaitingForTrigger;


int main() {
	//定义长度变量
	int send_len = 0;

	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	int sendCommand;

	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.82.83");
	server_addr.sin_port = htons(36701);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "服务器连接成功！" << endl;
	}


	m_bStartRecevieSocket = true;
	thread listenThread(listenRecevie, s_server);
	listenThread.detach();


	//发送,接收数据
	while (1) {
		cout << "请输入发送信息  " << endl;
		cout << "1: 连接相机或获取状态" << endl;
		cout << "2: 触发检测" << endl;
		cout << "3: 切换运行模式" << endl;
		cout << "4: 切换停止模式" << endl;
		cout << "5: 切换程序01" << endl;
		cout << "6: 切换程序02" << endl;
		cout << "7: 获取程序号" << endl;
		cin >> sendCommand;
		if (sendCommand == 1)
		{
			send_buf[0] = 0x01;
			send_buf[1] = 0x00;
			send_buf[2] = 0x00;
			send_buf[3] = 0x00;
			send_len = send(s_server, send_buf, 100, 0);
		}
		else if (sendCommand == 2)
		{

			if (m_bRunningMode && m_bWaitingForTrigger) {
				send_buf[0] = 0x40;
				send_buf[1] = 0x00;
				send_buf[2] = 0x00;
				send_buf[3] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
			else
			{
				cout << "未在运行模式或者未就绪" << endl;

			}
		}
		else if (sendCommand == 3)
		{
			if (!m_bRunningMode & !m_bBusy) {
				send_buf[0] = 0x41;
				send_buf[1] = 0x00;
				send_buf[2] = 0x00;
				send_buf[3] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
			else
			{
				cout << "已经在运行模式或Busy" << endl;

			}
		}
		else if (sendCommand == 4)
		{
			if (m_bRunningMode & !m_bBusy) {
				send_buf[0] = 0x42;
				send_buf[1] = 0x00;
				send_buf[2] = 0x00;
				send_buf[3] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
			else
			{
				cout << "已经在停止模式或Busy" << endl;

			}
		}
		else if (sendCommand == 5)
		{
			if (!m_bRunningMode && !m_bBusy) {
				send_buf[0] = 0x31;
				send_buf[1] = 0x00;
				send_buf[2] = 0x02;
				send_buf[3] = 0x00;
				send_buf[4] = 0x01;
				send_buf[5] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
			else
			{
				cout << "在运行模式或Busy无法切换" << endl;

			}
		}
		else if (sendCommand == 6)
		{
			if (!m_bRunningMode && !m_bBusy) {
				send_buf[0] = 0x31;
				send_buf[1] = 0x00;
				send_buf[2] = 0x02;
				send_buf[3] = 0x00;
				send_buf[4] = 0x02;
				send_buf[5] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
			else
			{
				cout << "在运行模式或Busy无法切换" << endl;

			}
		}
		else if (sendCommand == 7 )
		{
			if (true) {
				send_buf[0] = 0x30;
				send_buf[1] = 0x00;
				send_buf[2] = 0x00;
				send_buf[3] = 0x00;
				send_len = send(s_server, send_buf, 100, 0);
			}
		}


		if (send_len < 0) {
			cout << "发送失败！" << endl;
			break;
		}

		Sleep(500);

	}
	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
	return 0;
}

void initialization()
{
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		//cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		//cout << "套接字库版本正确！" << endl;
	}
}

void listenRecevie(SOCKET socketServer)
{
	int recv_len = 0;
	int dataLength = 0;
	int index, messageType;

	//开启接收Socket循环
	while (m_bStartRecevieSocket)
	{
		//清空缓存
		char recv_buf[100];
		//接收数据，计算长度
		recv_len = recv(socketServer, recv_buf, 100, 0);
		index = 0;
		//获取TCP包中第一组数据
		while (recv_len > index)
		{
			//获取消息类型
			messageType = (int)recv_buf[index];
			//判断消息类型
			switch (messageType)
			{
			case 0x01: //连接消息
				processConnectMessage();
				break;

			case 0x12: //状态消息
				processStatusMessage(recv_buf, index);
				break;

			case 0x20://结果消息
				processResultMessage(recv_buf, index);
				break;

			case 0x31: //切换程序信息
				processChangeProgramMessage(recv_buf, index);
				break;
				
			case 0x30: //获取程序ID
				processGetProgramIDMessage(recv_buf, index);
				break;

			default:
				break;
			}
			//获取数据长度
			dataLength = (int)recv_buf[index + 2] + (int)recv_buf[index + 3] * 256;
			//处理完第一组数据，增大index
			index = index + dataLength + 4;
		}

	}
}

void processConnectMessage()
{
	std::cout << "已连接Cockpit" << std::endl;
}

void processStatusMessage(char recevieBuffer[], int index)
{
	int lowByteStatus, highByteStatus;

	//分割高低字节
	lowByteStatus = (int)(recevieBuffer[index + 4] & 0xFF);
	highByteStatus = (int)(recevieBuffer[index + 5] & 0xFF);

	//赋值状态位
	m_bOverHeat = lowByteStatus & 0x01;
	m_bSystemError = lowByteStatus & 0x02;
	m_bInputStatus = lowByteStatus & 0x04;
	m_bBusy = lowByteStatus & 0x10;
	m_bReport = lowByteStatus & 0x20;
	m_bSimulationMode = lowByteStatus & 0x40;
	m_bRunningMode = lowByteStatus & 0x80;
	m_bOverTriggered = highByteStatus & 0x01;
	m_bWaitingForTrigger = highByteStatus & 0x02;

	std::cout << "已处理状态消息! " << "Run Bit: " << m_bRunningMode << "  WaitForTrigger: " << m_bWaitingForTrigger << "  Busy: " << m_bBusy << std::endl;
}

void processResultMessage(char recevieBuffer[], int index)
{
	int dataLength;
	dataLength = (int)recevieBuffer[index + 2] + (int)recevieBuffer[index + 3] * 256;

	std::cout << "已处理结果消息: 长度为 " << dataLength;
	std::cout << " 程序ID为 " << (int)recevieBuffer[index + 4] + (int)recevieBuffer[index + 5] * 256 << " 数据为 ";
	cout.setf(ios_base::hex, ios_base::basefield);
	for (size_t i = 0; i < dataLength - 2; i++)
	{
		std::cout << " " << hex << (int)(recevieBuffer[index + 6 + i] & 0xFF);
	}
	cout.unsetf(ios::hex);
	std::cout << std::endl;
}

void processChangeProgramMessage(char recevieBuffer[], int index)
{
	int dataLength;
	int changeResult = (int)recevieBuffer[index + 4];

	std::cout << "已处理切换程序消息: ";
	if (changeResult == 0)
	{
		std::cout << "切换成功,当前程序为  " << (int)recevieBuffer[index + 6] + (int)recevieBuffer[index + 7] * 256 << std::endl;
	}
	else
	{
		std::cout << "切换失败,当前程序为  " << (int)recevieBuffer[index + 6] + (int)recevieBuffer[index + 7] * 256 << std::endl;
	}
}

void processGetProgramIDMessage(char recevieBuffer[], int index)
{
	std::cout << "已处理获取程序ID消息: 当前程序为 "<< (int)recevieBuffer[index + 4] + (int)recevieBuffer[index + 5] * 256 << std::endl;
}