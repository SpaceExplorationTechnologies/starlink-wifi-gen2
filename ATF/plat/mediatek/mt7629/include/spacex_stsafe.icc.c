#ifndef __SPACEX_STSAFE_ICC_C_
#define __SPACEX_STSAFE_ICC_C_

/*
 * SpaceX library to interface with the STSAFE mounted on the i2c bus of the
 * Starlink V2 Router.
 * 
 * The STSAFE, besides providing the cert and signing capabilities that
 * authenticates our device as a genuine SpaceX device, also has extra zones. We
 * have repurposed zone 6 to store a 32bit boot counter. This counter is used to
 * determine whether to boot set 0 or 1 of our FIPs and Kernels. This builds in
 * redundancy in the event one set is inoperable. The parity of the counter is
 * used to determine the side. The boot counter is also communicated to the next
 * boot loader (U-boot) by storing it in the TRANSFER_LEN register of the i2c
 * MMIO (this store now happens right after select_boot_partition() returns).
 * This was the easiest way to pass the parameter with minor modifications to
 * how ATF crawls the bootloader chain.
 * 
 * Zones 7 and 8 of the STSAFE contain built-in decrementing counters. These
 * counters are used for anti-rollback functionality (the rest of the system
 * treats the anti-rollback counter as incrementing, so the revoked version is
 * "flipped" by subtracting itself from 500,000, the initial value of the
 * STSAFE's decrementing counters).
 *
 * Note: Wear leveling was not implemented for this counter, so the counter may
 * not function when counting over 500,000.
*/

#include <stdint.h>
#include <stddef.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>

#define DEBUG_SPACEX false

// I2C Address the STSAFE chip is on.
#define STSAFE_ADDR 0x20

// MMIO Register addrs
#define GPIO_DIR_1              0x10217000
#define GPIO_DOUT_1             0x10217100
#define GPIO_MODE_2             0x10217320
#define GPIO_MODE_3             0x10217324
#define DATA_PORT               0x11007000
#define SLAVE_ADDR              0x11007004
#define INTR_MASK               0x11007008
#define INTR_STAT               0x1100700C
#define CONTROL                 0x11007010
#define TRANSFER_LEN            0x11007014
#define TRANSAC_LEN             0x11007018
#define DELAY_LEN               0x1100701C
#define TIMING                  0x11007020
#define I2C_START               0x11007024
#define EXT_CONF                0x11007028
#define FIFO_STAT1              0x1100702C
#define FIFO_STAT               0x11007030
#define FIFO_THRESH             0x11007034
#define FIFO_ADDR_CLR           0x11007038
#define IO_CONFIG               0x11007040
#define I2C_DEBUG               0x11007044
#define I2C_HS                  0x11007048
#define SOFT_RESET              0x11007050
#define DCM_EN                  0x11007054
#define DEBUGSTAT               0x11007064
#define DEBUGCTRL               0x11007068
#define TRANSFER_LEN_AUX        0x1100706C
#define CLOCK_DIV               0x11007070
#define SCL_HIGH_LOW_RATIO      0x11007074
#define HS_SCL_HIGH_LOW_RATIO   0x11007078
#define SCL_MIS_COMP_POINT      0x1100707C
#define STA_STOP_AC_TIMING      0x11007080
#define HS_STA_STOP_AC_TIMING   0x11007084
#define SDA_TIMING              0x11007088
#define FIFO_PAUSE              0x1100708C

// Settings
#define GPIO_EN_I2C             0x00011000
#define STSAFE_BOOTCOUNTER_ZONE        0x6
#define STSAFE_ANTIROLLBACK_ZONE_0     0x7
#define STSAFE_ANTIROLLBACK_ZONE_1     0x8
#define STSAFE_DECREMENT_CMD           0x4
#define STSAFE_READ_CMD                0x5
#define STSAFE_UPDATE_CMD              0x6
#define STSAFE_DECREMENT_CMD_LEN        11
#define STSAFE_READ_CMD_LEN              9
#define STSAFE_UPDATE_CMD_LEN           11
#define STSAFE_RESP_MAX_LEN             16
#define WRITE_I2C                    false
#define READ_I2C                      true

