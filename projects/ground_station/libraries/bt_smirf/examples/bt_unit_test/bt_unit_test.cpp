#include <Arduino.h>
#include "bt_smirf.h"

static bt_smirf bt(Serial2);
void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");
    bt.begin(115200);
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
        //Serial.write(in);
        Serial2.write(in);
    }
    if (Serial2.available()) {
        in = Serial2.read();
        Serial.write(in);
    }
}
