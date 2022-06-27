#include "asm_sensors_w_mux_library.h"
#include "Arduino.h"

//Constructor
SpectroDesktop::SpectroDesktop()
{
}

// Initialize the device by:
// 1) checking for the qwiic mux
// 2) look for any and all as7262/as7263/as7265x sensor over qwiic
// Returns false if no sensor
bool SpectroDesktop::begin(TwoWire &wirePort) {
    button.begin();  // use this to represent every button

    use_mux = myMux.begin(MUX_ADDR);  // set boolean if the device has a i2c mux or not
    // begin turns all ports off
    if (use_mux) {
        Serial.println("Has a mux");
    }
    Serial.print("Use mux ");
    Serial.println(myMux.isConnected());
    //    for (int i=0; i <= 7; i++) {
    //      myMux.enablePort(i);
    //      if (button.isConnected()) {
    //        Serial.print("Button on port: ");
    //        Serial.println(i);
    //        button.clearEventBits();  // if the button has been pressed before, clear it now
    //      }
    get_sensor_info();
    return (true);
}

void SpectroDesktop::get_sensor_info() {
    if (use_mux) {
        Serial.println("Have mux");
        for (int i = 0; i <= 7; i++) {
            myMux.enablePort(i);
            int avail = check_channel();
            if (avail) {
                sensor_type[i] = get_sensor_type(i);
            }
            myMux.disablePort(i);
        }
    }
    else {
        Serial.println("No mux");
        int avail = check_channel();
        if (avail) {
            sensor_type[0] = get_sensor_type(0);  // just put in a place holder for the channel
        }
    }
    Serial.println("End Setup");
}

boolean SpectroDesktop::check_channel() {
    boolean available_channel = false;
    Wire.requestFrom(AS726X_ADDR, 1);
    available_channel = Wire.available();
    return available_channel;
}

byte SpectroDesktop::get_sensor_type(int channel) {
    byte _sensor_type = NO_SENSOR;
    if (as726x.begin(Wire) == true) {
        as726x.setIntegrationTime(150);
        as726x.setBulbCurrent(0b11);
    }
    uint8_t hw_type = as726x.getVersion();
    Serial.print("Hardware type: 0x");
    Serial.println(hw_type, HEX);

    if (hw_type == AS7262_CODE) {
        _sensor_type = AS7262_SENSOR;
        Serial.print("AS7262 device attached to port: ");
        Serial.print(channel); Serial.print("|  ");
    }
    else if (hw_type == AS7263_CODE) {
        _sensor_type = AS7263_SENSOR;
        Serial.print("AS7263 device attached to port: ");
        Serial.print(channel); Serial.print("|  ");
    }
    else if (hw_type == AS7265X_CODE) {
        _sensor_type = AS7265X_SENSOR;
        as7265x.begin();
        as7265x.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE);
        as7265x.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);
        as7265x.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);
        as7265x.disableIndicator();
        as7265x.setIntegrationCycles(150);
        Serial.print("AS7265x device attached to port: ");
        Serial.print(channel); Serial.print("|  ");
    }
    if (button.isConnected() == false) {
        Serial.println("No button attached to device.");
    }
    else {
        Serial.println("Button attached to device.");
        button.clearEventBits();  // Clear any clicks before being setup
        button.LEDoff();
        button.setDebounceTime(20);  // Sometime this can get messed up for some reason
    //    println(button.getDebounceTime());
    }
    return _sensor_type;
}
