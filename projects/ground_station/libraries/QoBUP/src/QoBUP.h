/**
 * @file
 * @brief This file outlines the class structure
 * for the QoBUP interface.
 *
 * @author Kyle Mercer
 *
 */

#ifndef QOBUP_H
#define QOBUP_H

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

/**
 * This class provides interfaces for handling, validating,
 * parsing, and processing of the Quad over Bluetooth
 * Universal Protocol (QoBUP) standard.
 *
 * See the QoBUP ICD for any details, features, and limitations
 * pertaining to the protocol.
 */

#include <stdint.h>
#include <Stream.h>

/** Max possible size of a command message, in bytes. */
#define Q_MAX_SIZE_CMD_BYTES 32

/** Timeout (in milliseconds) for receiving a command over serial. */
#define Q_SERIAL_TIMEOUT_MS 10

/**
 * @defgroup QoBUP message constants.
 * @{
 */

#define Q_MSG_ID_CONTROL           0xAA
#define Q_BLOCK_ID_THROTTLE        0x00
#define Q_BLOCK_ID_YAW             0x01
#define Q_BLOCK_ID_PITCH           0x02
#define Q_BLOCK_ID_ROLL            0x03
#define Q_BLOCK_ID_FLIGHT_CONTROL  0x04
#define Q_BLOCK_ID_EOM             0xA5

/** @} */

/** Structure representing an 8-bit status message. */
struct q_status_t {
    union {
        uint8_t word;
        struct {
           uint8_t bad_msg_id   : 1; /**< Flag indicating an unknown message ID. */
           uint8_t bad_block_id : 1; /**< Flag indicating an unknown block ID. */
           uint8_t bad_size     : 1; /**< Flag indicating an error in message size. */
           uint8_t uninit       : 1; /**< Flag indicating we haven't yet stated validation. */
           uint8_t timeout      : 1; /**< Flag indicating a timeout occurred receiving command. */
           uint8_t reserved     : 3; /**< Reserved error bits. */
        };
    };
};

/** Structure representing a full status message */
struct q_status_msg_t {
    uint8_t sid;       /**< The echo'ed session ID from the command message.*/
    q_status_t status; /**< The @sa q_status_t status. */
};

/** Describes the header for any message. */
struct q_message_header_t {
    uint8_t id;  /**< The ID of the block. */
    uint8_t wc;  /**< The word count of the block including id and wc. */
    uint8_t sid; /**< The unique ID that is sent back as the status ID. */
};

/** Describes the header for any block. */
struct q_generic_block_t {
    uint8_t id; /**< The ID of the block. */
    uint8_t wc; /**< The word count of the block including id and wc. */
};

/** Describes the contents of a flight movement block. */
struct q_single_flight_control_block_t {
    uint8_t id;  /**< The ID of the movement type (ie. yaw/pitch/role). */
    uint8_t wc;  /**< The word count of the block including id and wc. */
    uint8_t val; /**< The value for flight control update. */
};

/** Describes the contents of a all flight movements within a block. */
struct q_all_flight_control_block_t {
    uint8_t id;       /**< The ID of the movement type (ie. yaw/pitch/role). */
    uint8_t wc;       /**< The word count of the block including id and wc. */
    uint8_t throttle; /**< The value for throttle. */
    uint8_t yaw;      /**< The value for yaw. */
    uint8_t pitch;    /**< The value for pitch. */
    uint8_t roll;     /**< The value for roll. */
};

class QoBUP {

    public:
        /**
         * Default constructor
         */
        QoBUP();

        /**
         * Destructor
         */
        ~QoBUP();

        /**
         * Gets the latest status msg.
         * @return The latest @sa q_status_msg_t.
         */
        q_status_msg_t getCurrStatus();

        /**
         * Validates the integrity of the overall message
         * provided as an input parameter.
         * @param cmd Pointer to the start of the command message.
         * @return The @sa q_status_msg_t for this validation.
         */
        q_status_msg_t validateMessage(const uint8_t* const cmd);

        /**
         * Attempts to get a command message over serial, populating
         * it into the provided (pre-allocated) command buffer.
         * The function also performs an overall size validation
         * and returns a representative @sa q_status_msg_t.
         * If validation fails the caller should assume the cmdBuff
         * is now corrupt and should ignore the data it contains.
         * This function blocks until serial data is available.
         *
         * @param s The Serial interface from which to pull the cmd data.
         * @param cmdBuff[in/out] The pre-allocated buffer.
         * @param size The number of elements allocated to cmdBuff.
         * @return The @sa q_status_msg_t for this validation.
         */
        q_status_msg_t serialRxMsg(Stream &s, uint8_t* const cmdBuff, uint8_t size);

    private:
        //init this to non-zero/add bit for startup init?
        q_status_msg_t _curr_status; /**< The current status of the latest cmd. */

        /**
         * Level one validation validates the size of
         * the incoming command.
         *
         * @param cmd Pointer to the start of the command message.
         * @return the @sa q_status for this validation.
         */
        q_status_t levelOneValidation(const uint8_t* const cmd);

        /**
         * Level two validation validates all of the
         * IDs in the message (include message and block
         * IDs) to ensure the command is ready to be processed.
         *
         * @param cmd Pointer to the start of the command message.
         * @return the @sa q_status for this validation.
         */
        q_status_t levelTwoValidation(const uint8_t* const cmd);
};

#endif /* QOBUP_H */