// Anti-rollback value. If the currently-booting side's STSAFE anti-rollback counter
// indicates a required version greater than this value, this BL2 can't boot.
//
// Note that the required version is equal to 500,000 minus the actual value in the
// STSAFE because the STSAFE counters only decrement.
//
// IF YOU ARE CHANGING THIS NUMBER, PROCEED WITH EXTREME CAUTION.
// IF THE STSAFE VALUE BEING SET BY WIFI_CONTROL DOES NOT CORRESPOND
// TO THIS VALUE, YOU COULD PERMANENTLY BRICK ANY BOARD THIS GETS ON.
//
// See https://confluence.spacex.corp/display/STARPROD/Starlink+UX+-+V2+Anti-Rollback
#define BL2_ANTIROLLBACK_VALUE        9999

// The minimum required version from the STSAFE (equal to the STSAFE counter value
// minus 500,000), mentioned in the comment above. Declared in spacex_rollback_version.h
uint32_t required_rollback_version;

uint32_t stsafe_bootcount;

uint16_t crc16Table[256] = {
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
	0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
	0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
	0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
	0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
	0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
	0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
	0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
	0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
	0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
	0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
	0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
	0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
	0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
	0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
	0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
	0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
	0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
	0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
	0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
	0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
	0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
	0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
	0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
	0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
	0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
	0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
	0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
	0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
	0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
	0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

// Calulate a CRC16 over a buffer of data and return it.
uint16_t crc16(uint8_t* buf, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        uint16_t b = buf[i];
        uint16_t ndx = 0xff & (crc ^ b);
        crc = crc16Table[ndx] ^ (crc >> 8);
    }
    return ~crc;
}

// Given a buffer of data, calculate CRC over 0:len-2, then
// return whether it equals the last two bytes, in big endian.
bool valid_resp_crc(uint8_t* buf, uint16_t len) {
    if (len <= 2) {
        ERROR("valid_resp_crc: buffer too short\n");
        return false;
    }
    // Exclude last 2 bytes, which contain CRC.
    uint16_t crc = crc16(buf, len - 2);
    return (buf[len - 2] == (crc >> 8 & 0xFF)) && (buf[len - 1] == (crc & 0xFF));
}

// Given a buffer of data, calculate the crc over 0:len-2, then
// set crc as the two ending bytes, in big endian.
void populate_crc16(uint8_t* buf, uint16_t len) {
    if (len <= 2) {
        ERROR("command too short to populate CRC\n");
        return;
    }

    // Populate big endian CRC.
    uint16_t crc = crc16(buf, len - 2);
    buf[len - 2] = crc >> 8;
    buf[len - 1] = crc;
}

// Configure the MT7629 SoC i2c subsystem so that it is able to communicate with
// devices on the i2c bus at 100kHz.
void mt7629_i2c_init_hw() {
    // Enable i2c GPIO pins 19 and 20.
    mmio_write_32(GPIO_MODE_2, GPIO_EN_I2C);
    // Soft reset
    mmio_write_16(SOFT_RESET, 0x1);
    // Set open drain in io config
    mmio_write_16(IO_CONFIG, 0x3);
    // Disable DCM
    mmio_write_16(DCM_EN, 0x0);
    // Set clk div to 1
    mmio_write_16(CLOCK_DIV, 0x1);
    // Set timing to x422, derived from OpenWRT mt65xx driver
    mmio_write_16(TIMING, 0x422);
    // Disable HS mode
    mmio_write_16(I2C_HS, 0x0);
    // Set control reg 28
    mmio_write_16(CONTROL, 0x28);
    // set delay len to 2
    mmio_write_16(DELAY_LEN, 0x2);
    // Set our interrupt mask.
    mmio_write_16(INTR_MASK, 0x0001);
    // Clear interrupts
    mmio_write_16(INTR_STAT, 0x0000);
    // wait 50 usec
    udelay(50);
    // set start condition
    mmio_write_16(EXT_CONF, 0x8001);
    // Give hardware time to settle
    mdelay(10);
}

// Revert settings before handoff to higher levels.
void mt7629_i2c_close_hw() {
    // Clear the FIFO
    mmio_write_8(FIFO_ADDR_CLR, 1);
    // Clear interrupt mask.
    mmio_write_16(INTR_MASK, 0x0000);
    // Clear interrupts
    mmio_write_16(INTR_STAT, 0x0000);
}

