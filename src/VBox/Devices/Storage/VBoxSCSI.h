/* $Id: VBoxSCSI.h $ */
/** @file
 *
 * VBox storage devices:
 * Simple SCSI interface for BIOS access
 */

/*
 * Copyright (C) 2006-2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/**
 * This is a simple interface to access SCSI devices from the BIOS
 * which is shared between the BusLogic and the LsiLogic
 * SCSI host adapters to simplify the BIOS part.
 *
 * The BusLogic interface if available will be starting at port 0x330
 * and the LsiLogic starts at 0x340 and each will have a size of 3 ports.
 * The ports are used as described below:
 *
 * +--------+--------+----------+
 * | Offset | Access | Purpose  |
 * +--------+--------+----------+
 * |   0    |  Write | Command  |
 * +--------+--------+----------+
 * |   0    |  Read  | Status   |
 * +--------+--------+----------+
 * |   1    |  Write | Data in  |
 * +--------+--------+----------+
 * |   1    |  Read  | Data out |
 * +--------+--------+----------+
 * |   2    |  R/W   | Detect   |
 * +--------+--------+----------+
 * |   3    |  Write | Reset    |
 * +--------+--------+----------+
 *
 * The register at port 0 receives the SCSI CDB issued from the driver when writing to it but
 * before writing the actual CDB the first write gives the size of the CDB in bytes.
 *
 * Reading the port at offset 0 gives status information about the adapter.
 * If the busy bit is set the adapter is processing a previous issued request if it is
 * cleared the command finished and the adapter can process another request.
 * The driver has to poll this bit because the adapter will not assert an IRQ for simplicity reasons.
 *
 * The register at offset 2 is to detect if a host adapter is available
 * If the driver writes a value to this port and gets the same value after reading it
 * again the adapter is available.
 *
 * This part has no R0 or GC components.
 */

#ifndef ___Storage_VBoxSCSI_h
#define ___Storage_VBoxSCSI_h

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
//#define DEBUG
#include <VBox/vmm/pdmdev.h>

typedef enum VBOXSCSISTATE
{
    VBOXSCSISTATE_NO_COMMAND            = 0x00,
    VBOXSCSISTATE_READ_TXDIR            = 0x01,
    VBOXSCSISTATE_READ_CDB_SIZE         = 0x02,
    VBOXSCSISTATE_READ_BUFFER_SIZE_LOW  = 0x03,
    VBOXSCSISTATE_READ_BUFFER_SIZE_HIGH = 0x04,
    VBOXSCSISTATE_READ_COMMAND          = 0x05,
    VBOXSCSISTATE_COMMAND_READY         = 0x06
} VBOXSCSISTATE;

#define VBOXSCSI_TXDIR_FROM_DEVICE 0
#define VBOXSCSI_TXDIR_TO_DEVICE   1

/** Maximum CDB size the BIOS driver sends. */
#define VBOXSCSI_CDB_SIZE_MAX     10

typedef struct VBOXSCSI
{
    /** The identify register. */
    uint8_t              regIdentify;
    /** The target device. */
    uint8_t              uTargetDevice;
    /** Transfer direction. */
    uint8_t              uTxDir;
    /** The size of the CDB we are issuing. */
    uint8_t              cbCDB;
    /** The command to issue. */
    uint8_t              aCDB[12];
    /** Current position in the array. */
    uint8_t              iCDB;

#if HC_ARCH_BITS == 64
    uint32_t             Alignment0;
#endif

    /** Pointer to the buffer holding the data. */
    R3PTRTYPE(uint8_t *) pBuf;
    /** Size of the buffer in bytes. */
    uint32_t             cbBuf;
    /** Current position in the buffer. */
    uint32_t             iBuf;
    /** Flag whether a request is pending. */
    volatile bool        fBusy;
    /** The state we are in when fetching a command from the BIOS. */
    VBOXSCSISTATE        enmState;
} VBOXSCSI, *PVBOXSCSI;

#define VBOX_SCSI_BUSY RT_BIT(0)

#ifdef IN_RING3
RT_C_DECLS_BEGIN
int vboxscsiInitialize(PVBOXSCSI pVBoxSCSI);
int vboxscsiReadRegister(PVBOXSCSI pVBoxSCSI, uint8_t iRegister, uint32_t *pu32Value);
int vboxscsiWriteRegister(PVBOXSCSI pVBoxSCSI, uint8_t iRegister, uint8_t uVal);
int vboxscsiSetupRequest(PVBOXSCSI pVBoxSCSI, PPDMSCSIREQUEST pScsiRequest, uint32_t *puTargetDevice);
int vboxscsiRequestFinished(PVBOXSCSI pVBoxSCSI, PPDMSCSIREQUEST pScsiRequest);
void vboxscsiSetRequestRedo(PVBOXSCSI pVBoxSCSI, PPDMSCSIREQUEST pScsiRequest);
int vboxscsiWriteString(PPDMDEVINS pDevIns, PVBOXSCSI pVBoxSCSI, uint8_t iRegister,
                        RTGCPTR *pGCPtrSrc, PRTGCUINTREG pcTransfer, unsigned cb);
int vboxscsiReadString(PPDMDEVINS pDevIns, PVBOXSCSI pVBoxSCSI, uint8_t iRegister,
                       RTGCPTR *pGCPtrDst, PRTGCUINTREG pcTransfer, unsigned cb);
RT_C_DECLS_END
#endif

#endif /* ___Storage_VBoxSCSI_h */
