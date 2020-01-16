#include "windowHandler.hpp"
#include <memory>

WindowHandler::WindowHandler() {
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
}

void WindowHandler::reserveSpace(uint8_t layerIndex, uint16_t pixelType, uint16_t xOffset, uint16_t yOffset, uint16_t width, uint16_t height) {
	// Have to create everything
	std::unique_ptr<ViLayer> layer = std::make_unique<ViLayer>();

	// flag 0 allows non-fullscreen layer
	rc = viCreateManagedLayer(&display, (ViLayerFlags)0, 0, &__nx_vi_layer_id);
	if(R_FAILED(rc)) {
		viCloseDisplay(&display);
		fatalThrow(rc);
	}

	rc = viCreateLayer(&display, layer.get());
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}

	rc = viSetLayerScalingMode(layer.get(), ViScalingMode_FitToLayer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}
	// Arbitrary z index
	// layerIndex must be greater than 0, I think
	rc = viSetLayerZ(layer.get(), layerIndex);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}

	// These might not be screenWidth and screenHeight TODO
	// They are smaller in the source
	rc = viSetLayerSize(layer.get(), width, height);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}

	// The X and Y positions of the layer
	rc = viSetLayerPosition(layer.get(), (float)xOffset, (float)yOffset);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}

	std::unique_ptr<NWindow> window = std::make_unique<NWindow>();

	rc = nwindowCreateFromLayer(window.get(), layer.get());
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(layer.get());
		fatalThrow(rc);
	}

	std::unique_ptr<Framebuffer> framebuf = std::make_unique<Framebuffer>();

	// PIXEL_FORMAT_RGBA_8888  defined in LibNX (based on Android)
	// Not PIXEL_FORMAT_RGBA_8888 in source, but I don't care
	// Upscaling and downscaling will happen so that the Layer Size and the FB size match
	// Passed var is like PIXEL_FORMAT_RGBA_8888
	rc = framebufferCreate(framebuf.get(), window.get(), width, height, pixelType, 1);
	if(R_FAILED(rc)) {
		nwindowClose(window.get());
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

	// Add to all the vectors
	layers.push_back(layer);
	windows.push_back(window);
	framebufs.push_back(framebuf);
}

uint32_t WindowHandler::getIntersectArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width1, uint16_t height1, uint16_t width2, uint16_t height2) {
	// Simple algorithm
	// This is used to save memory
	uint16_t x_overlap = std::max(0, std::min(x1 + width1, x2 + width2) - std::max(x1, x2));
	uint16_t y_overlap = std::max(0, std::min(y1 + height1, y2 + height2) - std::max(y1, y2));
	return x_overlap * y_overlap;
}