// Does an i2c read/write transaction, for writes, data should be loaded into
// the data array, for reads, bytes are written into the data array, not more
// than buffer_len bytes total. The actual number of bytes written is put in
// read_len. Using the built in FIFO queue, we are SUPPOSED to not be able to
// read more than 16 bytes at a time. More than 16 bytes at a time has worked
// before, so increase if needed but at your own risk.
// 
// Re: calling this function, it is up to the caller to determine the number of
// bytes that should be read.
int mt7629_i2c_transaction(uint8_t slave_addr, bool read, uint8_t* data, uint16_t buffer_len, uint16_t* read_len) {
    if (read && (buffer_len < 3 || buffer_len > 16)) {
        ERROR("I2C Transaction buffer must have 3 <= length <= 16 for reads\n");
        return -1;
    }
    if (!read && buffer_len > 16) {
        ERROR("I2C Transaction buffer must have length <= 16 for writes\n");
        return -1;
    }

    // Clear the FIFO
    mmio_write_8(FIFO_ADDR_CLR, 1);

    // Clear interrupts
    mmio_write_16(INTR_STAT, 0x0000);

    // Set address to send transaction to. 7:1 is address, bit 0 is 1 for reads.
    mmio_write_8(SLAVE_ADDR, (slave_addr << 1) | read);

    // If we are going to write we need to load the FIFO queue.
    if (!read) {
        for (uint16_t i = 0; i < buffer_len; i++) {
            mmio_write_8(DATA_PORT, data[i]);
            if (DEBUG_SPACEX) {
                INFO("WRITE Byte: %x\n", data[i]);
            }
        }
    }
    // Set the transfer len register
    mmio_write_16(TRANSFER_LEN, buffer_len);

    // Start the transaction.
    mmio_write_16(I2C_START, 1);

    // Watch for the transaction or complete, or timeout.
    int tick = 10;
    while((mmio_read_16(INTR_STAT) & 0x1) != 1) {
        mdelay(1);
        if (tick <= 0) {
            ERROR("I2C Transaction timed out\n");
            return -1;
        }
        --tick;
    }

    // If we are reading, transfer bytes to the data buffer
    if (read) {
        mdelay(1);
        for (uint16_t i = 0; i < 3; i++) {
            data[i] = mmio_read_8(DATA_PORT);
            if (DEBUG_SPACEX) {
                INFO("READ Byte: %x\n", data[i]);
            }
        }
        *read_len = (((uint16_t)data[1] << 8) | data[2]) + 3;
        if (*read_len > buffer_len) {
            ERROR("data[1,2] indicate length longer than buffer (maybe garbage data?)\n");
            return -1;
        }
        for (uint16_t i = 3; i < *read_len; i++) {
            data[i] = mmio_read_8(DATA_PORT);
            if (DEBUG_SPACEX) {
                INFO("READ Byte: %x\n", data[i]);
            }
        }
    }
    return 0;
}

// Given a response, check that the response code is zero, and
// that the CRC is correct. read_len is the number of bytes that
// were read in the entire I2C response.
bool stsafe_is_valid_response(uint8_t* data, uint16_t read_len) {
    if (read_len < 3 || read_len > 16) {
        ERROR("STSAFE response validator assumes 3 <= len <= 16\n");
        return false;
    }

    // Get response code but don't check it until after CRC
    // is confirmed to be valid
    uint8_t resp_code = data[0];

    uint16_t data_len = read_len - 3; // First 3 bytes are metadata
    
    // Populate CRC check buffer, removing len (1:2) variable.
    uint8_t crcBuf[16] = {resp_code};
    for (int i = 0; i < data_len; i++) {
        crcBuf[i + 1] = data[i + 3];
    }

    if (!valid_resp_crc(crcBuf, data_len + 1)) {
        ERROR("Invalid CRC\n");
        return false;
    }
    if (resp_code) {
        ERROR("Response code: %x\n", resp_code);
        return false;
    }

    return true;
}

