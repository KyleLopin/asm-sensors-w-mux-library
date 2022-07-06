/*
  This is a library written to allow the AMS AS7265x,
  the AS7262 and AS7263 spectral boards to be used singly
  or in combination with a Qwiic mux board
  SparkFun sells these parts at its website: www.sparkfun.com
  https://www.sparkfun.com/products/15050  - AS7265X breakout board
  https://www.sparkfun.com/products/14347 - AS7262 breakout board
  https://www.sparkfun.com/products/14351 - AS7263 breakout board
  https://www.sparkfun.com/products/16784 - Qwiic Mux breakout board

  Written by Kyle Vitautas Lopin @ Naresuan University, June, 2022

  Development environment specifics:
  Arduino IDE 1.8.11

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "asm_sensors_w_mux_library.h"
#include "Arduino.h"

#define DEBUG_FLAG (0)

//Constructor
SpectroDesktop::SpectroDesktop()
{
}

// Initialize the device by:
// 1) checking for the qwiic mux
// 2) look for any and all as7262/as7263/as7265x sensor over qwiic
// Returns false if no sensor
bool SpectroDesktop::begin(TwoWire &wirePort) {
    /* Start the I2C and button, then check for a mux.  If there is a mux, each port
    of the mux will be enabled (and the other ports disabled) and checked for the AS726x / AS7265x 
    and the Qwicc button I2C address
    
    Returns true if a color sensor I2C address is found*/
    _i2cPort = &wirePort;

    button.begin();  // use this to represent every button
    #if(DEBUG_FLAG)
        Serial.println("Checking for a mux");
    #endif
    bool found_device = false;  // initialize to false, then set to true if a sensor is found
    use_mux = checkI2cAddress(MUX_ADDR);  // set boolean if the device has a i2c mux or not
    
    if (use_mux) {
        Serial.println("Have mux");
        for (byte i = 0; i <= 7; i++) {  // go thru each port on the i2c mux
            enableMuxPort(i);
            int avail = checkI2cAddress(AS726X_ADDR);  // check if sensor i2c address is on the port
            Serial.print("Port: "); Serial.print(i);
            Serial.print(" available: "); Serial.println(avail);
            if (avail) {  // get what type of sensor is on the port AS7262 / AS7263 / or AS7265X
                sensor_type_array[i] = getSensorType(i);
                found_device = true;
            }
        }
    }
    else {  // if no mux just check if a sensor is attached to the board's Qwiic connection
        Serial.println("No mux");
        int avail = checkI2cAddress(AS726X_ADDR);
        if (avail) {
            sensor_type_array[0] = getSensorType(0);  // just put in a place holder for the channel
            found_device = true;
        }
    }
    #if(DEBUG_FLAG)
        Serial.println("End Setup");
    #endif
    return (found_device);
}

void SpectroDesktop::pollButtons() {
    // go thru each port on the i2c mux, if no mux, sensor_type will be NO_SENSOR for 1-7 anyways
    // Serial.println("Polling Buttons");
    for (byte i = 0; i <= 7; i++) {  
        //Serial.print("i: "); Serial.println(i);
        //Serial.print("Sensor type: "); Serial.println(sensor_type_array[i]);
        enableMuxPort(i);
        if (sensor_type_array[i] != NO_SENSOR) {  // check if a sensor was initialized there
            byte settings = getMuxSettings();
            //Serial.print("Mux settings: "); Serial.println(settings);
            //Serial.print("sensor on port "); Serial.println(button.isConnected());
            if (button.isConnected() == true) {
                // Serial.println("Has button");
                if (button.hasBeenClicked()) {
                    button.LEDon(50);
                    button.clearEventBits();
                    #if(DEBUG_FLAG==2)
                        Serial.print("Button clicked on port: "); Serial.println(i);
                    #endif
                    if (sensor_type_array[i] == AS7262_SENSOR) {
                        Serial.println("running AS7262 Sensor");
                        readAS7262(i);
                    }
                    else if (sensor_type_array[i] == AS7263_SENSOR) {
                        Serial.println("running AS7263 Sensor");
                        readAS7263(i);
                    }
                    else if (sensor_type_array[i] == AS7265X_SENSOR) {
                        Serial.println("running AS7265x Sensor");
                        readAS7265x(i);
                    }
                    button.LEDoff();
                }
            }
        }
    }
}

void SpectroDesktop::readAS7262(byte portNumber) {
    /* Read an AS7262 (the mux must be connected correctly before calling this)
    no return, the data will be print to the serial port.  Different than AS7263 because
    the AS726x public methods will now allow it */
    bool useBulb = enableBulbsArray[portNumber] & 0x01;
    if (useBulb) {
        as726x.enableBulb();
    }
    as726x.takeMeasurements();
    if (useBulb) {
        as726x.disableBulb();
    }
    getAS7262Data();
}

