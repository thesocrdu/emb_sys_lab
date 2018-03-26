/**
 * @file
 * @brief This file implements the class structure
 * for the QoBUP interface.
 *
 * @author Kyle Mercer
 *
 */

#include <Arduino.h>
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

q_status_msg_t QoBUP::serialRxMsg(Stream &s, uint8_t* const cmdBuff, uint8_t size) {

    q_status_msg_t status;
    q_message_header_t* const startPtr = reinterpret_cast<q_message_header_t* const>(cmdBuff);
    uint8_t msgSize;
    unsigned long time;

    status.status.word = 0;

    if (cmdBuff == NULL) {
        status.status.word = 0xff;
        return status;
    }

    /* Wait for data on the serial line */
    while (s.available() < static_cast<signed int>(sizeof(q_message_header_t)));

    /*
     * Read the wc field from msg and attempt
     * to read those many bytes from serial
     * or timeout and set status accordingly.
     */
    startPtr->id = s.read(); /* First word is msg ID */
    startPtr->wc = s.read();
    startPtr->sid = s.read();
    msgSize = startPtr->wc;
    status.sid = startPtr->sid;

    /* Check our incoming message will fit into buffer */
    if (size < msgSize) {
        status.status.bad_size = 1;
        return status;
    }

    time = millis();
    for (size_t i = sizeof(q_message_header_t); i < msgSize; i++) {
        while (!s.available()) {
            if ((millis() - time) > Q_SERIAL_TIMEOUT_MS) {
                status.status.timeout = 1;
                return status;
            }
        }
        cmdBuff[i] = s.read();
    }

    /* Run standard validation on message */
    return validateMessage(cmdBuff);
}
