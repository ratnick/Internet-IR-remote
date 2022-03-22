/*
 Name:		IR_remote_control.ino
 Created:	11/11/2020 9:37:08 PM
 Author:	Nikolaj (lokal)
*/


// To get the time
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// To use Blynk
#include <Blynk.h>
#include <CertStoreBearSSL.h>
#include <BearSSLHelpers.h>

// To use IR Remote library
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <ir_Mitsubishi.h>
#include <ir_Daikin.h>

// Including the libraries below, allows to turn off Deep Search in libraries => faster compiling
#include "FirebaseLib.h"
#include <WifiLib.h>
#include <SysCall.h>
#include <sdios.h>
#include <BlockDriver.h>
#include <SDFSFormatter.h>
#include <SdFatConfig.h>
#include <SdFat.h>
#include <MinimumSerial.h>
#include <FreeStack.h>
#include <SDFS.h>
#include <SD.h>
#include <FirebaseJson.h>    // works with 2.2.8, but NOT 2.3.9. This will give random errors in communication with Firebase. I think it's becaise FirebaseJSON struct or class is changed
// end 

// OTA upload
#include <LEAmDNS_Priv.h>
#include <ArduinoOTA.h>
// end

#include "compile_flags.h"

#ifdef USE_WIFI
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#endif

#ifdef USE_FIREBASE
#include <SPI.h>
#include <FirebaseESP8266HTTPClient.h>
#endif


#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHTesp.h>
#include <EEPROM.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager
#include <time.h>

// own libraries
#include "PersistentMemory.h"
#include "globals.h"
#include "ciotc_config.h" // Wifi configuration here
#include "LEDHandler.h"
#include "OTALib.h"
#include <LogLib.h>


#define D1MINI
#define MITSIBISHI
//#define DAIKIN
//#define RECEIVE
//#define CANDLE_LIGHT_TEST

// Codes are in Global Cache format less the emitter ID and request ID.
// These codes can be found in GC's Control Tower database.

uint16_t Samsung_power_toggle[71] = {
    38000, 1,  1,  170, 170, 20, 63, 20, 63, 20, 63,  20, 20, 20, 20,
    20,    20, 20, 20,  20,  20, 20, 63, 20, 63, 20,  63, 20, 20, 20,
    20,    20, 20, 20,  20,  20, 20, 20, 20, 20, 63,  20, 20, 20, 20,
    20,    20, 20, 20,  20,  20, 20, 20, 20, 63, 20,  20, 20, 63, 20,
    63,    20, 63, 20,  63,  20, 63, 20, 63, 20, 1798 };

#ifdef MITSIBISHI
    IRMitsubishiAC ac(IR_TX_LED);  // Set the GPIO used for sending messages.
    // Blynk
    // You should get Auth Token in the Blynk App or via sign-on mail from Blynk.
    // Go to the Project Settings (nut icon).
    char auth[] = "pmbpuez54MNHS9waqM_gNpMJI21CL9Wk";   // Nikolaj, Mitsubishi
// Your WiFi credentials.
// Set password to "" for open networks.
    char ssid[] = "nohrTDC";
    char pass[] = "RASMUSSEN";
#endif

#ifdef DAIKIN
    IRDaikinESP ac(IR_TX_LED);  // Set the GPIO used for sending messages.
    char auth[] = "DwfsqL9OjsUV2qwxuUSmgTOaNIDbtuHX";   // Nikolaj, Mitsubishi
    char ssid[] = "Fibernet-IA01942298";
    char pass[] = "bo2F86dm";
#endif



const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
    delayNonBlocking(500);
    Serial.begin(115200);

    InitDebugLevel(DEBUGLEVEL);
    PersistentMemory.SetdebugLevel(DEBUGLEVEL);

    // read persistent memory
    PersistentMemory.init(FORCE_NEW_VALUES, TOTAL_SECS_TO_SLEEP, DEVICE_ID, HARDWARE_DESCRIPTION, DefaultRunmode.c_str(), VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize
    PersistentMemory.Printps();
    //PersistentMemory.UnitTest();  // just for testing
    //	PersistentMemory.SetdebugLevel(debuglevel);
    Serial.printf("persisten mem debugLevel=%d (should be %d)\n", PersistentMemory.GetdebugLevel(), DEBUGLEVEL);
    SetFBDebugLevel(PersistentMemory.GetdebugLevel());
    initFlashLED();
    LogLine(4, __FUNCTION__, "A");

#ifdef RECEIVE
        irrecv.enableIRIn();  // Start the receiver
        Serial.println("Receiving...");
    #else
        #ifdef CANDLE_LIGHT_TEST
            ac.begin();
        #else
            delay(2000);
            Blynk.begin(auth, ssid, pass);

            // get current time  https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            timeClient.begin();
            timeClient.update();
            Serial.printf("\nD1 local time: ");
            Serial.println(timeClient.getFormattedTime());

            ac.begin();
            Serial.println("Default state of the remote.");
            CommonStandardSettings();
            printState();

        #endif
    #endif
    while (!Serial)  // Wait for the serial connection to be establised.
        delay(50);
    Serial.printf("\nRunning");
}

