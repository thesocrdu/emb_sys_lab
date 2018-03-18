/**
 * @file
 * @brief This file is used for interfacing with the BlueSMiRF Silver
 *
 * @author Kyle Mercer
 *
 */

#ifndef BT_SMIRF_H
#define BT_SMIRF_H

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <stdint.h>
#include <Stream.h>

/** Buffer size for module response over serial. */
#define BT_SMIRF_RESP_BUF_LEN 64

/**
 * This class abstracts calls using serial libraries
 * to interface with the BlueSMiRF Silver module.
 */
class bt_smirf {
    public:
        
        /**
         * Default constructor assumes Serial.
         */
        bt_smirf();

        /**
         * User provides the HardwareSerial interface.
         * @param h The HardwareSerial interface
         */
        bt_smirf(HardwareSerial &h);

        /**
         * User provides the SoftwareSerial interface.
         * @param s The SoftwareSerial interface
         */
        bt_smirf(SoftwareSerial &s);

        /**
         * Using this constructor creates a
         * SoftwareSerial object internally.
         *
         * @param rx The RX pin from the MCU's perspective.
         * @param tx The TX pin from the MCU's perspective.
         */
         bt_smirf(const uint8_t rx, const uint8_t tx);
        
        /** Destructor. */
        ~bt_smirf();

        /**
         * Initializes the serial communication
         * interface with the module. The module
		 * will remain in command mode after this
		 * function has completed.
         *
         * @param baud_rate
         *     The desired bit rate for communication.
         *     Supported rates include: 
         *     1200, 2400, 9600, 19200, 28800, 38400,
         *     57600, 115200, 230400, 460800, and 921600
         */
        void begin(const long baud_rate);

        /**
         * Instructs the module to enter CMD mode.
         * @retval 0 If the device was successfully
         *           put into command mode.
         * @retval -1 If device was not successfully
         *            put into command mode.
         */
        int enterCmdMode();

        /**
         * Instructs the module to exit CMD mode.
         * @retval 0 If the device successfully
         *           exited command mode.
         * @retval -1 If device did not successfully
         *            exit from command mode.
         */
        int exitCmdMode();

        /**
         * Sends a "\r\n" to the module to clear its
         * command queue. Useful for clearing partial
         * commands after an MCU soft reset.
         */
        void resetCmdQueue();

        /**
         * Flushes all data in the recieve buffer
         * of the underlying serial interface.
         * @return a pointer to the content that eas flushed.
         */
        const char* flushRxBuffer();

        /**
         * Gets the current mode of the module.
         * @return the module's @see bt_smirf_mode
         */
        //bt_smirf_mode getCurrMode();

    private:

        /**
         * @note The reason both a HardwareSerial and a SoftwareSerial
         * interface exist in this class is because both require
         * the begin() call which is not a virtual function in
         * the Stream class. Thus, polymorphism cannot be used.
         * Instead, the begin() function in this class uses the
         * specific interface to call its respective begin() and
         * the generic Stream object member is used for everything
         * else.
         */
        Stream *_serial_if; /**< Underlying generic serial interface. */
        HardwareSerial *_hw_serial; /**< HardwareSerial interface. */
        SoftwareSerial *_sw_serial; /**< SoftwareSerial interface. */
        bool _internal_serial; /**< Flag to know if we need to free serial if. */

        /**
         * States of the Bluetooth module.
         */
        enum bt_smirf_state {
            IDLE     = 0, /**< Not in discovery or binded to a device. */
            DISC     = 1, /**< (Default) In discovery mode as a master device. */
            PAIRING  = 2, /**< Actively pairing with a device. */
            PAIRED   = 3  /**< Paired and talking to a device. */
        };

        bt_smirf_state _curr_state; /**< Current state of this module. */

        /**
         * Modes of the Bluetooth module.
         */
        enum bt_smirf_mode {
            CMD_MODE  = 0, /**< Command mode. */
            DATA_MODE = 1 /**< Data mode. */
        };

        bt_smirf_mode _curr_mode; /**< Current mode of this module. */

        /**
         * Populates the @see _cmdResp buffer with response message.
         * @return The response buffer which is
         *         @see BT_SMIRF_RESP_BUF_LEN in length
         *         and NULL terminated.
         */
        const char* getResponse();

        /**
         * Writes the provided command over serial.
         * @param cmd The character string of the cmd.
         * @param appendNewLine If true, appends "\r\n" to cmd.
         * @return The response buffer which is
         *         @see BT_SMIRF_RESP_BUF_LEN in length
         *         and NULL terminated.
         */
        const char* sendCmd(const char *cmd, bool appendNewline = true);

        /** Buffer storing latest command response. */
        char _cmdResp[BT_SMIRF_RESP_BUF_LEN];

};

#endif /* BT_SMIRF_H */
