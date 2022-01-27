#ifndef __SPACEX_PROTECT_TRUSTZONE_ICC_C_
#define __SPACEX_PROTECT_TRUSTZONE_ICC_C_

#include <stdint.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define	DRAM_BASE           0x40000000
#define	TZRAM_PROT_START    (TZRAM_BASE - DRAM_BASE)
#define	TZRAM_PROT_END      (TZRAM_PROT_START + TZRAM_SIZE)

#if (TZRAM_PROT_START & 0xFFFF != 0) || (TZRAM_PROT_END & 0xFFFF != 0)
	#error "TrustZone region is not 16-bit aligned, can not configure EMI"
#endif

// Permissions
#define	OPEN        0x0
#define	S_RW        0x1
#define	S_RW_NS_RO  0x2
#define	S_RW_NS_WO  0x3
#define	S_RO_NS_RO  0x4
#define	NO_ACCESS   0x5
#define	S_SO_NS_RW  0x6

// EMI registers
#define	EMI_MPUA    0x10203160
#define	EMI_MPUI    0x102031A0
#define	EMI_MPUM    0x102031C0
#define	EMI_MPUN    0x102031C8
#define	EMI_MPUO    0x102031D0

/********** EMI register fields **********/

// Fields for EMI_MPUA through EMI_MPUH
/*
 * Note that MTK implements this as inclusive on both start and end.
 * We treat end as exclusive to be more idiomatic, hence the -1 here.
 */
#define	EMI_REGION(start, end)	((start & 0xFFFF0000) | ((end-1) >> 16))

// Fields for EMI_MPUI through EMI_MPUL
#define	EMI_REGION0_LOCKED	(1 << 15)
#define	EMI_DOMAIN_PERMISSION(domain, permission)	(permission << (domain*3))

// Fields for EMI_MPUM through EMI_MPUO
#define	EMI_DOMAIN_LOCKED               ((uint32_t)1 << 31)
#define	EMI_DOMAIN_SECURE_ONLY          ((uint32_t)1 << 30)
#define	EMI_DOMAIN_SLVERR_ON_VIOLATION  ((uint32_t)1 << 28)

// Perform the register write sequence to protect ATF trustzone from being
// accessed by the non-secure world
void protect_trustzone() {
    // Configures region 0 to point to TZRAM
    mmio_write_32(EMI_MPUA, EMI_REGION(TZRAM_PROT_START, TZRAM_PROT_END));

    uint32_t tz_permissions = 0;

    // All domains allow *secure* reads/writes only for TZRAM
    tz_permissions |= EMI_DOMAIN_PERMISSION(0, S_RW);
    tz_permissions |= EMI_DOMAIN_PERMISSION(1, S_RW);
    tz_permissions |= EMI_DOMAIN_PERMISSION(2, S_RW);

    // Region 0 and its control registers are locked
    tz_permissions |= EMI_REGION0_LOCKED;

    mmio_write_32(EMI_MPUI, tz_permissions);

    // Cause SLVERR response to all domains
    mmio_write_32(EMI_MPUM, EMI_DOMAIN_LOCKED | EMI_DOMAIN_SECURE_ONLY | EMI_DOMAIN_SLVERR_ON_VIOLATION);
    mmio_write_32(EMI_MPUN, EMI_DOMAIN_LOCKED | EMI_DOMAIN_SECURE_ONLY | EMI_DOMAIN_SLVERR_ON_VIOLATION);
    mmio_write_32(EMI_MPUO, EMI_DOMAIN_LOCKED | EMI_DOMAIN_SECURE_ONLY | EMI_DOMAIN_SLVERR_ON_VIOLATION);
}

#endif /* __SPACEX_PROTECT_TRUSTZONE_ICC_C_ */