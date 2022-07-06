#include <Wire.h>
#include "asm_sensors_w_mux_library.h"

SpectroDesktop spectro;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Serial.println("ASM spectral sensor Desktop Example 1: Polling Buttons");
    bool SensorType = spectro.begin();
    if (SensorType == true) {
        Serial.println("Sensor connected");
    }

}

void loop() {
    spectro.pollButtons();
}
