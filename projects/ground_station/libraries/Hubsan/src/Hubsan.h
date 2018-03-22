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
         * 
         */
        void init(int cspin);
        void bind();
        void txPacket(uint8_t *ppacket);
        void rxPacket(uint8_t *ppacket);
        void getChecksum(uint8_t *ppacket);

};
#endif /* HUBSAN_H */