// For a given zone, use the first 4 bytes to implement an incrementing counter. The
// eeprom of the stsafe is only good for 500k writes per byte, so if you wish for a
// higher counter you will need to wear level, which this does not. Set the parameter
// counter to the updated counter value. Returns 0 on success.
int stsafe_inc_counter(uint8_t zone, uint32_t *counter) {
    uint16_t offset = 0;
    uint16_t length = 4;
    uint8_t readCmd[STSAFE_READ_CMD_LEN] = {
        STSAFE_READ_CMD, 0x00, zone,
        offset >> 8, offset,
        length >> 8, length
    };
    populate_crc16(readCmd, STSAFE_READ_CMD_LEN);

    // Read STSAFE for the first 4 bytes of the zone.
    int ret = mt7629_i2c_transaction(STSAFE_ADDR, WRITE_I2C, readCmd, STSAFE_READ_CMD_LEN, NULL);
    if (ret) {
        ERROR("Failed to send read zone %d command\n", zone);
        return -1;
    }
    mdelay(10);

    uint16_t readLen;

    // Create a buffer to read into.
    uint8_t buf[STSAFE_RESP_MAX_LEN] = {};
    ret = mt7629_i2c_transaction(STSAFE_ADDR, READ_I2C, buf, STSAFE_RESP_MAX_LEN, &readLen);
    if (ret) {
        ERROR("Failed to get read zone %d response\n", zone);
        return -1;
    }

    if (!stsafe_is_valid_response(buf, readLen)) {
        ERROR("Invalid read zone %d response\n", zone);
        return -1;
    }
    mdelay(10);

    // Increment and set counter return value.
    *counter = (
        ((uint32_t)buf[3] << 24) |
        ((uint32_t)buf[4] << 16) |
        ((uint32_t)buf[5] <<  8) |
        (uint32_t)buf[6]
    ) + 1;

    // Update the counter data. Only use the first 4 bytes of the zone, big endian style.
    offset = 0;
    uint8_t options = 0x80; // Set atomic update.
    uint8_t updateCmd[STSAFE_UPDATE_CMD_LEN] = {
        STSAFE_UPDATE_CMD, options, zone,
        offset >> 8, offset,
        *counter >> 24, *counter >> 16, *counter >> 8, *counter,
    };
    populate_crc16(updateCmd, STSAFE_UPDATE_CMD_LEN);

    ret = mt7629_i2c_transaction(STSAFE_ADDR, WRITE_I2C, updateCmd, STSAFE_UPDATE_CMD_LEN, NULL);
    if (ret) {
        ERROR("Failed to send update zone %d command\n", zone);
        return -1;
    }
    // 10ms is not enough time for STSAFE to confirm update on first write after power cycle,
    // but 20ms seems to be enough.
    mdelay(20);

    // Create a buffer to read into.
    uint8_t updateResp[STSAFE_RESP_MAX_LEN] = {};

    const int MAX_UPDATE_RESP_ATTEMPTS = 5;
    bool gotValidResponse = false;
    for (int attempts = 0; attempts < MAX_UPDATE_RESP_ATTEMPTS; ++attempts) {
        ret = mt7629_i2c_transaction(STSAFE_ADDR, READ_I2C, updateResp, STSAFE_RESP_MAX_LEN, &readLen);
        if (ret) {
            ERROR("Failed to get update zone %d response\n", zone);
            return -1;
        }

        if (stsafe_is_valid_response(updateResp, readLen)) {
            gotValidResponse = true;
            break;
        }

        // 10ms is not enough time for STSAFE to confirm update on first write after power cycle,
        // but 20ms seems to be enough.
        mdelay(20);

        if (DEBUG_SPACEX) {
            INFO("Counter Update zone %d, command response read attempt %d invalid\n", zone, attempts);
        }
    }

    if (!gotValidResponse) {
        ERROR("Invalid STSAFE update zone %d response after multiple attempts\n", zone);
        return -1;
    }

    return 0;
}

