#include <Wire.h>
#include "asm_sensors_w_mux_library.h"

SpectroDesktop spectro;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    //Wire.setTimeout(3000);
    Serial.println("ASM spectral sensor Desktop Example 1: Polling Buttons"); 
    delay(500);
    bool sensor = spectro.begin();
    if (sensor == true) {
        Serial.println("Sensor connected");
    }

}

void loop() {
    //Serial.println("loop");
    delay(500);

}
