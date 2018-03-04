/**
 * @file
 * @brief This file is used for interfacing with the BlueSMiRF Silver
 *
 * @author Kyle Mercer
 *
 */

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
class QoBUP {

    public:
        /**
         * Default constructor
         */
        QoBUP();

        /**
         * Level one validation validates the size of
         * the incoming command.
         *
         * @param cmd Pointer to the start of the command message.
         * @return the @sa q_status for this validation.
         */
        q_status levelOneValidation(const uint8_t* const cmd);

        /**
         * Level two validation validates all of the
         * IDs in the message (include message and block
         * IDs) to ensure the command is ready to be processed.
         *
         * @param cmd Pointer to the start of the command message.
         * @return the @sa q_status for this validation.
         */
        q_status levelTwoValidation(const uint8_t* const cmd);

        /**
         * Destructor
         */
        ~QoBUP();

    private:
};
