#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <switch.h>

// I dunno what this is
extern "C" u64 __nx_vi_layer_id;

class WindowHandler {
private:
	// Error handling for everything
	Result rc;

	ViDisplay display;
	// Vector of layers and such that increases on demand
	std::vector<std::unique_ptr<ViLayer>> layers;
	std::vector<std::unique_ptr<NWindow>> windows;
	std::vector<std::unique_ptr<Framebuffer>> framebufs;

public:
	WindowHandler();

	// Basically adds a window to the display for writing
	void reserveSpace(uint8_t layerIndex, uint16_t pixelType, uint16_t xOffset, uint16_t yOffset, uint16_t width, uint16_t height);

	uint32_t getIntersectArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width1, uint16_t height1, uint16_t width2, uint16_t height2);
}