// Read the one-way counter in an STSAFE zone and write it to *counter.
// Returns -1 on failure, 0 on success.
int stsafe_read_oneway_counter(uint8_t zone, uint32_t *counter) {
    // Initialize counter so it has a deterministic value on error.
    *counter = 0;

    uint16_t offset = 0;
    uint16_t length = 0; // We only want the one-way counter, which isn't included in length
    uint8_t readCmd[STSAFE_READ_CMD_LEN] = {
        STSAFE_READ_CMD, 0x00, zone,
        offset >> 8, offset,
        length >> 8, length
    };
    populate_crc16(readCmd, STSAFE_READ_CMD_LEN);

    // Read STSAFE for the one-way counter of the zone.
    int ret = mt7629_i2c_transaction(STSAFE_ADDR, WRITE_I2C, readCmd, STSAFE_READ_CMD_LEN, NULL);
    if (ret) {
        ERROR("Failed to send read zone %d command\n", zone);
        return -1;
    }
    mdelay(10);

    // Create a buffer to read into.
    uint8_t buf[STSAFE_RESP_MAX_LEN] = {};
    uint16_t readLen;
    ret = mt7629_i2c_transaction(STSAFE_ADDR, READ_I2C, buf, STSAFE_RESP_MAX_LEN, &readLen);
    if (ret) {
        ERROR("Failed to get read zone %d response\n", zone);
        return -1;
    }

    if (!stsafe_is_valid_response(buf, readLen)) {
        ERROR("Invalid read zone %d response\n", zone);
        return -1;
    }
    mdelay(10);

    // Set counter return value.
    *counter = (
        ((uint32_t)buf[3] << 24) |
        ((uint32_t)buf[4] << 16) |
        ((uint32_t)buf[5] <<  8) |
        (uint32_t)buf[6]
    );

    return 0;
}

// Decrement the one-way counter in an STSAFE zone by an amount and write the returned
// "decremented" counter value into decremented. The STSAFE will not accept 0 as an amount.
// Returns -1 on failure, 0 on success.
int stsafe_decrement_oneway_counter(uint8_t zone, uint32_t amount, uint32_t* decremented) {
    if (amount == 0) {
        ERROR("STSAFE counter can't be decremented by zero\n");
        return -1;
    }

    uint16_t offset = 0; // 0 offset, built-in counter
    uint8_t decrementCmd[STSAFE_DECREMENT_CMD_LEN] = {
        STSAFE_DECREMENT_CMD, 0x00, zone,
        offset >> 8, offset,
        amount >> 24, amount >> 16, amount >> 8, amount
    };
    populate_crc16(decrementCmd, STSAFE_DECREMENT_CMD_LEN);

    // Decrement STSAFE one-way counter.
    int ret = mt7629_i2c_transaction(STSAFE_ADDR, WRITE_I2C, decrementCmd, STSAFE_DECREMENT_CMD_LEN, NULL);
    if (ret) {
        ERROR("Failed to send decrement zone %d command\n", zone);
        return -1;
    }
    mdelay(10);

    // Create a buffer to read into.
    uint8_t buf[STSAFE_RESP_MAX_LEN] = {};
    uint16_t readLen;
    ret = mt7629_i2c_transaction(STSAFE_ADDR, READ_I2C, buf, STSAFE_RESP_MAX_LEN, &readLen);
    if (ret) {
        ERROR("Failed to get decrement zone %d response\n", zone);
        return -1;
    }

    if (!stsafe_is_valid_response(buf, readLen)) {
        ERROR("Invalid decrement zone %d response\n", zone);
        return -1;
    }
    mdelay(10);

    // Set decremented counter return value.
    *decremented = (
        ((uint32_t)buf[3] << 24) |
        ((uint32_t)buf[4] << 16) |
        ((uint32_t)buf[5] <<  8) |
        (uint32_t)buf[6]
    );

    return 0;
}

// In the event of an error, use GPIO 25 to reset the STSAFE by driving it
// low then back high again.
void gpio_reset_stsafe() {
    // Set GPIO25 mode to gpio, mask out 25 settings. Setting the 2nd octet to 0
    // puts GPIO25 into gpio mode.
    mmio_write_32(GPIO_MODE_3, mmio_read_32(GPIO_MODE_3) & 0xFFFFFF0F);

    // Set GPIO25 to output mode, bit 25 = 1 is output.
    mmio_write_32(GPIO_DIR_1, mmio_read_32(GPIO_DIR_1) | 0x02000000);

    // Set GPIO25 low, bit 25 = 0 drives the output low.
    mmio_write_32(GPIO_DOUT_1, mmio_read_32(GPIO_DOUT_1) & 0xFDFFFFFF);

    mdelay(1);

    // Set GPIO25 high
    mmio_write_32(GPIO_DOUT_1, mmio_read_32(GPIO_DOUT_1) | 0x02000000);
}

