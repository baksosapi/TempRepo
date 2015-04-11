/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    FaceProcPcAPI.h
 *
 */
/*
    Property Estimation Common Library API
*/

#ifndef FACEPROCPCAPI_H__
#define FACEPROCPCAPI_H__

#define FACEPROC_API
#include    "FaceProcDef.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**********************************************************/
/* Get Version                                            */
/**********************************************************/
/* Get Property Estimation Common Library API Version */
FACEPROC_API INT32      FACEPROC_PC_GetVersion(UINT8 *pbyMajor, UINT8 *pbyMinor);

#ifdef  __cplusplus
}
#endif


#endif /* FACEPROCPCAPI_H__ */