void loop() {
    #ifdef RECEIVE
        ReceiveIR();
    #else
        #ifdef CANDLE_LIGHT_TEST
            // LED: LOW = on, HIGH = off
            Serial.println("Start blinking");
            for (int i = 0; i < 20; i++)
            {
                digitalWrite(BUILTIN_LED, LOW);
                delay(100);
                digitalWrite(BUILTIN_LED, HIGH);
                delay(100);
            }
            /*    Serial.println(0);
                IRsend ac00(0);  // Set the GPIO to be used to sending the message.
                ac00.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(1);
                IRsend ac01(1);  // Set the GPIO to be used to sending the message.
                ac01.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(2);
                IRsend ac02(2);  // Set the GPIO to be used to sending the message.
                ac02.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(3);
                IRsend ac03(3);  // Set the GPIO to be used to sending the message.
                ac03.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(4);
                IRsend ac04(4);  // Set the GPIO to be used to sending the message.
                ac04.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(5);
                IRsend ac05(5);  // Set the GPIO to be used to sending the message.
                ac05.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(6);
                IRsend ac06(6);  // Set the GPIO to be used to sending the message.
                ac06.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(7);
                IRsend ac07(7);  // Set the GPIO to be used to sending the message.
                ac07.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(8);
                IRsend ac08(8);  // Set the GPIO to be used to sending the message.
                ac08.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(9);
                IRsend ac09(9);  // Set the GPIO to be used to sending the message.
                ac09.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(10);
                IRsend ac10(10);  // Set the GPIO to be used to sending the message.
                ac10.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(11);
                IRsend ac11(11);  // Set the GPIO to be used to sending the message.
                ac11.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(12);
                IRsend ac12(12);  // Set the GPIO to be used to sending the message.
                ac12.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(13);
                IRsend ac13(13);  // Set the GPIO to be used to sending the message.
                ac13.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(14);
                IRsend ac14(14);  // Set the GPIO to be used to sending the message.
                ac14.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(15);
                IRsend ac15(15);  // Set the GPIO to be used to sending the message.
                ac15.sendNEC(CandleLight_power_on);
                delay(4000);
                Serial.println(16);
                IRsend ac16(16);  // Set the GPIO to be used to sending the message.
            */
            SendIR();
        #else
            //UnitTest();
            Blynk.run();
        #endif
    #endif
}

#ifdef CANDLE_LIGHT_TEST
    uint64_t CandleLight_power_off = 0xFF906F; // NEC FF906F
    uint64_t CandleLight_power_on = 0xFF807F;  // NEC FF807F
    IRsend ac(IR_TX_LED);  // Set the GPIO to be used to sending the message.
    // An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
    // board).
    // Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
    IRrecv irrecv(IR_RX_LED);
    decode_results results;
    // Your WiFi credentials.
    // Set password to "" for open networks.
    char ssid[] = "nohrTDC";
    char pass[] = "RASMUSSEN";