void SpectroDesktop::getAS7262Data() {
    /* Get the AS7262 data (public names of as726x library is different between as7262 and as7263
    prints the data to the serial port*/
    Serial.print("AS7262 Data: ");
    Serial.print(as726x.getCalibratedViolet(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedBlue(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedGreen(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedYellow(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedOrange(), 4); Serial.print(", ");
    Serial.println(as726x.getCalibratedRed(), 4);
}

void SpectroDesktop::readAS7263(byte portNumber) {
    /* Read an AS7263 (the mux must be connected correctly before calling this)
    no return, the data will be print to the serial port.  Different than AS7262 because
    the AS726x public methods will now allow it */
    bool useBulb = enableBulbsArray[portNumber] & 0x01;
    if (useBulb) {
        as726x.enableBulb();
    }
    as726x.takeMeasurements();  // AS7262 and AS7263 have same method here
    getAS7263Data();
    if (useBulb) {
        as726x.disableBulb();
    }
}

void SpectroDesktop::getAS7263Data() {
    /* Get the AS7262 data (public names of as726x library is different between as7262 and as7263
    prints the data to the serial port*/
    Serial.print("AS7263 Data: ");
    Serial.print(as726x.getCalibratedR(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedS(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedT(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedU(), 4); Serial.print(", ");
    Serial.print(as726x.getCalibratedV(), 4); Serial.print(", ");
    Serial.println(as726x.getCalibratedW(), 4);
}

void SpectroDesktop::readAS7265x(byte portNumber) {
    /* Read an AS7265x (the mux must be connected correctly before calling this)
    no return, the data will be print to the serial port.*/
    if (enableBulbsArray[portNumber] & 0x01) {
        as7265x.enableBulb(AS7265x_LED_WHITE);
    }
    if (enableBulbsArray[portNumber] & 0x02) {
        as7265x.enableBulb(AS7265x_LED_UV);
    }
    if (enableBulbsArray[portNumber] & 0x04) {
        as7265x.enableBulb(AS7265x_LED_IR);
    }
    as7265x.takeMeasurements();  
    if (enableBulbsArray[portNumber] & 0x01) {
        as7265x.disableBulb(AS7265x_LED_WHITE);
    }
    if (enableBulbsArray[portNumber] & 0x02) {
        as7265x.disableBulb(AS7265x_LED_UV);
    }
    if (enableBulbsArray[portNumber] & 0x04) {
        as7265x.disableBulb(AS7265x_LED_IR);
    }
    getAS7265xData();
}

void SpectroDesktop::getAS7265xData() {
    Serial.print("AS7265x Data: ");

    Serial.print(as7265x.getCalibratedA()); Serial.print(", ");  // 410 nm 
    Serial.print(as7265x.getCalibratedB()); Serial.print(", ");  // 435 nm
    Serial.print(as7265x.getCalibratedC()); Serial.print(", ");  // 460 nm
    Serial.print(as7265x.getCalibratedD()); Serial.print(", ");  // 485 nm
    Serial.print(as7265x.getCalibratedE()); Serial.print(", ");  // 510 nm
    Serial.print(as7265x.getCalibratedF()); Serial.print(", ");  // 535 nm

    Serial.print(as7265x.getCalibratedG()); Serial.print(", ");  // 565 nm
    Serial.print(as7265x.getCalibratedH()); Serial.print(", ");  // 585 nm
    Serial.print(as7265x.getCalibratedR()); Serial.print(", ");  // 610 nm
    Serial.print(as7265x.getCalibratedI()); Serial.print(", ");  // 645 nm
    Serial.print(as7265x.getCalibratedS()); Serial.print(", ");  // 680 nm
    Serial.print(as7265x.getCalibratedJ()); Serial.print(", ");  // 705 nm

    Serial.print(as7265x.getCalibratedT()); Serial.print(", ");  // 730 nm
    Serial.print(as7265x.getCalibratedU()); Serial.print(", ");  // 760 nm
    Serial.print(as7265x.getCalibratedV()); Serial.print(", ");  // 810 nm
    Serial.print(as7265x.getCalibratedW()); Serial.print(", ");  // 860 nm
    Serial.print(as7265x.getCalibratedK()); Serial.print(", ");  // 900 nm
    Serial.println(as7265x.getCalibratedL());  // 940 nm
}

SensorType SpectroDesktop::getSensorType(byte channel) {
    /* Begin the AS726X data class and use that class to get the hardware type of the sensor
    The AS726X class is not fully compatible with the AS7265x, but getting the hardware type is the same
    
    Return sensor type*/
    #if(DEBUG_FLAG)
        Serial.println("Get sensor type");
    #endif
    SensorType _sensor_type = NO_SENSOR;  // Initialize to no sensor and fill in if one is found
    //bool sensor_begins = as726x.begin(*_i2cPort);
    bool sensor_begins = as726x.begin();  // will be 0 for no sensor AND for AS7265x so have to check this later
    #if(DEBUG_FLAG)
        Serial.print("sensor begins: "); Serial.println(sensor_begins);
    #endif
    uint8_t hw_type = as726x.getVersion();
    #if(DEBUG_FLAG)
        Serial.print("Hardware type: 0x"); Serial.println(hw_type, HEX);
    #endif
    if (hw_type == AS7262_CODE) {
        _sensor_type = AS7262_SENSOR;
        as726x.setIntegrationTime(150);
        as726x.setMeasurementMode(0b11);  // read all channels
        Serial.print("AS7262 device attached to port: ");
        Serial.print(channel); Serial.print("|  ");
    }
    else if (hw_type == AS7263_CODE) {
        _sensor_type = AS7263_SENSOR;
        as726x.setIntegrationTime(150);
        as726x.setMeasurementMode(0b11);  // read all channels
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
    }  // else SensorType is already set to no sensor
    //  Now check if a button is also attached
    Serial.println(button.isConnected());
    if (button.isConnected() == false) {
        Serial.println("No button attached to device.");
    }
    else {
        Serial.println("Button attached to device.");
        button.clearEventBits();  // Clear any clicks before being setup
        button.LEDoff();
        button.setDebounceTime(20);  // Sometime this can get messed up for some reason
        #if(DEBUG_FLAG)
            Serial.print("Button debounce time: "); Serial.println(button.getDebounceTime());
        #endif
    }
    return _sensor_type;
}

bool SpectroDesktop::enableMuxPort(byte portNumber) {
    /* Enable one of the mux ports, and 1 port only. The mux is then
    checked and will return true if the mux is set correctly or false if not*/
    #if(DEBUG_FLAG)
        Serial.print("enabling port1: "); Serial.println(portNumber);
    #endif
    if (portNumber > 8) {  // Check for a correct port number
        Serial.println("enableMuxPort: port Number has to be 7 or less");
        return false;
    }
    byte settings = (1 << portNumber);
    _i2cPort->beginTransmission(MUX_ADDR);
    _i2cPort->write(settings);
    byte end_transmission = _i2cPort->endTransmission();
    //if (end_transmission != 0) {
    #if(DEBUG_FLAG)
        Serial.print("End transmission (enable): "); Serial.println(end_transmission);
    #endif
    //    //return false;  // Device is not responding correctly
    //}
    byte current_settings = getMuxSettings();
    if (current_settings == settings) {
        #if(DEBUG_FLAG)
            Serial.println("Mux set correctly");
        #endif
        return true;
    }
    return false;  // if _settings != settings
}

byte SpectroDesktop::getMuxSettings() {
    /* Check what the setting are in the mux */
    _i2cPort->requestFrom(MUX_ADDR, 1);
    if (!_i2cPort->available()) {  // Make sure the mux will respond
        Serial.println("Mux not sending settings");
        return 254;
    }
    // get current settings
    byte settings = _i2cPort->read();
    #if(DEBUG_FLAG)
        Serial.print("mux settings (get): "); Serial.println(settings);
    #endif
    return settings;
}

bool SpectroDesktop::sendMuxSettings(byte _settings) {
    /* Write a new mux setting to set the ports that are open */
    _i2cPort->beginTransmission(MUX_ADDR);
    _i2cPort->write(_settings);
    byte end_trans = _i2cPort->endTransmission();
    #if(DEBUG_FLAG)
        Serial.print("Send mux settings end trans: "); Serial.println(end_trans);
    #endif
    if (end_trans != 0) {
        return false;  // Device is not responding correctly
    }
    return true;
}

bool SpectroDesktop::checkI2cAddress(byte _addr) {
    /* Check if an i2c address is on the i2c bus.  If an error is caused by this
    the i2c (Wire) will be reset
    Return true if the address is on the i2c, else false */
    _i2cPort->beginTransmission(_addr);
    byte end_trans = _i2cPort->endTransmission();
    #if(DEBUG_FLAG)
        Serial.print("End transmion code (check i2c): "); Serial.println(end_trans);
    #endif
    if (end_trans == 0) {
        return true;
    }
    else if (end_trans == 4) {
        #if(DEBUG_FLAG)
            Serial.println("Restarting i2c from end transmission error of 4");
        #endif
        _i2cPort->end();
        _i2cPort->begin();
    }
    return false;
}
