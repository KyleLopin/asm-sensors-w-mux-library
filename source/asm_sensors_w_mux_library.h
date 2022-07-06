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

#ifndef _ASM_SENSORS_W_MUX_LIBRARY_H
#define _ASM_SENSORS_W_MUX_LIBRARY_H

// Dependent Libraries
#include <AS726X.h>  // http://librarymanager/All#Sparkfun_AS726X
#include <SparkFun_AS7265X.h>  // http://librarymanager/All#Sparkfun_AS7265X
#include <SparkFun_Qwiic_Button.h>  // http://librarymanager/All#Sparkfun_Qwiic_Button_Switch
#include <Wire.h>

// Define statements
// I2C info
#define MUX_ADDR	0x70
#define AS726X_ADDR 0x49
#define BUTTON_ADDR 0x6F
// SENSOR TYPE
#define AS7262_CODE 0x3E
#define AS7263_CODE 0x3F
#define AS7265X_CODE 0x41
// Sensor register infor
#define FIRST_CAL_REGISTER	0x14
// Project constants
const int MAX_CHANNEL_VALUE = 65000;
const int DEFAULT_BULB_ENABLE = 0x07;

// Enums and constants
enum SensorType : byte {
	NO_SENSOR, AS7262_SENSOR, AS7263_SENSOR, AS7265X_SENSOR
};

class SpectroDesktop {
public:
	SpectroDesktop();
	QwiicButton button;  // this will represent EVERY button so its not an array
	AS726X as726x;  // treat all as726x the same
	AS7265X as7265x;  // treat all as7265x the same
	byte enableBulbsArray[8] {DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE, 
	                          DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE, DEFAULT_BULB_ENABLE};
	bool begin(TwoWire &wirePort = Wire);
	void pollButtons();
	void readAS7262(byte portNumber);
	void readAS7263(byte portNumber);
	void readAS7265x(byte portNumber);

private:
	TwoWire *_i2cPort;
	bool use_mux = false;  // flag if there is a mux or not
	SensorType sensor_type_array[8] {NO_SENSOR, NO_SENSOR, NO_SENSOR, NO_SENSOR, NO_SENSOR, NO_SENSOR, NO_SENSOR, NO_SENSOR};
	SensorType getSensorType(byte channel);
	void getAS7262Data();
	void getAS7263Data();
	void getAS7265xData();
	bool enableMuxPort(byte portNumber);
	byte getMuxSettings();
	bool sendMuxSettings(byte _settings);
	bool checkI2cAddress(byte _addr);
};

#endif
