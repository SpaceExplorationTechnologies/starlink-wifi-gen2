#include <stdint.h>
#include <stddef.h>
#include <linux/delay.h>

// Helpers.
#define REG(addr) (*(volatile uint32_t*)addr)

// MT7629 registers.
#define GPIO_DIR_1      (REG(0x10217000))
#define GPIO_DOUT_1     (REG(0x10217100))

/**
 * Starlink WiFi v2: Reset peripherals. Find a better home for this some day.
 */
void spacex_v2_reset(void)
{
    // Reset LAN PHY by setting GPIO28 to output mode, then toggling.
    GPIO_DIR_1 |= 0x10000000;
    mdelay(1);
    GPIO_DOUT_1 &= ~0x10000000;
    mdelay(1);
    GPIO_DOUT_1 |=  0x10000000;
}