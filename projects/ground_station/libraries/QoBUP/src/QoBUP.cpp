/**
 * @file
 * @brief This file implements the class structure
 * for the QoBUP interface.
 *
 * @author Kyle Mercer
 *
 */

#include "QoBUP.h"

QoBUP::QoBUP() {
   /* Set the uninitialized bit. */
    _curr_status.sid = -1;
    _curr_status.status.word = 0x08;
}

QoBUP::~QoBUP() {}

q_status_msg_t QoBUP::getCurrStatus() {

    return _curr_status;
}

q_status_msg_t QoBUP::validateMessage(const uint8_t* const cmd) {

    _curr_status.status.word = levelOneValidation(cmd).word;
    return _curr_status;
}
#include <HardwareSerial.h>
q_status_t QoBUP::levelOneValidation(const uint8_t* const cmd) {

    const q_message_header_t* const startPtr = reinterpret_cast<const q_message_header_t*>(cmd);
    const uint8_t msgSize = startPtr->wc;
    const uint8_t *currPtr, *eomHeader;
    q_status_t retval;
 
    retval.word = 0;
    _curr_status.sid = startPtr->sid;

    /* Check we're within max msg size */
    if (msgSize > Q_MAX_SIZE_CMD_BYTES) {
        retval.bad_size = 1;
        Serial.println("A");
        return retval;
    }

    /* Check the msg header is valid */
    switch (startPtr->id) {
        case Q_MSG_ID_CONTROL:
            break;

        default:
            retval.bad_msg_id = 1;
            return retval;
    }

    /* Check for end of msg header and get end ptr */
    eomHeader = reinterpret_cast<const uint8_t*>(startPtr) +
        msgSize - sizeof(q_generic_block_t);
    if (*eomHeader != Q_BLOCK_ID_EOM) {
        retval.bad_size = 1;
        return retval;
    }
 
    /* Set start pointer */
    currPtr = reinterpret_cast<const uint8_t*>(startPtr) + sizeof(q_message_header_t);

    /* Loop through message and check all block IDs */
    while (currPtr < eomHeader) {

        /* Overlay generic block pointer */
        const q_generic_block_t *currBlock = reinterpret_cast<const q_generic_block_t*>(currPtr);

        /* check if we have a block ID match. */
        switch (*currPtr) {
            case Q_BLOCK_ID_FLIGHT_CONTROL:
            case Q_BLOCK_ID_THROTTLE:
            case Q_BLOCK_ID_YAW:
            case Q_BLOCK_ID_PITCH:
            case Q_BLOCK_ID_ROLL:
                currPtr += currBlock->wc;
                break;

            default:
                /* Set false while condition and error bit */
                currPtr = eomHeader;
                retval.bad_block_id = 1;
                break;
        }
    }
    return retval;
}
