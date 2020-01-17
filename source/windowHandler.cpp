#include "windowHandler.hpp"
#include <bits/stdint-uintn.h>
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

	// Use alpha is default true
	useAlpha = true;
}

void WindowHandler::setUseAlpha(uint8_t use) {
	useAlpha = use;
}

void WindowHandler::reserveSpace(std::unique_ptr<RectInfo> rect) {
	// Have to create everything
	// std::unique_ptr<RectInfo> rect = std::make_unique<RectInfo>();

	rect->layer    = std::make_unique<ViLayer>();
	rect->window   = std::make_unique<NWindow>();
	rect->framebuf = std::make_unique<Framebuffer>();

	// flag 0 allows non-fullscreen layer
	rc = viCreateManagedLayer(&display, (ViLayerFlags)0, 0, &__nx_vi_layer_id);
	if(R_FAILED(rc)) {
		viCloseDisplay(&display);
		fatalThrow(rc);
	}

	rc = viCreateLayer(&display, rect->layer.get());
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}

	rc = viSetLayerScalingMode(rect->layer.get(), ViScalingMode_FitToLayer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}
	// Arbitrary z index
	// layerIndex must be greater than 0, I think
	rc = viSetLayerZ(rect->layer.get(), rect->layerIndex);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}

	// These might not be screenWidth and screenHeight TODO
	// They are smaller in the source
	rc = viSetLayerSize(rect->layer.get(), rect->width, rect->height);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}

	// The X and Y positions of the layer
	rc = viSetLayerPosition(rect->layer.get(), (float)rect->x, (float)rect->y);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}

	rc = nwindowCreateFromLayer(rect->window.get(), rect->layer.get());
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(rect->layer.get());
		fatalThrow(rc);
	}

	// PIXEL_FORMAT_RGBA_8888  defined in LibNX (based on Android)
	// Not PIXEL_FORMAT_RGBA_8888 in source, but I don't care
	// Upscaling and downscaling will happen so that the Layer Size and the FB size match
	// Passed var is like PIXEL_FORMAT_RGBA_8888
	/* Supported pixel formats:
		PIXEL_FORMAT_RGBA_8888
		PIXEL_FORMAT_RGBX_8888
		PIXEL_FORMAT_RGB_565
		PIXEL_FORMAT_BGRA_8888
		PIXEL_FORMAT_RGBA_4444
		Greyscale might be supported via software to save precious memory
	*/
	rc = framebufferCreate(rect->framebuf.get(), rect->window.get(), rect->width, rect->height, rect->pixelType, 1);
	if(R_FAILED(rc)) {
		nwindowClose(rect->window.get());
		fatalThrow(rc);
	}

	// Need to generate ctz at runtime
	// Runtime equivlent of this https://www.go4expert.com/articles/builtin-gcc-functions-builtinclz-t29238/
	// https://www.geeksforgeeks.org/count-number-of-trailing-zeros-in-binary-representation-of-a-number-using-bitset/
	std::bitset<sizeof(uint16_t)> bits;
	bits |= rect->width;
	int trailingZeros = 0;
	for(int i = 0; i < sizeof(uint16_t); i++) {
		if(bits[i] == 0) {
			trailingZeros++;
		} else {
			break;
		}
	}
	// Set ctz
	rect->ctz = trailingZeros;

	// Add the data
	rects.push_back(rect);
}

void WindowHandler::startDraw(std::unique_ptr<RectInfo> rect) {
	framebufferBegin(rect->framebuf.get(), NULL);
}

void WindowHandler::endDraw(std::unique_ptr<RectInfo> rect) {
	framebufferEnd(rect->framebuf.get());
}

void WindowHandler::closeWindow(std::unique_ptr<RectInfo> rect) {
	framebufferClose(rect->framebuf.get());
	nwindowClose(rect->window.get());
	viDestroyManagedLayer(rect->layer.get());
}

uint32_t WindowHandler::getIntersectArea(std::unique_ptr<RectInfo> rect1, std::unique_ptr<RectInfo> rect2) {
	// Simple algorithm
	// This is used to save memory
	uint16_t x_overlap = std::max(0, std::min(rect1->x + rect1->width, rect2->x + rect2->width) - std::max(rect1->x, rect2->x));
	uint16_t y_overlap = std::max(0, std::min(rect1->y + rect1->height, rect2->y + rect2->height) - std::max(rect1->y, rect2->y));
	return x_overlap * y_overlap;
}

// https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
uint32_t WindowHandler::getPixelOffset(uint32_t x, uint32_t y, uint8_t ctz) {
	// This should, in theory, work for any pixel type
	// Swizzling pattern:
	//    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
	// -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
	// Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
	constexpr uint32_t x_mask = (ctz - 1) << 5;
	const uint32_t swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
	const uint32_t swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
	return swizzled_x + swizzled_y;
}

WindowHandler::~WindowHandler() {
	viCloseDisplay(&display);
	viExit();
}