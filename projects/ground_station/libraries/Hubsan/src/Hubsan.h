/**
 * @file
 * @brief This file outlines the class structure
 * for the Hubsan H107C Quadcopter
 *
 * @author Kyle Mercer
 *
 */

#ifndef HUBSAN_H
#define HUBSAN_H

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#include <A7105.h>
#include <Arduino.h>
#include <stdint.h>

/**
 * This class provides an interface for controlling the
 * Hubsan H107C Quadcopter.
 */
class Hubsan {
    public:

        /** Constructor */
        Hubsan();

        /** Destructor */
        ~Hubsan();

        /**
         * Initialize the A7105 RF module for
         * communication with the Hubsam quadcopter.
         *
         * @param[in] cspin The chip select pin of the A7105.
         * @return zero if init was successful.
         */
        int init(int cspin);
        void txPacket(uint8_t *ppacket);
        void rxPacket(uint8_t *ppacket);
        void getChecksum(uint8_t *ppacket);

    private:

        /** The @sa A7105 interface for this Hubsan object. */
        A7105 _a7105;

};
#endif /* HUBSAN_H */
