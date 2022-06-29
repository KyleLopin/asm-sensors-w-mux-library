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
    _i2cPort = &wirePort;

    button.begin();  // use this to represent every button

    use_mux = check_i2c_addr(MUX_ADDR);  // set boolean if the device has a i2c mux or not
    // begin turns all ports off
    if (use_mux) {
        Serial.println("Has a mux");
    }
    get_sensor_info();
    return (true);
}

void SpectroDesktop::get_sensor_info() {
    if (use_mux) {
        Serial.println("Have mux");
        for (byte i = 0; i <= 7; i++) {
            enableMuxPort(i);
            int avail = check_channel();
            Serial.print("Port: "); Serial.print(i);
            Serial.print(" available: "); Serial.println(avail);
            if (avail) {
                Serial.println("Get sensor type");
                sensor_type[i] = get_sensor_type(i);
            }
            disableMuxPort(i);
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
    bool sensor_begins = as726x.begin(Wire);
    Serial.print("sensor begins: "); Serial.println(sensor_begins);
    if (sensor_begins == true) {
        Serial.println("Check 0a");
        as726x.setIntegrationTime(150);
        as726x.setBulbCurrent(0b11);
    }
    Serial.println("Check1");
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

bool SpectroDesktop::enableMuxPort(byte portNumber) {
    if (portNumber > 8) {  // Check for a correct port number
        Serial.println("enableMuxPort: port Number has to be 7 or less");
        return false;
    }
    // Make sure the mux is not trying to send information before changing
    _i2cPort->requestFrom(MUX_ADDR, 1);
    if (!_i2cPort->available()) {
        return false;
    }
    // get current settings
    byte settings = _i2cPort->read();
    // Set the bit in the settings to enable the port
    settings |= (1 << portNumber);
    // Write new settings to mux
    _i2cPort->beginTransmission(MUX_ADDR);
    _i2cPort->write(settings);
    if (_i2cPort->endTransmission() != 0) {
        return false;  // Device is not responding correctly
    }
    return true;
}

bool SpectroDesktop::disableMuxPort(byte portNumber) {
    if (portNumber > 8) {  // Check for a correct port number
        Serial.println("disableMuxPort: port Number has to be 7 or less");
        return false;
    }
    // Make sure the mux is not trying to send information before changing
    _i2cPort->requestFrom(MUX_ADDR, 1);
    if (!_i2cPort->available()) {
        return false;
    }
    // get current settings
    byte settings = _i2cPort->read();
    // Clear the bit in the settings to disable the port
    settings &= ~(1 << portNumber);
    // Write new settings to mux
    _i2cPort->beginTransmission(MUX_ADDR);
    _i2cPort->write(settings);
    if (_i2cPort->endTransmission() != 0) {
        return false;  // Device is not responding correctly
    }
    return true;
}

bool SpectroDesktop::check_i2c_addr(byte _addr) {
    _i2cPort->beginTransmission(_addr);
    byte end_trans = _i2cPort->endTransmission();
    Serial.print("End transmion code: "); Serial.println(end_trans);
    if (end_trans == 0) {
        return true;
    }
    return false;
}
