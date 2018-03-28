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

extern volatile uint8_t throttle, rudder, aileron, elevator;

void loop(void) {

    if(Serial.available()) {
        uint8_t in = Serial.read();
        switch(in) {
            case 'a':
                throttle = 0;
                rudder = aileron = elevator = 0x80;
                break;
            default:
                throttle = (uint8_t) rand() % 0x30;
                rudder = aileron = elevator = 0x80;
                break;
        }
        Serial.println(throttle);
    }

    uint16_t hubsanWait = hubs.hubsan_cb();
    delayMicroseconds(hubsanWait - 400);

}
