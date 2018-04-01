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
        int init(const unsigned int cspin);

        /**
         *
         */
        void update_crc();

        /**
         *
         */
        void hubsan_build_bind_packet(uint8_t state);

        /**
         *
         */
        int16_t get_channel();

        /**
         *
         */
        void hubsan_build_packet();

        /**
         *
         */
        uint16_t hubsan_cb();

        void txPacket(uint8_t *ppacket);
        void rxPacket(uint8_t *ppacket);
        void getChecksum(uint8_t *ppacket);

    private:

        /** The @sa A7105 interface for this Hubsan object. */
        A7105 _a7105;

        /** Internal Hubsan bind stages. */
        enum {
            BIND_1,
            BIND_2,
            BIND_3,
            BIND_4,
            BIND_5,
            BIND_6,
            BIND_7,
            BIND_8,
            DATA_1,
            DATA_2,
            DATA_3,
            DATA_4,
            DATA_5,
        };

        /** Current bind state for this @sa Hubsan. */
        uint8_t state;

        /** Buffer used for all packet transmissions. */
        uint8_t packet[16];

        /** The selected channel ID. Should be a value from @sa allowed_ch array. */
        uint8_t channel;

        /** Available channel IDs for the Hubsan. */
        const static uint8_t allowed_ch[];

        /** Session ID for this tranmission session. Randomly generated. */
        uint32_t sessionid;

        /**
         * The TX ID. Potentially not needed
         * but currently being set at @sa packet[11]
         * in reference code.
         */
        const static uint32_t txid = 0xdb042679;

};
#endif /* HUBSAN_H */
