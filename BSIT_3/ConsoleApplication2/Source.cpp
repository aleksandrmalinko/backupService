#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <winsvc.h>
#include <Tchar.h>
#include <string>
#include <fstream>


TCHAR* serviceName = L"My_service";
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE stopServiceEvent = NULL;
TCHAR servicePath[_MAX_PATH + 1];
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
FILE *conf;
std::string tmp_str;

int addLogMessage(const char* str)
{
	errno_t err;
	FILE* log;
	if ((err = fopen_s(&log, "log.txt", "a+")) != 0) {
		return -1;
	}
	fprintf(log, "%s\n", str);
	fclose(log);
	return 0;
}

int InstallService() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager)
	{
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(hSCManager,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePath,
		NULL, NULL, NULL, NULL, NULL);
	if (!hService)
	{
		int err = GetLastError();
		switch (err)
		{
		case ERROR_ACCESS_DENIED: addLogMessage("Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY: addLogMessage("Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME: addLogMessage("Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE: addLogMessage("Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME: addLogMessage("Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER: addLogMessage("Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT: addLogMessage("Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS: addLogMessage("Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage("Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage("Success install service!");
	printf("Success install service!");
	return 0;
}

int RemoveService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP | DELETE);
	if (!hService)
	{
		addLogMessage("Error: Can't remove service"); CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage("Success remove service!");
	printf("Success remove service!");
	return 0;
}

int StartService1()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (hSCManager)
	{
		SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_START);
		if (hService)
		{
			int f = StartService(hService, 0, NULL);
			if (!f)
			{
				CloseServiceHandle(hSCManager);
				addLogMessage("Error: Can't start service");
				printf("StartService failed (%d)\n", GetLastError());
				return -1;
			}
			addLogMessage("Success start service!");
			printf("Success start service!");
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return 0;
		}
		return -1;
	}
	else
	{
		printf("-1");
		return -1;
	}

}


void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP: serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;
	case SERVICE_CONTROL_SHUTDOWN: serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;
	default: break;
	}
	SetServiceStatus(serviceStatusHandle, &serviceStatus);
	return;
}
	
void StopService(void)
{
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
	if (serviceControlManager)
	{
		SC_HANDLE hService = OpenService(serviceControlManager, serviceName, SERVICE_STOP);
		if (hService)
		{
			SERVICE_STATUS serv_stat;
			if (!ControlService(hService, SERVICE_CONTROL_STOP, &serv_stat))
			{
				printf("\nERROR: Stop service\n");
				return;
			}
		}
	}
	CloseServiceHandle(serviceControlManager);
	addLogMessage("Success stop service!");
	printf("Success stop service!");
}

void InitService()
{

	std::string path_from;
	std::string arhive_name;
	std::string path_to;
	std::string mask;
	std::ifstream file("E:\\config.txt");
	getline(file, path_from);
	getline(file, arhive_name);
	getline(file, path_to);
	getline(file, mask);
	tmp_str += "C:\\WinRAR\\Rar.exe a -r -ep1 ";
	tmp_str += path_from;
	tmp_str += arhive_name;
	tmp_str += ' ';
	tmp_str += path_to;

	for (unsigned int i = 0; i < mask.size(); i++)
	{
		if (mask[i] != ' ') 
			tmp_str += mask[i];
		else
		{
			tmp_str += mask[i];
			tmp_str += path_from;
		}
	}
	file.close();
	std::cout << tmp_str << "\n";
}

void ServiceMain(int argc, char** argv)
{
		serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwServiceSpecificExitCode = 0;
		serviceStatus.dwCheckPoint = 0;
		serviceStatus.dwWaitHint = 0;
		serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, (LPHANDLER_FUNCTION)ControlHandler);
		if (serviceStatusHandle == (SERVICE_STATUS_HANDLE)0)
		{
			return;
		}
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
		{
			InitService();
			Sleep(3000);
			const char *c = tmp_str.c_str();
			system(c);
		}
	return;
}

void Restart()
{
	StopService(); 
	StartService1();
}
void Todo(_TCHAR* argv)
{
	if (wcscmp(argv, L"install") == 0) { printf("\nInstall command\n");  InstallService(); }
	else if (wcscmp(argv, L"start") == 0) { printf("\nStart command\n");  StartService1(); }
	else if (wcscmp(argv, L"remove") == 0) { printf("\nRemove command\n");  RemoveService(); }
	else if (wcscmp(argv, L"stop") == 0) { printf("\nStop command\n");  StopService(); }
	else if (wcscmp(argv, L"restart") == 0) { printf("\nRestart command\n"); Restart(); }
	else printf("\nError: wrong command");
}

int _tmain(int argc, _TCHAR* argv[])
{
	InitService();
	GetModuleFileName(0, servicePath, sizeof(servicePath) / sizeof(servicePath[0]));
	if (argc - 1 == 0)
	{
		SERVICE_TABLE_ENTRY ServiceTable[1]; ServiceTable[0].lpServiceName = serviceName;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		if (!StartServiceCtrlDispatcher(ServiceTable))
		{
			addLogMessage("Error: StartServiceCtrlDispatcher");
		}
	}
	else
		Todo(argv[argc - 1]);
}