#else
    void CommonStandardSettings() {   // Personal preferences
        ac.on();
        ac.stateReset();
        ac.setISave10C(false);
        ac.setMode(kMitsubishiAcHeat);
        ac.setEcocool(false);
        ac.setNaturalFlow(false);
        ac.setFan(kMitsubishiAcFanAuto);
        ac.setVane(kMitsubishiAcVaneHighest);   // right set high to point towards bathroom
        ac.setVaneLeft(kMitsubishiAcVaneAuto);  // let it find out itself
        ac.setWideVane(kMitsubishiAcWideVaneAuto);
        ac.setAbsenseDetect(false);
        ac.setISee(true);
        ac.setDirectIndirect(kMitsubishiAcDirect);
        ConvertCurrentTimeToACFormat();  // sets the time
        ac.setTimer(kMitsubishiAcNoTimer);
        ac.setWeeklyTimerEnabled(false);
    }

    void BlynkV0() {
        Serial.println("RESET A/C");
        CommonStandardSettings();
        ac.send();
        printState();
        delay(5000);
    }
    BLYNK_WRITE(V0) { 
        BlynkV0(); 
    }

    // Parameter is temperature
    void BlynkV1(int temp) {
        Serial.printf("SET TEMPERATURE TO %d \n", temp);
        ConvertCurrentTimeToACFormat();  // sets the time
        ac.setTemp(temp);
        ac.send();
        printState();
        delay(5000);
    }
    BLYNK_WRITE(V1) { 
        int temp = param.asInt();
        BlynkV1(temp); 
    }

    // Parameter is new mode (1, 2 or 3)
    void BlynkV2(int newMode) {
        CommonStandardSettings();
        switch (newMode) {
        case 1:
            Serial.println("TURN OFF");
            ac.off();
            break;
        case 2:
            Serial.println("Turn ON and set to and set to Heating and I-SAVE at 10 deg.");
            ac.setTemp(kMitsubishiAcMinTemp);
            ac.setFan(kMitsubishiAcFanAuto);
            ac.setDirectIndirect(kMitsubishiAcDirectOff);
            ac.setISee(false);
            ac.setISave10C(true);
            break;
        case 3:
            Serial.println("V26: Turn ON and set to Heating at 26 deg. Set fan and vane to AUTO.");
            BlynkV0(); // reset first
            CommonStandardSettings();
            ac.setTemp(26);
            ac.setWeeklyTimerEnabled(true);
            break;
        default:
            Serial.printf("V2: Unsupported parameter: %d\n", newMode);
            break;
        }
        ac.send();
        printState();
        delay(5000);
    }
    BLYNK_WRITE(V2) { 
        int newMode = param.asInt();
        BlynkV2(newMode);
    }

    void BlynkV3(int dummy) {
        Serial.printf("SET WEEKLY TIMER\n");
        SetWeeklyTimer();   // transmits to A/C
        printState();
        delay(5000);
    }
    BLYNK_WRITE(V3) {
        int dummy = param.asInt();
        BlynkV3(dummy);
    }

    void BlynkV4(int dummy) {
        Serial.printf("iSAVE standard setting\n");
        SendRawLineToAC("23CB260100200800C0B86A00000092203809");   // transmits to A/C
        printState();
        delay(5000);
    }
    BLYNK_WRITE(V4) {
        int dummy = param.asInt();
        BlynkV4(dummy);
    }

    void SetWeeklyTimer() {
        SendRawLineToAC("23CB26010D0001212E41226A2E870E000002");
        SendRawLineToAC("23CB26010D0100212E41228616870E000006");
        SendRawLineToAC("23CB26010D0200212E41228616870E000007");
        SendRawLineToAC("23CB26010D0300212E41228616870E000008");
        SendRawLineToAC("23CB26010D0400212E41228616870E000009");
        SendRawLineToAC("23CB26010D0500212E41228616870E00000A");
        SendRawLineToAC("23CB26010D0602212E41228616870E00000D");
    }

    // hexString MUST BE 36 chars long (Mitsubishi 144-bit protocol)
    void SendRawLineToAC(char* hexString) {

        const int LEN = 36 / 2;   // half the amount of chars in the Hex string
        char hexStr[2];
        unsigned long number;

        uint8_t raw_array[LEN];

        for (int i = LEN - 1; i >= 0; i--)    // start with lowest byte of number
        {
            hexStr[0] = hexString[2 * i];
            hexStr[1] = hexString[2 * i + 1];

            number = strtoul(hexStr, nullptr, 16);
            raw_array[i] = number;
        }

        Serial.printf("0x%s\n", hexString);
        Serial.print("0x");
        for (int i = 0; i < LEN; i++)
        {
            Serial.printf("%02X", raw_array[i]);
        }
        Serial.printf(".\n\n");

        ac.setRaw(raw_array);
        ac.send();
        delay(100);
    }

    uint8_t ConvertCurrentTimeToACFormat() {
        timeClient.update();
        uint8_t ticksSinceMidnight = 6 * timeClient.getHours() + uint8_t(timeClient.getMinutes() / 10);
        ac.setClock(ticksSinceMidnight);
        /*
        Serial.print(daysOfTheWeek[timeClient.getDay()]);
        Serial.print(", ");
        Serial.print(timeClient.getHours());
        Serial.print(":");
        Serial.print(timeClient.getMinutes());
        Serial.print("    :    ");
        Serial.println(timeClient.getSeconds());
        Serial.print(":");
        Serial.println(ticksSinceMidnight);
        */
    }

    void printState() {
        // Display the settings.
        // Serial.println("A/C remote is in the following state:");
        // Display the encoded IR sequence.
        unsigned char* ir_code = ac.getRaw();
        Serial.printf("Code      : 0x");
        for (uint8_t i = 0; i < kMitsubishiACStateLength; i++)
            Serial.printf("%02X", ir_code[i]);
        Serial.printf(" : %s  -  \n\n", ac.toString().c_str());
    }
#endif

    void UnitTest() {
        BlynkV0();
        BlynkV1(26);
        BlynkV2(1);
        BlynkV2(2);
        BlynkV2(3);
        delay(20000);
    }


#ifdef RECEIVE
    void ReceiveIR() {
        if (irrecv.decode(&results)) {
            // print() & println() can't handle printing long longs. (uint64_t)
            serialPrintUint64(results.value, HEX);
            Serial.println("");
            irrecv.resume();  // Receive the next value
        }
        delay(100);
    }
#endif

#ifdef CANDLE_LIGHT_TEST
    void SendIR() {
        Serial.println("Toggling power ON");
        #if SEND_GLOBALCACHE
            //ac.sendGC(Samsung_power_toggle, 71);
            ac.sendNEC(CandleLight_power_on);
            delay(2000);
            Serial.println("Toggling power OFF");
            ac.sendNEC(CandleLight_power_off);
        #else   // SEND_GLOBALCACHE
            Serial.println("Can't send because SEND_GLOBALCACHE has been disabled.");
        #endif  // SEND_GLOBALCACHE
        delay(2000);
    }
#endif
