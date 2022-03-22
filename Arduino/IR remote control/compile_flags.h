#pragma once


#define COMPILE_TESTMODE
//#define COMPILE_RUNMODE

// general flags used during development and debugging
#define DEBUGLEVEL 5					// Has dual function: 1) serving as default value for Firebase Logging (which can be modified at runtime). 2) Defining debug level on serial port.
#define FORCE_NEW_VALUES false			// [default:false] True will overwrite all values in persistent memory. 
										// To use: Enable once, disable and recompile
//#define USE_DEEP_SLEEP				   // [true] When enabling, connect D0 to RST (on Wemos D1 mini)
//#define RUN_ONCE					   // Debug mode: no looping, just execute once


// ====================================================
#ifdef COMPILE_TESTMODE
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, Aircon Remote Control"
	#define DEVICE_ID "#1"
	#define USE_WIFI
	#define USE_FIREBASE
	// Program control:

	// Development & debugging

	// Hardware
	#define D1MINI

#endif //COMPILE_TESTMODE

	// ====================================================
#ifdef COMPILE_RUNMODE
	#define USE_DISTANCE_SENSOR 
	const String DefaultRunmode = RUNMODE_DISTANCE;
	//const String DefaultRunmode = RUNMODE_SENSORTEST;
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, Distance, solar"
	#define DEVICE_ID "Dist_#1"
	#define USE_WIFI
	#define USE_FIREBASE
	#define NBR_OF_LOOPS_BEFORE_SLEEP 1    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 2                   // [10] secs
	#define TOTAL_SECS_TO_SLEEP 5		   // [20] Default sleep time
	#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.

	// Not relevant for this device but still needed for compilation
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	#define SIMULATE_WATERING true

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;
	const int CHANNEL_TEMPERATURE = 2;
#endif  //COMPILE_RUNMODE


//Hardware pin configuration on WeMOS D1
#ifdef D1MINI
#define IR_TX_LED D2  // ESP8266 GPIO pin to use. Recommended: 4 (D2). (13 on a WeMOS D1 R2 board)
#define IR_RX_LED D5
#else //D1 R2
#define IR_TX_LED 13  // ESP8266 GPIO pin to use. Recommended: 4 (D2). (13 on a WeMOS D1 R2 board)
#define IR_RX_LED 14
#endif
