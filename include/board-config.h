#pragma once

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */

#define BOARD_TCXO_WAKEUP_TIME                      0
#define BOARD_READY_AFTER_POR						10000


/*!
 * Board MCU pins definitions - NodeMCUv2
 */
#define RADIO_RESET                                 4   // NodeMCU D2

#define RADIO_MOSI                                  13  // NodeMCU D7
#define RADIO_MISO                                  12  // NodeMCU D6
#define RADIO_SCLK                                  14  // NodeMCU D5
#define RADIO_NSS                                   15  // NodeMCU D8

#define RADIO_DIO_1                                 2   // NodeMCU D4
#define RADIO_DIO_0                                 5   // NodeMCU D1
#define RADIO_DIO_4                                 2   // NodeMCU D4
#define RADIO_DATA_AVAIL                            RADIO_DIO_1     // FIFO empty from Radio
#define RADIO_PACKET_AVAIL                          RADIO_DIO_0     // Packet Received / CRC ok from Radio
#define RADIO_PREAMBLE_DETECTED                     RADIO_DIO_4     // Preamble detected from Radio (used instead of FIFO empty)


#define PREAMBLE_MSB                                0x00
#define PREAMBLE_LSB                                0x34    // 0x34: 12ms to have receiver up and running (52 0x55 bytes - 13,54mS)
#define SYNC_BYTE_1                                 0xff
#define SYNC_BYTE_2                                 0x33    // Sync word - Size must be set to 2; fist byte 0xff then 0x33 size-1 times

#define MAX_FREQS                                   3       // Number of Frequencies to scan through Fast Hopping
#define FREQS2SCAN                                  {868250000, 868950000, 869850000}

#define SCAN_LED                                    D0
#define RX_LED                                      LED_BUILTIN