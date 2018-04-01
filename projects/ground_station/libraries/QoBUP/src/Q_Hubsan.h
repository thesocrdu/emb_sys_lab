/**
 * @file
 * @brief This file outlines the class structure
 * for the Hubsan Quadcopter functions specific to
 * the QoBUP interface.
 *
 * @author Kyle Mercer
 *
 */

#ifndef Q_HUBSAN_H
#define Q_HUBSAN_H

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#include "QoBUP.h"
#include <stdint.h>

/**
 * Hubsan flag field sent every control packet transmission.
 * At this time, only bit 2 is known to control the LEDs of
 * the Hubsan (1 for on, 0 for off). Otherwise, this 8-bit
 * struct should always either be set to 0x0E or 0x0A.
 */
struct q_hubsan_flight_control_flags_t {
    union {
        uint8_t word;
        struct {
            uint8_t unknown0 : 1;
            uint8_t unknown1 : 1;
            uint8_t ledOn    : 1;
            uint8_t unknown3 : 1;
            uint8_t unknown4 : 1;
            uint8_t unknown5 : 1;
            uint8_t unknown6 : 1;
            uint8_t unknown7 : 1;
        };
    };
};

/**
 * Flight control structure. Laid out in Hubsan format.
 * @note all values should be stored in format expected
 *       by the Hubsan quadcopter.
 */
struct q_hubsan_flight_controls_t {
    uint8_t header;       /**< Always set to 0x20. */
    uint8_t res1;         /**< Reserved region. Set to 0x0. */
    uint8_t throttle;     /**< Throttle value ranges from 0x0 to 0xff. */
    uint8_t res2;         /**< Reserved region. Set to 0x0. */
    uint8_t yaw;          /**< Yaw value centered at 0x80. Range from 0x0 to 0xff. */
    uint8_t res3;         /**< Reserved region. Set to 0x0. */
    uint8_t pitch;        /**< Pitch value centered at 0x80. Range from 0x0 to 0xff. */
    uint8_t res4;         /**< Reserved region. Set to 0x0. */
    uint8_t roll;         /**< Roll value centered at 0x80. Range from 0x0 to 0xff. */

    /**See @sa q_hubsan_flight_control_flags_t for definition. */
    q_hubsan_flight_control_flags_t flags;

    uint8_t setTo0x19;    /**< Always set to 0x19. */
    uint8_t res5;         /**< Reserved region. Set to 0x0. */
    uint8_t res6;         /**< Reserved region. Set to 0x0. */
    uint8_t res7;         /**< Reserved region. Set to 0x0. */
    uint8_t res8;         /**< Reserved region. Set to 0x0. */
    uint8_t crc;          /**< CRC checksum of the message. */
};

/**
 * This sub-class provides device-specific data handling
 * for the Hubsan H107L. Functions include:
 *
 * - Parsing QoBUP messages
 * - Translate QoBUP data into Hubsan flight controls
 * - Populating flight control into data structures
 * - Offering flight control data to the @Hubsan inferface
 */
class Q_Hubsan : public QoBUP {

    public:

        /** Constructor. */
        Q_Hubsan();

        /** Destructor. */
        ~Q_Hubsan();

        /**
         * Traverses through the received (and already validated)
         * message and populates the common data structures for
         * command/control values which are stored in this class.
         * @retval 0 On success.
         * @retval -1 If validations was not performed on this message.
         */
        int parseMessage(const uint8_t* const cmd);

    private:

        /** Hold the current flight controls. */
        q_hubsan_flight_controls_t _currFlightCntls;

        /**
         * Translates and populates the throttle.
         * @param[in] thStruct pointer to the @sa q_single_flight_control_block_t
         */
        void translateThrottle(
                const q_single_flight_control_block_t* const thStruct);

        /**
         * Translates and populates the yaw.
         * @param[in] yawStruct pointer to the @sa q_single_flight_control_block_t
         */
        void translateYaw(
                const q_single_flight_control_block_t* const yawStruct);

        /**
         * Translates and populates the pitch.
         * @param[in] pitchStruct pointer to the @sa q_single_flight_control_block_t
         */
        void translatePitch(
                const q_single_flight_control_block_t* const pitchStruct);

        /**
         * Translates and populates the roll.
         * @param[in] rollStruct pointer to the @sa q_single_flight_control_block_t
         */
        void translateRoll(
                const q_single_flight_control_block_t* const rollStruct);

        /**
         * Translates and populates all flgiht movement controls.
         * @param[in] flightStruct pointer to the @sa q_all_flight_control_block_t
         */
        void translateAllFlightControls(
                const q_all_flight_control_block_t* const flightStruct);

};
#endif /* Q_HUBSAN_H */
