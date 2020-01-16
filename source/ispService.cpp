#include "ispService.hpp"

IpsService::IpsService() {
	// Set up the overlay code
	// https://github.com/averne/dvdnx/blob/master/src/screen.cpp
	rc = smInitialize();
	if(R_FAILED(rc)) {
		fatalThrow(rc);
	}
	// ViServiceType_Manager defined here https://switchbrew.github.io/libnx/vi_8h.html
	rc = viInitialize(ViServiceType_Manager);
	if(R_FAILED(rc)) {
		fatalThrow(rc);
	}
	rc = viOpenDefaultDisplay(&display);
	if(R_FAILED(rc)) {
		viExit();
		return;
	}
	// flag 0 allows non-fullscreen layer
	rc = viCreateManagedLayer(&display, (ViLayerFlags)0, 0, &__nx_vi_layer_id);
	if(R_FAILED(rc)) {
		viCloseDisplay(&display);
		fatalThrow(rc);
	}
	rc = viCreateLayer(&display, &layer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	rc = viSetLayerScalingMode(&layer, ViScalingMode_FitToLayer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// Arbitrary z index
	rc = viSetLayerZ(&layer, 100);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// These might not be screenWidth and screenHeight TODO
	// They are smaller in the source
	rc = viSetLayerSize(&layer, screenWidth, screenHeight);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// The X and Y positions of the layer
	rc = viSetLayerPosition(&layer, 0.0f, 0.0f);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	rc = nwindowCreateFromLayer(&window, &layer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// PIXEL_FORMAT_RGBA_8888  defined in LibNX (based on Android)
	// Not PIXEL_FORMAT_RGBA_8888 in source, but I don't care
	// Upscaling and downscaling will happen so that the Layer Size and the FB size match
	rc = framebufferCreate(&framebuf, &window, screenWidth, screenHeight, PIXEL_FORMAT_RGBA_8888, 1);
	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}

	// Make Framebuffer linear to make things easier
	// Imma too dumb to figure out the raw format
	// 4 bytes per pixel (outstride)
	// rc = framebufferMakeLinear(&framebuf);
	// if(R_FAILED(rc)) {
	//	nwindowClose(&window);
	//	fatalThrow(rc);
	//}
	// Not actually making the framebuffer linear to save memory
}

ams::Result IspService::StartGetFramebuffer(s16 width, s16 height, s16 x, s16 y) {
	// Check this https://github.com/TheGreatRambler/nx-TAS/blob/dev/source/handle_savestates.cpp
}

ams::Result IspService::GetFramebufferChunk(s32 bytesToRead) { }

ams::Result IspService::EndGetFramebuffer() { }

ams::Result IspService::SetupOverlay(s8 layer) { }

ams::Result IspService::StartWriteOverlay(s16 width, s16 height, s16 x, s16 y) {
	// Check this https://github.com/TheGreatRambler/NX-TAS-UI/blob/dev/sysmodule/source/writeToScreen.hpp
}

ams::Result IspService::WriteOverlayChunk(s32 bytesToWrite) { }

ams::Result IspService::EndWriteOverlay() { }

~IpsService::IpsService() {
	// Close everything
	framebufferClose(&framebuf);
	nwindowClose(&window);
	viDestroyManagedLayer(&layer);
	viCloseDisplay(&display);
	viExit();
}
