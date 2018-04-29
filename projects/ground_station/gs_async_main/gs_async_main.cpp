/**
 * @file
 * @brief This file serves as the top level
 * driver program for the entire ground station.
 *
 * @author Kyle Mercer
 *
 */

#include <bt_smirf.h>
#include <Hubsan.h>
#include <Q_Hubsan.h>


#ifdef GS_DEBUG

#include <SoftwareSerial.h>
#define BT_RX_PIN 3
#define BT_TX_PIN 4
#define BT_SERIAL_IF softSerial
#define BT_BAUD 9600
static SoftwareSerial BT_SERIAL_IF(BT_RX_PIN, BT_TX_PIN);

#else

#define BT_SERIAL_IF Serial
#define BT_BAUD 57600

#endif

#define CMD_BUFF_SIZE 32
#define CS_PIN 9
#define HUBSAN_TX_PERIOD_US 10000

#define BIND_LED_PIN 2
#define TRAINING_LED_PIN A0
#define TRAINING_BUT_PIN A1
#define A7105_TX_EN_PIN A2

static bt_smirf bt(BT_SERIAL_IF);
static Q_Hubsan qh;
static Hubsan hubs;

uint8_t cmd[CMD_BUFF_SIZE];
q_hubsan_flight_controls_t fltCnt;
q_status_msg_t status;
unsigned long txTimestamp = 0;
bool trainingEnabled = false;

void hubsanControlUpdate() {

    hubs.hubsan_send_data_packet(0);
}

void initSerialDebug(void) {
    
#ifdef GS_DEBUG
    Serial.begin(115200);
    while(!Serial){}

    Serial.write(27);
    Serial.print("[2J");
#endif
}

void initBluetoothInterface(void) {

    bt.begin(BT_BAUD);
    bt.exitCmdMode();
}

void initHubsanInterface(void) {

    pinMode(BIND_LED_PIN, OUTPUT);
    digitalWrite(BIND_LED_PIN, LOW);
    hubs.init(CS_PIN);

    qh.getFlightControls(fltCnt);
    hubs.updateFlightControlPtr(&fltCnt);
    hubs.bind();

    pinMode(A7105_TX_EN_PIN, OUTPUT);
    digitalWrite(A7105_TX_EN_PIN, HIGH);

    /* Toggle Bind LED to let user know we're ready. */
    digitalWrite(BIND_LED_PIN, HIGH);
    delay(200);
    digitalWrite(BIND_LED_PIN, LOW);
    delay(200);
    digitalWrite(BIND_LED_PIN, HIGH);
    delay(200);
    digitalWrite(BIND_LED_PIN, LOW);
    delay(200);
    digitalWrite(BIND_LED_PIN, HIGH);
    delay(200);
    digitalWrite(BIND_LED_PIN, LOW);
    delay(200);
    digitalWrite(BIND_LED_PIN, HIGH);
}

void initTrainingFeature() {

    pinMode(TRAINING_LED_PIN, OUTPUT);
    pinMode(TRAINING_BUT_PIN, INPUT_PULLUP);

    trainingEnabled = false;
    digitalWrite(TRAINING_LED_PIN, LOW);
}

void handleTrainingButtonEvent() {

    static int lastButtonState = HIGH;
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 50;

    static int ledState = LOW;
    static int buttonState = HIGH;

    // read the state of the switch into a local variable:
    int reading = digitalRead(TRAINING_BUT_PIN);
    //digitalWrite(TRAINING_LED_PIN, reading);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:

        // if the button state has changed:
        if (reading != buttonState) {
            buttonState = reading;

            // only toggle the LED if the new button state is HIGH
            if (buttonState == HIGH) {
                ledState = !ledState;
            }
        }
    }
    // set the LED:
    digitalWrite(TRAINING_LED_PIN, ledState);

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState = reading;
}

void initTimer() {

    txTimestamp = micros();
}

void sendStatusResp() {
    BT_SERIAL_IF.write(status.sid);
    BT_SERIAL_IF.write(status.status.word);
}

void setup(void) {

    initSerialDebug();
    initBluetoothInterface();
    initHubsanInterface();
    initTrainingFeature();
    initTimer();
}

void loop(void) {

    if (BT_SERIAL_IF.available() >= 3) {
        status = qh.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);

        if (status.status.word != 0) {
#ifdef GS_DEBUG
            Serial.print("Err: status = ");
            Serial.println(status.status.word, HEX);
#endif
        } else if (qh.parseMessage(cmd) != 0) {
#ifdef GS_DEBUG
            Serial.println("Err: failed to parse message");
#endif
        } else {
            qh.getFlightControls(fltCnt);
            hubs.updateFlightControlPtr(&fltCnt);
            //printFltControls();
        }
        sendStatusResp();
    }

    handleTrainingButtonEvent();

    /*
     * Wait 10ms since last update to send
     * new controls to Hubsan quadcopter.
     */
    while(txTimestamp - micros() < HUBSAN_TX_PERIOD_US);
    txTimestamp = micros();
    hubsanControlUpdate();
}