void reboot_via_watchdog() {
    // Set watchdog timer to 15.6ms
    mmio_write_16(0x10212004, 0x0028);
    // Reset watchdog timer, and wait for reset.
    mmio_write_16(0x10212008, 0x1971);
    // Wait for death.
    while(true){}
}

// Queries the STSAFE chip, sets the stsafe_bootcount global.
void select_boot_partition() {
    // Initialize hardware.
    mt7629_i2c_init_hw();

    if (stsafe_inc_counter(STSAFE_BOOTCOUNTER_ZONE, &stsafe_bootcount)) {
        // We've had an error, reset STSAFE and let the watchdog reset us.
        gpio_reset_stsafe();
        reboot_via_watchdog();
    }
    INFO("Got Boot Count: %d\n", stsafe_bootcount);

    mt7629_i2c_close_hw();
}

// Return if BL2 has a valid anti-rollback counter relative to the STSAFE counter for
// the currently-booting side; otherwise, reboot.
void abort_boot_if_bl2_revoked() {
    int antirollbackZone = (stsafe_bootcount & 0x1) ? STSAFE_ANTIROLLBACK_ZONE_1 : STSAFE_ANTIROLLBACK_ZONE_0;
    uint32_t stsafeAntirbCount;

    // Check the STSAFE anti-rollback counter corresponding to the currently-booting side
    // of the board. If I2C fails in some way, reboot to try again. If the STSAFE counter
    // indicates a required version greater than this BL2's rollback value, send this
    // board into an infinite boot loop / cast it to the shadow realm. This should only
    // occur if someone goes onto a board and flashes an old, revoked BL2.
    if (stsafe_read_oneway_counter(antirollbackZone, &stsafeAntirbCount)) {
        ERROR("STSAFE zone %d anti-rollback counter read failed. Rebooting.\n", antirollbackZone);
        reboot_via_watchdog();
    }
    required_rollback_version = 500000 - stsafeAntirbCount;
    if (BL2_ANTIROLLBACK_VALUE < required_rollback_version) {
        ERROR("BL2 anti-rollback value (%d) < required version (%d), revoked. Rebooting.\n",
            BL2_ANTIROLLBACK_VALUE, required_rollback_version);
        reboot_via_watchdog();
    }

    INFO("BL2 anti-rollback value (%d) >= required version (%d), valid. Continuing boot.\n",
        BL2_ANTIROLLBACK_VALUE, required_rollback_version);
}

// Update the anti-rollback count of the STSAFE zone corresponding to the active side
// of the board to reflect a desired rollback version. Return -1 on error, 0 on success.
int update_anti_rollback(uint32_t current_stsafe_ver, uint32_t desired_stsafe_ver) {
    // Don't allow an update to brick the BL2 it's being loaded on.
    // If a BL2 revocation (or, somehow, 10,000 FIP revocations) occurs,
    // BL2_ANTIROLLBACK_VALUE should be incremented by 10,000 in the same upgrade that
    // "performs the revocation".
    if (desired_stsafe_ver > BL2_ANTIROLLBACK_VALUE) {
        ERROR("update_anti_rollback: desired STSAFE rollback version (%d) > BL2 rollback version (%d), illegal\n",
            desired_stsafe_ver, BL2_ANTIROLLBACK_VALUE);
        return -1;
    }

    uint8_t active_side = stsafe_bootcount & 0x1;
    uint8_t zone = active_side ? STSAFE_ANTIROLLBACK_ZONE_1 : STSAFE_ANTIROLLBACK_ZONE_0;

    int32_t decrement_amount = desired_stsafe_ver - current_stsafe_ver;
    if (decrement_amount <= 0) {
        ERROR("update_anti_rollback: desired rollback version must be > current to update\n");
        return -1;
    }

    uint32_t final_stsafe_count;
    int rc = stsafe_decrement_oneway_counter(zone, (uint32_t)decrement_amount, &final_stsafe_count);
    if (rc) {
        ERROR("update_anti_rollback: stsafe_decrement_oneway_counter failed");
        return -1;
    }

    if (500000 - final_stsafe_count != desired_stsafe_ver) {
        ERROR("update_anti_rollback: final STSAFE version != desired\n");
        return -1;
    }

    return 0;
}


#endif // __SPACEX_STSAFE_ICC_C_
