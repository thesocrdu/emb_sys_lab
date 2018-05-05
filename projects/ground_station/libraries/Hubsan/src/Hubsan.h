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
#include <Q_Hubsan.h>
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
         * Updates the internal pointer to the neweset
         * set of controls available.
         * @param[in] pointer to the new controls struct.
         */
        void updateFlightControlPtr(q_hubsan_flight_controls_t* const newControls);

        /**
         * Initiates the binding procedure for the Hubsan
         *
         * @note Refer to the protocol spec published by Jim Hung
         *       http://www.jimhung.co.uk/wp-content/uploads/2014/11/HubsanX4_ProtocolSpec_v1.txt
         *
         */
        void bind();

        /**
         * Pushes updated controls to the @sa A7105
         * interface for transmit to the Hubsan.
         */
        void hubsan_send_data_packet();

        /**
         * Set the LEDS on the Hubsan
         * @param[in] on Set @sa true to turn LEDs on
         */
        void setLedState(const bool on);

    private:

        /**
         * Updates the CRC field in the @sa currFlightControls.
         */
        void update_flight_control_crc();

        /**
         * Updates the last element byte of the passed array
         * with the CRC checksum. Used during the binding phase.
         * @param[in] ppacket The byte packet of size 16
         */
        void getChecksum(uint8_t *ppacket);

        /** The @sa A7105 interface for this Hubsan object. */
        A7105 _a7105;

        /** Buffer used for all packet transmissions. */
        uint8_t packet[16];

        /** Pointer to the current flight controls we are transmitting. */
        q_hubsan_flight_controls_t *currFlightControls;

        /** The selected channel ID. Should be a value from @sa allowed_ch array. */
        uint8_t _channel;

/** Length of the @sa allowed_ch array. */
#define HUBSAN_CHAN_ARR_LEN 12

        /** Available channel IDs for the Hubsan. */
        const static uint8_t allowed_ch[HUBSAN_CHAN_ARR_LEN];

        /** Session ID for this tranmission session. Randomly generated. */
        uint32_t sessionid;

        /**
         * The TX ID. Potentially not needed
         * but currently being set at @sa packet[11]
         * in reference code.
         */
        const static uint32_t txid = 0xdb042679;

        /** Transmit packet used for binding. */
        uint8_t _txpacket[16];

        /** Receive packet used for binding. */
        uint8_t _rxpacket[16];

};
#endif /* HUBSAN_H */
