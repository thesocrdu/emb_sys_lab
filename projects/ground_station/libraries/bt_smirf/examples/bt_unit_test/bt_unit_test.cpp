#include <Arduino.h>
#include "bt_smirf.h"

static SoftwareSerial ss(4, 3);

static bt_smirf bt(ss);
void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");
    bt.begin(9600);
    Serial.println("Toggling module into CMD mode...");

//    bt.resetCmdQueue();
//    if (bt.exitCmdMode() == 0) {
//        Serial.println("Success!");
//    } else {
//        Serial.println("Error: Unable to switch out of CMD mode");
//    }
//    if (bt.enterCmdMode() == 0) {
//        Serial.println("Success!");
//    } else {
//        Serial.println("Error switching back into CMD mode");
//    }
    Serial.println("Enter input to send to BlueSMiRF");
}

int cnt = 0;
void loop(void) {
    uint8_t in;
    if (Serial.available()) {
        in = Serial.read();
        Serial.write(in);
        ss.write(in);
    }
    if (ss.available()) {
        in = ss.read();
        Serial.write(in);
    }
}
