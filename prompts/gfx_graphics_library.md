# Graphics Library

Prompt:

```txt
I am developing an embedded 2D game on an ESP32-S3. The application has two main components with vastly different timing requirements: a high-frequency game loop running at circa 50Hz (handling low-pass filtered IMU input, physics sub-stepping, collision resolution, and display rendering) and a low-frequency network stack running at circa 2Hz (handling blocking MQTT connections, TLS handshakes, and telemetry JSON serialization). 
I need to design a GraphicsManager class for the display. 

Because of the tight 20ms frame budget on Core 1, full screen updates will stall the game. I need a rendering strategy that supports fast, partial updates to erase and redraw only changing elements (the ball, scores, and cookie collection states), plus a custom brand loading screen.

Which graphics library should I use for this type of low-level, high-frequency rendering on the ESP32: The lightweight GFX library or something more complex like LVGL? Please give a recommendation based on performance, explain its basic rendering usage, and provide a short, problem-agnostic example showing how to initialize the library and draw basic shapes.
```

Answer with Gemini Flash 3.5 Thinking:

```txt
Recommendation: Arduino_GFX_Library vs. LVGL

For a 50Hz custom physics game loop running on a tight 20ms budget, Arduino_GFX_Library is highly recommended over LVGL (Light and Versatile Graphics Library). 

- LVGL is designed for object-oriented, widget-based User Interfaces (buttons, sliders, animations). It introduces a virtual DOM-like layout tree and substantial memory/CPU overhead, making it difficult to optimize for raw, frame-by-frame game updates.
- Arduino_GFX_Library provides low-level, direct hardware access to pixel manipulation and fast primitive rendering. It is lightweight, allows you to implement manual dirty-rectangle tracking easily, and minimizes driver-level overhead, which is exactly what your Game Core requires.

Basic Graphics Usage Explanation
Graphics libraries interact with the display via a communications bus (SPI, I2S, etc.) and a specific panel driver. Once initialized, rendering generally follows three core concepts:
1. Canvas/Frame Buffers: Drawing to an off-screen buffer in RAM prevents screen flickering.
2. Primitives: Utilizing built-in hardware-accelerated methods for common shapes (lines, circles, rectangles).
3. Partial Redrawing (Dirty Tracking): Instead of clearing the entire screen, you selectively overwrite the old bounding box of an object with background color, then draw the object at its new coordinates.

GFX Library Usage Example

Here is a streamlined, clean example of how to initialize Arduino_GFX and perform basic, frame-controlled primitive drawing without application-specific logic.

#include <Arduino_GFX_Library.h>

// 1. Initialize the display data bus and driver panel (pins from config)
Arduino_DataBus *bus = new Arduino_ESP32SPI(DC_PIN, CS_PIN, SCK_PIN, MOSI_PIN, MISO_PIN);
Arduino_GFX *gfx = new Arduino_ST7789(bus, RST_PIN, ROTATION);

void setup() {
    // Start the display hardware connection
    if (!gfx->begin()) {
        Serial.println("Display initialization failed!");
        return;
    }
    
    // Fill background color initially
    gfx->fillScreen(BLACK);
}

void loop() {
    // Example primitive rendering sequence representing a single tick update
    
    // Erase previous state area (Mock Dirty Rect Optimization)
    int prevX = 50, prevY = 50, radius = 10;
    gfx->fillCircle(prevX, prevY, radius, BLACK); 
    
    // Draw updated primitive state
    int newX = 55, newY = 50;
    gfx->fillCircle(newX, newY, radius, RED);
    
    // Draw localized UI elements 
    gfx->setTextColor(WHITE);
    gfx->setCursor(10, 10);
    gfx->print("FPS: 50");

    delay(20); // Maintain frame budget cadence
}
```
