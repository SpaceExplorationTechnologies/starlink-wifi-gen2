/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_io.h
*/

#ifndef __MT_WIFI_IO_H__
#define __MT_WIFI_IO_H__

struct _RTMP_ADAPTER;

UINT32 mt_physical_addr_map(struct _RTMP_ADAPTER *pAd, UINT32 addr);
BOOLEAN mt_mac_cr_range_mapping(struct _RTMP_ADAPTER *pAd, UINT32 *mac_addr);

VOID hif_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID hif_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);
VOID hif_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID hif_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID phy_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID phy_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID mcu_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mcu_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID sys_io_read32(ULONG reg, UINT32 *val);
VOID sys_io_write32(ULONG reg, UINT32 val);

VOID mac_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mac_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);
VOID mac_io_read16(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mac_io_write16(void *hdev_ctrl, UINT32 reg, UINT32 val);
VOID mac_io_read8(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mac_io_write8(void *hdev_ctrl, UINT32 reg, UINT32 val);


VOID hw_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);

VOID hw_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID mt_io_ops_init(void *hdev_ctrl, INT hifType);



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/

#define MAC_IO_READ32(_hdc, _R, _pV) mac_io_read32(_hdc, _R, _pV)
#define MAC_IO_WRITE32(_hdc, _R, _V) mac_io_write32(_hdc, _R, _V)

#define HIF_IO_READ32(_hdc, _R, _pV) hif_io_read32(_hdc, _R, _pV)
#define HIF_IO_WRITE32(_hdc, _R, _V) hif_io_write32(_hdc, _R, _V)

#define PHY_IO_READ32(_hdc, _R, _pV) phy_io_read32(_hdc, _R, _pV)
#define PHY_IO_WRITE32(_hdc, _R, _V) phy_io_write32(_hdc, _R, _V)

#define HW_IO_READ32(_hdc, _R, _pV) hw_io_read32(_hdc, _R, _pV)
#define HW_IO_WRITE32(_hdc, _R, _V) hw_io_write32(_hdc, _R, _V)

#define MCU_IO_READ32(_hdc, _R, _pV) mcu_io_read32(_hdc, _R, _pV)
#define MCU_IO_WRITE32(_hdc, _R, _V) mcu_io_write32(_hdc, _R, _V)

#define RTMP_IO_READ32(_hdc, _R, _pV) mac_io_read32(_hdc, _R, _pV)
#define RTMP_IO_WRITE32(_hdc, _R, _V) mac_io_write32(_hdc, _R, _V)

#define RTMP_SYS_IO_READ32(_R, _pV) sys_io_read32(_R, _pV)
#define RTMP_SYS_IO_WRITE32(_R, _V) sys_io_write32(_R, _V)

#define RTMP_IO_FORCE_READ32(_hdc, _R, _pV) hif_io_force_read32(_hdc, _R, _pV)
#define RTMP_IO_FORCE_WRITE32(_hdc, _R, _V) hif_io_force_write32(_hdc, _R, _V)
#define RTMP_IO_READ16(_hdc, _R, _pV) mac_io_read16(_hdc, _R, _pV)
#define RTMP_IO_WRITE16(_hdc, _R, _V) mac_io_write16(_hdc, _R, _V)
#define RTMP_IO_READ8(_hdc, _R, _pV) mac_io_read8(_hdc, _R, _pV)
#define RTMP_IO_WRITE8(_hdc, _R, _V) mac_io_write8(_hdc, _R, _V)


#endif
