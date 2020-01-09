// Include the most common headers from the C standard library
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

// Include the main libnx system header, for Switch development
#include <switch.h>

// Create VSync event
Event vsync_event;

// Initialize frame counter variable
int frameCount = 0;

extern "C" {
// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.
#define INNER_HEAP_SIZE 0x80000
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_init_time(void);
void __libnx_initheap(void);
void __appInit(void);
void __appExit(void);
}

void __libnx_initheap(void) {
	void* addr  = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

// Init/exit services, update as needed.
void __attribute__((weak)) __appInit(void) {
	Result rc;

	// Initialize default services.
	rc = smInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

	rc = setsysInitialize();
	if(R_SUCCEEDED(rc)) {
		SetSysFirmwareVersion fw;
		rc = setsysGetFirmwareVersion(&fw);
		if(R_SUCCEEDED(rc))
			hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
		setsysExit();
	}

	// HID
	rc = hidInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

	rc = fsInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

	fsdevMountSdmc();

	rc = hiddbgInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	// vsync
	rc = viInitialize(ViServiceType_System);
	if(R_FAILED(rc))
		fatalThrow(rc);
}

void __attribute__((weak)) userAppExit(void);

void __attribute__((weak)) __appExit(void) {
	// Cleanup default services.
	fsdevUnmountAll();
	fsExit();
	hidExit();
	smExit();
}

// Main program entrypoint
int main(int argc, char* argv[]) {

	// Look here for an IPC example https://github.com/XorTroll/fsp-usb/blob/master/fsp-usb/source/fspusb_service.hpp
	return 0;
}
