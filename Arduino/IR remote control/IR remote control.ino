/*
 Name:		IR_remote_control.ino
 Created:	11/11/2020 9:37:08 PM
 Author:	Nikolaj (lokal)
*/

#include "Blynk.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <ir_Mitsubishi.h>
#include <ir_Daikin.h>

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

#ifdef D1MINI
#define IR_TX_LED D2  // ESP8266 GPIO pin to use. Recommended: 4 (D2). (13 on a WeMOS D1 R2 board)
#define IR_RX_LED D5
#else //D1 R2
#define IR_TX_LED 13  // ESP8266 GPIO pin to use. Recommended: 4 (D2). (13 on a WeMOS D1 R2 board)
#define IR_RX_LED 14
#endif

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
    void printState() {
        // Display the settings.
        Serial.println("A/C remote is in the following state:");
        Serial.printf("  %s\n", ac.toString().c_str());
        // Display the encoded IR sequence.
        unsigned char* ir_code = ac.getRaw();
        Serial.print("IR Code: 0x");
        for (uint8_t i = 0; i < kMitsubishiACStateLength; i++)
            Serial.printf("%02X", ir_code[i]);
        Serial.println();
    }




BLYNK_WRITE(V0) {  // CANDLE LIGHT ON
    if (param.asInt() == 1) {
        Serial.println("ON");
        ac.on();
    }
    else {
        Serial.println("OFF");
        ac.off();
    }
    ac.send();
    printState();
    delay(5000);
}

BLYNK_WRITE(V1) {  // TEMPERATURE
    Serial.println("TEMPERATUR");
    ac.setTemp(param.asInt());
    ac.send();
    printState();
    delay(5000);
}

#endif

void setup() {
    Serial.begin(115200);


    #ifdef RECEIVE
        irrecv.enableIRIn();  // Start the receiver
    #else
        #ifdef CANDLE_LIGHT_TEST
            ac.begin();
        #else
            ac.begin();
            Serial.println("Default state of the remote.");
            printState();
            Blynk.begin(auth, ssid, pass);
        #endif
    #endif

    while (!Serial)  // Wait for the serial connection to be establised.
        delay(50);
    Serial.printf("\nRunning");
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
            Blynk.run();
        #endif
    #endif
}
