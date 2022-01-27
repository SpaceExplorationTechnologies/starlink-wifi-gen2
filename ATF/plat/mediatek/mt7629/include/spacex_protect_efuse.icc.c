#ifndef __SPACEX_PROTECT_EFUSE_ICC_C_
#define __SPACEX_PROTECT_EFUSE_ICC_C_

#include <stdint.h>
#include <lib/mmio.h>

// APC helpers
#define	APC_BASE    0x10007000

#define	APC_CON     (APC_BASE + 0xF00)
#define	APC_LOCK    (APC_BASE + 0xF04)

#define	APC_D0_BASE (APC_BASE + 0x000)
#define	APC_D1_BASE (APC_BASE + 0x100)
#define	APC_D2_BASE (APC_BASE + 0x200)
#define	APC_D3_BASE (APC_BASE + 0x300)

// APC_CON fields
#define	APC_CON_SECURE_ONLY             (1 << 0)
#define	APC_CON_SECURE_SETTING_LOCKED   (1 << 1)

// Note: APC_D0 *must* be last here, otherwise we lock ourselves out while configuring the APCs (ATF will be in domain 0)
const uint32_t apcs[] = {APC_D3_BASE, APC_D2_BASE, APC_D1_BASE, APC_D0_BASE};

#define	APC_REGISTER(apc_base, slave_id)	(apc_base + ((slave_id*2)/32)*4)
#define	APC_VALUE(slave_id, apc_permission)	(apc_permission << ((slave_id*2)%32))

#define	APC_APC_ID      7
#define	EFUSE_APC_ID    19

// Device APC permissions
#define	APC_OPEN        0x0
#define	APC_SECURE_RW   0x1
#define	APC_S_RW_NS_RO  0x2
#define	APC_NO_ACCESS   0x3

// Perform the register write sequence to protect eFuse registers from being
// accessed by the non-secure world
void protect_efuse() {
    mmio_write_32(APC_CON, APC_CON_SECURE_ONLY | APC_CON_SECURE_SETTING_LOCKED);

    for (int i = 0; i < (sizeof(apcs) / sizeof(apcs[0])); i++) {
        uint32_t apc = apcs[i];

        // Fuses are secure RW only
        mmio_write_32(APC_REGISTER(apc, EFUSE_APC_ID), APC_VALUE(EFUSE_APC_ID, APC_SECURE_RW));

        // Prevent any further access to the APC
        mmio_write_32(APC_REGISTER(apc, APC_APC_ID), APC_VALUE(APC_APC_ID, APC_NO_ACCESS));
    }

    mmio_write_32(APC_LOCK, 0xF); // There are 4 domains, lock them all
}

#endif /* __SPACEX_PROTECT_EFUSE_ICC_C_ */