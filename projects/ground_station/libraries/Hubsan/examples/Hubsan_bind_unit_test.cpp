#include "Hubsan.h"

#define CS_PIN 9

Hubsan hubs;

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");

    hubs.init(CS_PIN);
}

void loop(void) {

    uint16_t hubsanWait = hubs.hubsan_cb();
    delayMicroseconds(hubsanWait);

}
