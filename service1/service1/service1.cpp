#include <windows.h>
#include <stdio.h>
#include <Winsvc.h>




#define SLEEP_TIME 5000
#define LOGFILE "C:\\memstatus1.txt"
SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus; 
 
void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
int InitService();

int WriteToLog(char* str)
{
	FILE* log;
	log = fopen(LOGFILE, "a+");
	if (log == NULL)
		return -1;
	fprintf(log, "%s\n", str);
	fclose(log);
	return 0;
}



void ServiceMain(int argc, char** argv) 
{ 
    int error; 
 
    ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS;; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hStatus = RegisterServiceCtrlHandler(
		"MemoryStatus2", 
		(LPHANDLER_FUNCTION)ControlHandler); 
    if (hStatus == (SERVICE_STATUS_HANDLE)0) 
    { 
        // Registering Control Handler failed
        return; 
    }  
    // Initialize Service 
    error = InitService(); 
    if (error) 
    {
		// Initialization failed
        ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
        ServiceStatus.dwWin32ExitCode      = -1; 
        SetServiceStatus(hStatus, &ServiceStatus); 
        return; 
    } 
    // We report the running status to SCM. 
    ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
    SetServiceStatus (hStatus, &ServiceStatus);
 
    MEMORYSTATUS memory;
    // The worker loop of a service
    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		char buffer[16];
		GlobalMemoryStatus(&memory);
		sprintf(buffer, "%d", memory.dwAvailPhys);
		int result = WriteToLog(buffer);
		if (result)
		{
			ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
			ServiceStatus.dwWin32ExitCode      = -1; 
			SetServiceStatus(hStatus, &ServiceStatus);
			return;
		}

		Sleep(SLEEP_TIME);
	}
    return; 
}
 
// Service initialization
int InitService() 
{ 
    int result;
    result = WriteToLog("Monitoring started.");
    return(result); 
} 

// Control handler function
void ControlHandler(DWORD request) 
{ 
    switch(request) 
    { 
        case SERVICE_CONTROL_STOP: 
             WriteToLog("Monitoring stopped.");

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
 
        case SERVICE_CONTROL_SHUTDOWN: 
            WriteToLog("Monitoring stopped.");

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
        
        default:
            break;
    } 
 
    // Report current status
    SetServiceStatus (hStatus,  &ServiceStatus);
 
    return; 
} 



void InstallService()
{
	 SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CREATE_SERVICE );	

	if ( serviceControlManager )
	{
		TCHAR path[ _MAX_PATH + 1 ];
		if ( GetModuleFileName( 0, path, sizeof(path)/sizeof(path[0]) ) > 0 )
		{
			SC_HANDLE service = CreateService( serviceControlManager,
							"MemoryStatus4", "MemoryStatus4",
							SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
							SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
							0, 0, 0, 0, 0 );
			if ( service )
				CloseServiceHandle( service );
		}

		CloseServiceHandle( serviceControlManager );
	}
}


//void StartService(SC_HANDLE hSCM, char *szSvcName)
//{
//    SC_HANDLE hService = ::OpenService(hSCM, szSvcName, SERVICE_START);
//
//    if (hService == NULL)
//    {
//        printf("ERROR: COULDN'T ACCESS SERVICE\n");
//        return;
//    }
//
//    
//
//    ::CloseServiceHandle(hService);
//
//     
//}

void StartSvc()
{
	SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE serviceHandle = OpenService(serviceControlManager,"MemoryStatus4" , SERVICE_ALL_ACCESS );

	StartService(serviceHandle,0,NULL);
}

void main() 
{ 
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = "MemoryStatus4";
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    // Start the control dispatcher thread for our service
    StartServiceCtrlDispatcher(ServiceTable);  
	
	InstallService();
	StartSvc();
}
