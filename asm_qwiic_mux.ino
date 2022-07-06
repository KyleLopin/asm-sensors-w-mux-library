#include <Wire.h>
#include "asm_sensors_w_mux_library.h"

SpectroDesktop spectro;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    //Wire.setTimeout(3000);
    Serial.println("ASM spectral sensor Desktop Example 1: Polling Buttons"); 
    delay(500);
    bool SensorType = spectro.begin();
    if (SensorType == true) {
        Serial.println("Sensor connected 2");
    }

}

void loop() {
    Serial.println("Loop");
    spectro.pollButtons();
}
