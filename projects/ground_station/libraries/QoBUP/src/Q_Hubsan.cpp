/**
 * @file
 * @brief This file implements the class structure
 * for the Q_Hubsan class.
 *
 * @author Kyle Mercer
 *
 */

#include "Q_Hubsan.h"
#include "QoBUP.h"
#include <string.h>

Q_Hubsan::Q_Hubsan() {

    /* Init the flight control structure to nominal Hubsan values. */
    memset(&_currFlightCntls, 0, sizeof(_currFlightCntls));

    _currFlightCntls.header = 0x20;
    _currFlightCntls.yaw = 0x80;
    _currFlightCntls.pitch = 0x80;
    _currFlightCntls.roll = 0x80;
    _currFlightCntls.flags.word = 0x0e;
    _currFlightCntls.setTo0x19 = 0x19;

}

Q_Hubsan::~Q_Hubsan() {
}

int Q_Hubsan::parseMessage(const uint8_t* const cmd) {

    const q_message_header_t* const startPtr =
        reinterpret_cast<const q_message_header_t*>(cmd);;
    const uint8_t msgSize = startPtr->wc;
    const uint8_t *currPtr, *eomHeader;

    if (getCurrStatus().status.word != 0) {
        return -1;
    }

    currPtr = reinterpret_cast<const uint8_t*>(startPtr) + sizeof(q_message_header_t);
    eomHeader = reinterpret_cast<const uint8_t*>(startPtr) +
        msgSize - sizeof(q_generic_block_t);

    while (currPtr < eomHeader) {

        /* Overlay generic block pointer */
        const q_generic_block_t *currBlock =
            reinterpret_cast<const q_generic_block_t*>(currPtr);

        /* Check if we have a block ID match. */
        switch (*currPtr) {

            case Q_BLOCK_ID_FLIGHT_CONTROL:
                translateAllFlightControls(
                        reinterpret_cast<const q_all_flight_control_block_t*>(currPtr));

                currPtr += currBlock->wc;
                break;

            case Q_BLOCK_ID_THROTTLE:
                translateThrottle(
                        reinterpret_cast<const q_single_flight_control_block_t*>(currPtr));

                currPtr += currBlock->wc;
                break;

            case Q_BLOCK_ID_YAW:
                translateYaw(
                        reinterpret_cast<const q_single_flight_control_block_t*>(currPtr));

                currPtr += currBlock->wc;
                break;

            case Q_BLOCK_ID_PITCH:
                translatePitch(
                        reinterpret_cast<const q_single_flight_control_block_t*>(currPtr));

                currPtr += currBlock->wc;
                break;

            case Q_BLOCK_ID_ROLL:
                translateRoll(
                        reinterpret_cast<const q_single_flight_control_block_t*>(currPtr));

                currPtr += currBlock->wc;
                break;

            default:
                /*
                 * This should never happen
                 */
                return -1;
        }
    }

    return 0;
}


void Q_Hubsan::translateThrottle(
        const q_single_flight_control_block_t* const thStruct) {

    _currFlightCntls.throttle = thStruct->val;
}

void Q_Hubsan::translateYaw(
        const q_single_flight_control_block_t* const yawStruct) {

    _currFlightCntls.yaw = yawStruct->val;
}

void Q_Hubsan::translatePitch(
        const q_single_flight_control_block_t* const pitchStruct) {

    _currFlightCntls.pitch = pitchStruct->val;
}

void Q_Hubsan::translateRoll(
        const q_single_flight_control_block_t* const rollStruct) {

    _currFlightCntls.roll = rollStruct->val;
}

void Q_Hubsan::translateAllFlightControls(
        const q_all_flight_control_block_t* const flightStruct) {

    _currFlightCntls.throttle = flightStruct->throttle;
    _currFlightCntls.yaw = flightStruct->yaw;
    _currFlightCntls.pitch = flightStruct->pitch;
    _currFlightCntls.roll = flightStruct->roll;
}
