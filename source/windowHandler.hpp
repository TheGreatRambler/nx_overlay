#pragma once

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <memory>
#include <switch.h>

// I dunno what this is
extern "C" u64 __nx_vi_layer_id;

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

// Guessing how many bytes the overhead is for window creation
#define WINDOW_CREATION_OVERHEAD 300

// Struct for every window section
struct RectInfo {
	// Wether this actually has a framebuffer mapped to it or not
	uint8_t isFramebufferMapped = true;
	std::unique_ptr<ViLayer> layer;
	std::unique_ptr<NWindow> window;
	std::unique_ptr<Framebuffer> framebuf;
	uint16_t pixelType;
	uint8_t layerIndex;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	// For the weird ctz stuff
	uint8_t ctz;
};

class WindowHandler {
private:
	// Error handling for everything
	Result rc;

	ViDisplay display;

	// Vector of layers and such that increases on demand
	std::vector<std::unique_ptr<RectInfo>> rects;

	// Whether alpha is used at all, significant
	// optimizations can be made if this is false
	uint6_t useAlpha;

public:
	WindowHandler();

	void setUseAlpha(uint8_t use);

	void createRect(uint8_t layerIndex, uint16_t pixelType, uint16_t xOffset, uint16_t yOffset, uint16_t width, uint16_t height);

	void reserveSpace(std::unique_ptr<RectInfo> rect);

	void startDraw(std::unique_ptr<RectInfo> rect);

	void endDraw(std::unique_ptr<RectInfo> rect);

	void closeWindow(std::unique_ptr<RectInfo> rect);

	uint32_t getIntersectArea(std::unique_ptr<RectInfo> rect1, std::unique_ptr<RectInfo> rect2);

	uint32_t getPixelOffset(uint32_t x, uint32_t y, uint8_t ctz);

	~WindowHandler();
}