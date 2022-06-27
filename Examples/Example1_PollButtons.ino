#include "asm_sensors_w_mux_library.h"

SpectroDesktop spectro;

void setup() {
  Serial.begin(115200);
  Serial.println("ASM spectral sensor Desktop Example 1: Polling Buttons");
  if (spectro.begin() == true) {
    Serial.println("Sensor connected");

  }

}

void loop() {
  // put your main code here, to run repeatedly:
  
}
