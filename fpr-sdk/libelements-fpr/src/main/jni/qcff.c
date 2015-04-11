/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    qcff.c
 *
 */

#include "qcff_native.h"
#include "FaceProcAPI.h"
#include "FaceProcDef.h"
#include "FaceProcDtAPI.h"
#include "FaceProcTime.h"
#include "FaceProcPtAPI.h"
#include "FaceProcSmAPI.h"
#include "FaceProcFrAPI.h"
#include "FaceProcGbAPI.h"
#include "FaceProcCtAPI.h"
//#include "FaceProcEdAPI.h" //eye detection

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

//#define PROFILING
#ifdef PROFILING
#include <sys/time.h>
#endif

#define LOG_NIDEBUG 0
#define LOG_TAG "QCFF"
#include <android/log.h>
#define QCFF_LOG(fmt, args...)     __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOG(msg)   __android_log_print(ANDROID_LOG_DEBUG, "QCFF", msg);

/*========================================================================



 *//** @file qcff.c


 Copyright (c) 2011 QUALCOMM Incorporated. All Rights Reserved.
 QUALCOMM Proprietary and Confidential.

 *//*====================================================================== */

#define MAJOR_VERSION    1
#define MINOR_VERSION    1

#define NUM_MAX_REGISTERED_USERS  64

//#define QCFF_LOG
#define MIN2(a,b)      ((a<b)?a:b)
#define MIN4(a,b,c,d)  (MIN2(MIN2(a,b),MIN2(c,d)))
#define MAX2(a,b)      ((a>b)?a:b)
#define MAX4(a,b,c,d)  (MAX2(MAX2(a,b),MAX2(c,d)))

typedef struct {
    uint32_t SEARCH_DENSITY;
    int32_t FR_THRESHOLD;
    uint32_t PT_THRESHOLD;
    int32_t HIGH_CONFIDENCE_MARK;
    int32_t LOW_CONFIDENCE_MARK;
    uint32_t MIN_FACE_SIZE;
    uint32_t MAX_FACE_SIZE;
    uint32_t MAX_FACE_TO_DETECT;
    uint32_t MAX_REGISTERED_USERS;
    uint32_t MAX_DATA_PER_USER;
    uint32_t ROT_FRONT;
    uint32_t ROT_HALF_PROFILE;
    uint32_t ROT_PROFILE;

} qcff_default_params_t;

typedef struct {
    /* Frame dimension */
    uint32_t frame_width;
    uint32_t frame_height;
    /* Face-Engine-specific fields */
    HDETECTION hdt;
    HDTRESULT hdt_result;
    HPOINTER hpt;
    HPTRESULT hpt_result;
    HSMILE hsm;
    HSMRESULT hsm_result;
    HGAZEBLINK hgb;
    HGBRESULT hgb_result;
    HCONTOUR hct;
    HCTRESULT hct_result;
    HFEATURE hfr;
    HALBUM hal;
//HEYEDETECTION            hed; //eye detection
//HEDRESULT                hed_result; //eye detection

    uint32_t num_registered_users;
    uint32_t local_frame_size;

    /* Optional parameters */
    qcff_mode_t mode;
    uint32_t frontal_rot;
    uint32_t half_profile_rot;
    uint32_t profile_rot;

    /* Scratch buffers */
    void *bmem; /* backup memory */
    void *wmem; /* work memory */
    /* Flags */
    uint8_t fdBmemorySet;
    uint8_t fdWmemorySet;

    uint8_t *p_local_frame;
    uint32_t num_faces;

    /* Experimental feature: downscale processing */
    uint32_t downscale_factor;
} qcff_t;

/* Default parameters */
static qcff_default_params_t default_params = //very important, all conf values here
        { 33, /* SEARCH_DENSITY       */
        300, /* FR_THRESHOLD         */
        30, /* PT_THRESHOLD         */
        800, /* HIGH_CONFIDENCE_MARK */
        600, /* LOW_CONFIDENCE_MARK  */
        20, /* MIN_FACE_SIZE        */
        300, /* MAX_FACE_SIZE        */
        64, /* MAX_FACE_TO_DETECT   */
        1000, /* MAX_REGISTERED_USERS */
        10, /* MAX_DATA_PER_USER    */
        ROT_ANGLE_ULR45, //ANGLE_U45,  /* ROT_FRONT */
                ANGLE_NONE, /* ROT_HALF_PROFILE */
                ANGLE_NONE, /* ROT_PROFILE */
        };

static qcff_complete_face_info_t empty_info = { NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, };

/************************************************************************
 * Helper function prototypes
 ***********************************************************************/
static void qcff_translate_face_info_to_rect(FACEINFO*, qcff_face_rect_t*);
static int qcff_config_dt(qcff_t *p_qcff, qcff_config_t *p_cfg);
static int qcff_config_fr(qcff_t *p_qcff);
static int qcff_config_gb(qcff_t *p_qcff);
static int qcff_config_ct(qcff_t *p_qcff);
static int qcff_config_pt(qcff_t *p_qcff);
static int qcff_config_sm(qcff_t *p_qcff);
static int qcff_extract_feature(qcff_t *p_qcff, uint32_t face_index,
        HFEATURE hfr);

/************************************************************************
 * Main exposed wrapper functions below
 ***********************************************************************/
/************************************************************************
 * qcff_is_feature_supported
 *
 * This function will check to see if the device supports facial
 * processing.
 *
 * RETURN_VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_FAILURE
 ************************************************************************/
int qcff_is_feature_supported(qcff_feature feature) {
    void * handle = dlopen("vendor/lib/libmmcamera_faceproc.so", RTLD_LAZY);
    if (handle != NULL) {
        dlclose(handle);
        return QCFF_RET_SUCCESS;
    } else {
        return QCFF_RET_FAILURE;
    }
} //Keep
/*************************************************************************
 * qcff_get_version
 *
 * This function retrieves the version of the framework API.
 *
 * OUTPUT:       p_version_major    Major version of the framework API.
 *               p_version_minor    Minor version of the framework API.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_version(uint32_t *p_version_major, uint32_t *p_version_minor) {
    if (!p_version_major || !p_version_minor)
        return QCFF_RET_INVALID_PARM;

    *p_version_major = MAJOR_VERSION;
    *p_version_minor = MINOR_VERSION;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_create
 *
 * All QCFF interface functions (except get version) require the handle
 * to an instance of QCFF as the first parameter. It encapsulates all
 * necessary memory and session management inside. This function creates
 * such a handle for subsequent use. The handle should be destroyed by
 * qcff_destroy at the end of use.
 *
 * OUTPUT:       p_handle    Created handle if operation successful.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_NO_RESOURCE
 ************************************************************************/
int qcff_create(qcff_handle_t *p_handle) {
    qcff_t *p_qcff;

    if (!p_handle)
        return QCFF_RET_FAILURE;

    p_qcff = (qcff_t *) malloc(sizeof(qcff_t));
    if (NULL == p_qcff)
        return QCFF_RET_NO_RESOURCE;

    memset((void*) p_qcff, 0, sizeof(qcff_t));
    qcff_config_fr(p_qcff);
    qcff_config_gb(p_qcff);
    qcff_config_ct(p_qcff);
    qcff_config_pt(p_qcff);
    qcff_config_sm(p_qcff);
//p_qcff->frontal_rot = default_params.ROT_FRONT; May 13
//p_qcff->half_profile_rot = default_params.ROT_HALF_PROFILE; //May 13
//p_qcff->profile_rot = default_params.ROT_PROFILE; //May 13
    QCFF_LOG("qcff_create: succeeded 0x%p", p_qcff);
    /* Output the new engine */
    *p_handle = (qcff_t *) p_qcff;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_set_detect_rot
 *
 * This function provides the optional parameter of the angles to detect
 * in different planes. There are three planes to configure: frontal,
 * half-profile (+/-30 degree from frontal) and profile (+/-90 side view).
 * For each plane, valid configuration includes the combinations of:
 * ROT_ANGLE_NONE  (Does not detect),
 * ROT_ANGLE_0  (-15 to 15    degrees),
 * ROT_ANGLE_1  (15   to 45   degrees),
 * ROT_ANGLE_2  (45   to 75   degrees),
 * ROT_ANGLE_3  (75   to 105  degrees),
 * ROT_ANGLE_4  (105  to 135  degrees),
 * ROT_ANGLE_5  (135  to 165  degrees),
 * ROT_ANGLE_6  (165  to -165 degrees),
 * ROT_ANGLE_7  (-165 to -135 degrees),
 * ROT_ANGLE_8  (-135 to -105 degrees),
 * ROT_ANGLE_9  (-105 to -75  degrees),
 * ROT_ANGLE_10 (-75  to -45  degrees),
 * ROT_ANGLE_11 (-45  to -15  degrees),
 * ROT_ANGLE_ALL (Detects all angles)
 *
 * INPUT:        handle       Handle to QCFF instance created previously.
 *               frontal      Degree of rotation in the frontal plane.
 *               half_profile Degree of rotation in the half profile plane.
 *               profile      Degree of rotation in the side profile plane.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_set_detect_rot(qcff_handle_t handle, uint32_t frontal,
        uint32_t half_profile, uint32_t profile) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;

    if (!p_qcff)
        return QCFF_RET_INVALID_PARM;

    p_qcff->frontal_rot = (uint32_t) frontal;
    p_qcff->half_profile_rot = (uint32_t) half_profile;
    p_qcff->profile_rot = (uint32_t) profile;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_config
 *
 * This function provides mandatory configurations to the QCFF instance.
 * It includes the input image resolution (width and height). QCFF will
 * read all subsequent inputs assuming the same resolution. It currently
 * assumes and accepts only 8-bit grayscale images.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *               p_cfg      Pointer to the configuration structure.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_config(qcff_handle_t handle, qcff_config_t *p_cfg) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc = QCFF_RET_FAILURE;

    if (!p_qcff) {
        QCFF_LOG("QCCameraSDKConfig falsed initialy");
        return QCFF_RET_FAILURE;
    }

    /* Experimental feature: downscale processing */
    if (p_cfg->downscale_factor != 2 && p_cfg->downscale_factor != 4)
        p_cfg->downscale_factor = 1;
    p_qcff->downscale_factor = p_cfg->downscale_factor;

    QCFF_LOG("QCCameraSDKConfig width x height %d x %d",
            p_cfg->width, p_cfg->height);
    /* Delete local frame buffer */
    if (p_qcff->p_local_frame)
        free(p_qcff->p_local_frame);
    p_qcff->p_local_frame = (uint8_t *) malloc(p_cfg->width * p_cfg->height);

    if (p_qcff->p_local_frame == 0) {
        QCFF_LOG("p_local_frame malloc failed");
        return QCFF_RET_FAILURE;
    }

    rc = qcff_config_dt(p_qcff, p_cfg);

    if (QCFF_FAILED(rc)) {
        QCFF_LOG("QCFF_config failed");
        return QCFF_RET_FAILURE;
    }
    p_qcff->local_frame_size = p_cfg->width * p_cfg->height;

    /* Save the input frame dimension */
    p_qcff->frame_width = p_cfg->width;
    p_qcff->frame_height = p_cfg->height;

    return QCFF_RET_SUCCESS;
} //KEEP

/*************************************************************************
 * qcff_set_frame
 *
 * This function provides the input to the QCFF instance. A local copy of
 * the frame will be made and therefore the input frame can be released
 * and altered freely after this call is finished.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_set_frame(qcff_handle_t handle, uint8_t *p_frame) {
#ifdef PROFILING
    struct timeval t1, t2;
    int diff;
    static FILE* fdump = NULL;
    static int cnt = 0;
#endif

    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;

    if (!p_qcff || !p_frame)
        return QCFF_RET_INVALID_PARM;
    if (p_frame == 0) {
        LOG("p_frame is 0");
    }

#ifdef PROFILING
    gettimeofday(&t1, NULL);
#endif
    /* Experimental feature: downscale processing */
    if (p_qcff->downscale_factor != 1) {
        uint8_t *p_src = p_frame;
        uint8_t *p_dest = p_qcff->p_local_frame;
        uint32_t j = p_qcff->frame_height / p_qcff->downscale_factor;
        uint32_t i_cache = p_qcff->frame_width / p_qcff->downscale_factor;
        if (p_qcff->downscale_factor == 2) {
            while (j--) {
                uint32_t i = i_cache;
                while (i--) {
                    *p_dest++ = *p_src++;
                    p_src++;
                }
                p_src = p_src + p_qcff->frame_width;
            }
        } else if (p_qcff->downscale_factor == 4) {
            while (j--) {
                uint32_t i = i_cache;
                while (i--) {
                    *p_dest++ = *p_src++;
                    p_src++;
                    p_src++;
                    p_src++;
                }
                p_src = p_src + p_qcff->frame_width * 3;
            }
        }
    } else {
        /* Make a copy of the frame */
//QCFF_LOG("Before memcopy wxh %d %d", p_qcff->frame_width, p_qcff->frame_height);
        memcpy((void *) p_qcff->p_local_frame, (void *) p_frame,
                p_qcff->frame_width * p_qcff->frame_height);
    }

//LOG("After memcopy");
    /* Do detection */
    rc = FACEPROC_Detection(p_qcff->hdt, (RAWIMAGE *) p_qcff->p_local_frame,
            p_qcff->frame_width / p_qcff->downscale_factor,
            p_qcff->frame_height / p_qcff->downscale_factor, ACCURACY_HIGH_TR,
            p_qcff->hdt_result);
//LOG("Ater FQCEPROC_Detection");

    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_Detection returned %d %d",
                (uint32_t)rc, p_qcff->frame_width);
        return QCFF_RET_FAILURE;
    }
#ifdef PROFILING
    gettimeofday(&t2, NULL);
    diff = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000;
    QCFF_LOG("set frame: detection takes: %d ms", diff);
#if 0
    if (fdump == NULL) {
        fdump = fopen("/data/data/com.android.qcurf/data/dump.csv", "w");
    } else {
        cnt++;
        if (cnt < 350 && cnt > 50) {
            fprintf(fdump, "%d,%d\n", cnt, diff);
        } else if (cnt == 350) {
            fclose(fdump);
        }
    }
#endif
#endif

    /* Get the number of faces */
    rc = FACEPROC_GetDtFaceCount(p_qcff->hdt_result,
            (INT32*) &p_qcff->num_faces);
//QCFF_LOG("After FACEPROC_GetDT... %d", rc);
    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_GetDtFaceCount returned %d", (uint32_t)rc);
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_set_mode
 *
 * This function sets the optional parameter: detection mode.
 * It should be configured differently when QCFF is used in still image
 * vs video sequence scenario. If unset, default is QCFF_MODE_VIDEO.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_set_mode(qcff_handle_t handle, qcff_mode_t mode) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;

    if (!p_qcff || mode >= QCFF_MODE_MAX)
        return QCFF_RET_INVALID_PARM;

    p_qcff->mode = mode;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_get_num_faces
 *
 * This function retrieves the number of detected faces in the frame
 * received in the last 'qcff_set_frame' call.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_num_faces(qcff_handle_t handle, uint32_t *p_num_faces) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;

    if (!p_qcff || !p_num_faces)
        return QCFF_RET_INVALID_PARM;

    *p_num_faces = p_qcff->num_faces;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_get_rects
 *
 * This function retrieves the rectangles of the detected faces. It
 * allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the face rectangles requested. The output
 * will be an array of face rectangles in the type of qcff_face_rect_t.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the rectangle information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_face_rects         The face rectangles outputted.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_rects(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_face_rect_t *p_face_rects) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_face_rects)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_rects = p_face_rects;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_parts
 *
 * This function retrieves the facial parts of the detected faces. It
 * allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of facial parts in the type of qcff_face_parts_t.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the facial parts information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_face_parts         The facial parts outputted.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_parts(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_face_parts_t *p_face_parts) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_face_parts)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_parts = p_face_parts;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_parts_ex
 *
 * This function retrieves the extended facial parts of the detected faces.
 * It allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of facial parts in the type of qcff_face_parts_ex_t.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the facial parts information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_face_parts_ex      The extended facial parts outputted.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_parts_ex(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_face_parts_ex_t *p_face_parts_ex) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_face_parts_ex)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_parts_ex = p_face_parts_ex;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_smiles
 *
 * This function retrieves the smile degree of the detected faces. It
 * allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of smile degrees. The smile degree ranges from
 * 0 to 100 with 0 being the lowest smile degree.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the facial parts information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_smile_degrees      The smile degrees outputted.
 *                                    They range from 0 to 100.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_smiles(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        uint32_t *p_smile_degrees) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_smile_degrees)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_smile_degrees = p_smile_degrees;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_eye_opens
 *
 * This function retrieves the eye open-close degree of the detected faces.
 * It allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of eye open-close degrees. They range from 0 to 100
 * with 0 meaning complete close of the eye.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the facial parts information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_eye_open_degrees   The eye open-close degree outputted.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_eye_opens(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_eye_open_deg_t *p_eye_open_degrees) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_eye_open_degrees)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_eye_open_degrees = p_eye_open_degrees;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_gaze_degs
 *
 * This function retrieves the gaze degree of the detected faces.
 * It allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of gaze degrees. They range from -90 to 90 for
 * both the left-right and up-down plane.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the gaze degree information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_gaze_degrees       The gaze degree outputted.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_gaze_degs(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_gaze_deg_t *p_gaze_degrees) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_gaze_degrees)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_gaze_degrees = p_gaze_degrees;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_directions
 *
 * This function retrieves the face directions of the detected faces.
 * It allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the faces requested. The output
 * will be an array of face directions.
 * The face indices are zero-based and should be between 0 to the
 * number of detected faces - 1.

 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the facial parts information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_face_directions    The face directions outputted.

 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_directions(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_face_dir_t *p_face_directions) {
    qcff_complete_face_info_t complete_info = empty_info;

    if (!p_face_directions)
        return QCFF_RET_INVALID_PARM;

    complete_info.p_directions = p_face_directions;

    return qcff_get_complete_info(handle, num_faces_queried, p_face_indices,
            p_num_faces_returned, &complete_info);
}

/*************************************************************************
 * qcff_get_complete_info
 *
 * This function retrieves the all possible information about each face
 * such as locations, directions, smile degrees, eye open-close degrees, etc.
 * It allows multiple faces to be retrieved by taking an array of face
 * indices corresponding to the information requested. The output
 * is an array for a structure containing pointers to various information
 * of the face. For each face, user can select a subset of information to
 * retrieve by providing valid pointers to those information in interest.
 * For other pointers, they can be set to NULL. The function will skip
 * for pointers that are NULL. The face indices are zero-based and should be
 * between 0 to the number of detected faces - 1.

 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               num_faces_queried    The number of faces being queried.
 *                                    (it determines how many entries in
 *                                    p_face_indices to be read)
 *               p_face_indices       The indices of the faces in which
 *                                    the complete information is
 *                                    requested. Whenever an invalid
 *                                    face index is encountered, the
 *                                    function call ends.
 *                                    p_num_faces_returned indicates how
 *                                    many faces are returned before
 *                                    the error happened.
 * OUTPUT:       p_num_faces_returned Returns the actual number of
 *                                    entries returned. It might be
 *                                    less than the number of faces queried
 *                                    if the number queried is actually
 *                                    more than the available faces.
 *               p_complete_info      The complete infos outputted.

 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_complete_info(qcff_handle_t handle, uint32_t num_faces_queried,
        uint32_t *p_face_indices, uint32_t *p_num_faces_returned,
        qcff_complete_face_info_t *p_complete_info) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;
    uint32_t i, j;

    /*if (!p_qcff || !p_complete_info || !p_face_indices || !p_num_faces_returned)
     return QCFF_RET_INVALID_PARM;*/

    /* Loop through each requested face */
    for (i = 0; i < num_faces_queried; i++) {

        /* Face Rectangle requested */
        if (p_complete_info->p_rects) {
            FACEINFO face_info;

            if (FACEPROC_NORMAL
                    != FACEPROC_GetDtFaceInfo(p_qcff->hdt_result,
                            p_face_indices[i], &face_info))
                break;
            /* Experimental feature: downscale processing */
            if (p_qcff->downscale_factor != 1) {
                face_info.ptLeftTop.x *= p_qcff->downscale_factor;
                face_info.ptLeftTop.y *= p_qcff->downscale_factor;
                face_info.ptRightTop.x *= p_qcff->downscale_factor;
                face_info.ptRightTop.y *= p_qcff->downscale_factor;
                face_info.ptLeftBottom.x *= p_qcff->downscale_factor;
                face_info.ptLeftBottom.y *= p_qcff->downscale_factor;
                face_info.ptRightBottom.x *= p_qcff->downscale_factor;
                face_info.ptRightBottom.y *= p_qcff->downscale_factor;
            }

            qcff_translate_face_info_to_rect(&face_info,
                    &(p_complete_info->p_rects[i]));
        }

        /* Perform parts detection of any other info requested */
        if (p_complete_info->p_parts || p_complete_info->p_parts_ex
                || p_complete_info->p_smile_degrees
                || p_complete_info->p_gaze_degrees
                || p_complete_info->p_eye_open_degrees
                || p_complete_info->p_directions) {

            /* Set face location to parts detection handle */
            if (FACEPROC_NORMAL
                    != FACEPROC_PT_SetPositionFromHandle(p_qcff->hpt,
                            p_qcff->hdt_result, p_face_indices[i]))
                break;

            /* Do parts detection */
            if (FACEPROC_NORMAL
                    != FACEPROC_PT_DetectPoint(p_qcff->hpt,
                            (RAWIMAGE *) p_qcff->p_local_frame,
                            p_qcff->frame_width / p_qcff->downscale_factor,
                            p_qcff->frame_height / p_qcff->downscale_factor,
                            p_qcff->hpt_result))
                break;

            /* Extract parts information if requested */
            if (p_complete_info->p_parts) {
                POINT points[QCFF_PARTS_MAX];
                INT32 confs[QCFF_PARTS_MAX];

                if (FACEPROC_NORMAL
                        != FACEPROC_PT_GetResult(p_qcff->hpt_result,
                                QCFF_PARTS_MAX, points, confs))
                    break;

                for (j = 0; j < QCFF_PARTS_MAX; j++) {
                    p_complete_info->p_parts[i].parts[j].x = points[j].x
                            * p_qcff->downscale_factor;
                    p_complete_info->p_parts[i].parts[j].y = points[j].y
                            * p_qcff->downscale_factor;
                }
            }

            /* Extract Face Direction */
            if (p_complete_info->p_directions) {
                if (FACEPROC_NORMAL
                        != FACEPROC_PT_GetFaceDirection(p_qcff->hpt_result,
                                (INT32*) &p_complete_info->p_directions[i].up_down_in_degree,
                                (INT32*) &p_complete_info->p_directions[i].left_right_in_degree,
                                (INT32*) &p_complete_info->p_directions[i].roll_in_degree))
                    break;

            }

            /* Extract extended parts information if requested */
            if (p_complete_info->p_parts_ex) {
                POINT points[QCFF_PARTS_EX_MAX];

                if (FACEPROC_NORMAL
                        != FACEPROC_CT_SetPointFromHandle(p_qcff->hct,
                                p_qcff->hpt_result))
                    break;
                if (FACEPROC_NORMAL
                        != FACEPROC_CT_DetectContour(p_qcff->hct,
                                p_qcff->p_local_frame,
                                p_qcff->frame_width / p_qcff->downscale_factor,
                                p_qcff->frame_height / p_qcff->downscale_factor,
                                p_qcff->hct_result))
                    break;
                if (FACEPROC_NORMAL
                        != FACEPROC_CT_GetResult(p_qcff->hct_result,
                                QCFF_PARTS_EX_MAX, points))
                    break;

                for (j = 0; j < QCFF_PARTS_EX_MAX; j++) {
                    p_complete_info->p_parts_ex[i].parts[j].x = points[j].x
                            * p_qcff->downscale_factor;
                    p_complete_info->p_parts_ex[i].parts[j].y = points[j].y
                            * p_qcff->downscale_factor;
                }
            }

            /* Perform smile degree estimation if requested */
            if (p_complete_info->p_smile_degrees) {
                INT32 smile_conf;
                /* Set Smile Estimation Point from handle */
                if (FACEPROC_NORMAL
                        != FACEPROC_SM_SetPointFromHandle(p_qcff->hsm,
                                p_qcff->hpt_result))
                    break;

                /* Do Smile Estimation */
                if (FACEPROC_NORMAL
                        != FACEPROC_SM_Estimate(p_qcff->hsm,
                                (RAWIMAGE *) p_qcff->p_local_frame,
                                p_qcff->frame_width / p_qcff->downscale_factor,
                                p_qcff->frame_height / p_qcff->downscale_factor,
                                p_qcff->hsm_result))
                    break;

                /* Extract Result */
                if (FACEPROC_NORMAL
                        != FACEPROC_SM_GetResult(p_qcff->hsm_result,
                                (INT32*) &p_complete_info->p_smile_degrees[i],
                                &smile_conf))
                    break;
            }

            /* Perform gaze-blink estimation if requested */
            if (p_complete_info->p_eye_open_degrees
                    || p_complete_info->p_gaze_degrees) {

                /* Set Gaze-Blink Estimation Point from handle */
                if (FACEPROC_NORMAL
                        != FACEPROC_GB_SetPointFromHandle(p_qcff->hgb,
                                p_qcff->hpt_result))
                    break;

                /* Do Gaze-Blink Estimation */
                if (FACEPROC_NORMAL
                        != FACEPROC_GB_Estimate(p_qcff->hgb,
                                (RAWIMAGE *) p_qcff->p_local_frame,
                                p_qcff->frame_width / p_qcff->downscale_factor,
                                p_qcff->frame_height / p_qcff->downscale_factor,
                                p_qcff->hgb_result))
                    break;

                /* Extract Gaze Result */
                if (p_complete_info->p_gaze_degrees) {
                    if (FACEPROC_NORMAL
                            != FACEPROC_GB_GetGazeDirection(p_qcff->hgb_result,
                                    (INT32*) &p_complete_info->p_gaze_degrees[i].left_right,
                                    (INT32*) &p_complete_info->p_gaze_degrees[i].up_down))
                        break;
                }
                /* Extract Eye Open-Close Result */
                if (p_complete_info->p_eye_open_degrees) {
                    if (FACEPROC_NORMAL
                            != FACEPROC_GB_GetEyeCloseRatio(p_qcff->hgb_result,
                                    (INT32*) &p_complete_info->p_eye_open_degrees[i].left,
                                    (INT32*) &p_complete_info->p_eye_open_degrees[i].right))
                        break;
                    p_complete_info->p_eye_open_degrees[i].left /= 10;
                    p_complete_info->p_eye_open_degrees[i].right /= 10;
                }
            }
        } /* If infos other than rectangles is needed */
    } /* For loop on face indices */

    *p_num_faces_returned = i;
    return QCFF_RET_SUCCESS;
}

int qcff_create_feature_cache(qcff_handle_t handle, uint32_t face_index,
        qcff_face_feature_t* p_feature) {
    HFEATURE hfr = NULL;
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;

    if (!p_qcff || !p_feature)
        return QCFF_RET_INVALID_PARM;

    hfr = FACEPROC_FR_CreateFeatureHandle();
    if (!hfr)
        return QCFF_RET_NO_RESOURCE;

    rc = qcff_extract_feature(p_qcff, face_index, hfr);
    if (QCFF_RET_SUCCESS != rc)
        FACEPROC_FR_DeleteFeatureHandle(hfr);
    else
        *p_feature = (qcff_face_feature_t) hfr;
    QCFF_LOG("Create_Feature_Cache: rc = %d hfr = %p face_index = %d",
            rc, *p_feature, face_index);
    return rc;
}

int qcff_destroy_feature_cache(qcff_face_feature_t feature) {
    return FACEPROC_FR_DeleteFeatureHandle((HFEATURE) feature);
}

int qcff_find_unused_user_id(qcff_t *p_qcff, uint32_t *new_user_id) {
    int32_t user_id = 0;
    while (1) {
        INT32 num_data;
        INT32 rc = FACEPROC_FR_GetRegisteredUsrDataNum(p_qcff->hal, user_id,
                &num_data);
        QCFF_LOG("result=%d user %d has %d data", rc, user_id, num_data);
        if (num_data == 0) {
            *new_user_id = (uint32_t) user_id;
            return QCFF_RET_SUCCESS;
        }
        user_id++;
    }
    return QCFF_RET_FAILURE;
}

/*************************************************************************
 * qcff_reg_new_usr
 *
 * This function registers a new user in the internal database. It
 * requires the association of one of the detected faces. The face index
 * is zero-based and should be between 0 to the number of detected faces-1.
 * When the operation is successful, the new user ID is returned. This is
 * used to subsequently refer to this user during identification.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *               feature    Feature obtained earlier through
 *                          qcff_create_feature_cache.
 * OUTPUT:       p_new_user_id  The user ID of the new user returned.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_reg_new_usr(qcff_handle_t handle, qcff_face_feature_t feature,
        uint32_t *p_new_user_id) {
    qcff_t *p_qcff = (qcff_t *) handle;
    HFEATURE hfr = (qcff_face_feature_t) feature;
    int rc;

    if (!p_qcff || !p_new_user_id || !hfr)
        return QCFF_RET_INVALID_PARM;

//*p_new_user_id = p_qcff->num_registered_users;
    qcff_find_unused_user_id(p_qcff, p_new_user_id);

    /* Register the new user with the extracted feature */
    if (FACEPROC_NORMAL
            != FACEPROC_FR_RegisterData(p_qcff->hal, hfr, *p_new_user_id, 0))
        return QCFF_RET_FAILURE;

    p_qcff->num_registered_users++;
    QCFF_LOG("qcff_reg_new_usr: hfr = %p successful", feature);
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_reg_ex_usr
 *
 * This function registers an existing user. This improves the face
 * recognition accuracy by learning faces of the same person under
 * different lighting conditions, poses and out-of-plane rotations.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *               feature    Feature obtained earlier through
 *                          qcff_create_feature_cache.
 *               user_id    The user ID of the existing user.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_reg_ex_usr(qcff_handle_t handle, qcff_face_feature_t feature,
        uint32_t user_id) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;
    INT32 num_data;
    HFEATURE hfr = (qcff_face_feature_t) feature;

    if (!p_qcff || !hfr || user_id >= p_qcff->num_registered_users)
        return QCFF_RET_INVALID_PARM;

    /* Get current number of data as the new data ID */
    if (FACEPROC_NORMAL
            != FACEPROC_FR_GetRegisteredUsrDataNum(p_qcff->hal, user_id,
                    &num_data))
        return QCFF_RET_FAILURE;

    /* Register the existing user */
    if (FACEPROC_NORMAL
            != FACEPROC_FR_RegisterData(p_qcff->hal, hfr, user_id, num_data))
        return QCFF_RET_FAILURE;

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_remove_ex_usr
 *
 * This function removes an existing user from the database.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *               user_id    The user ID of the existing user to be removed.
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_remove_ex_usr(qcff_handle_t handle, uint32_t user_id) {
    qcff_t *p_qcff = (qcff_t *) handle;
    INT32 num_users;

    if (!p_qcff || user_id >= p_qcff->num_registered_users)
        return QCFF_RET_INVALID_PARM;

    QCFF_LOG("Clearing user %d", user_id);
    /* Clear user from database */
    if (FACEPROC_NORMAL != FACEPROC_FR_ClearUser(p_qcff->hal, user_id))
        return QCFF_RET_FAILURE;

    QCFF_LOG("Cleared successfully");
    p_qcff->num_registered_users--;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_identify_usr
 *
 * This function performs face recognition by matching the subject face
 * to the bank of previously registered faces. The result is the ID of
 * the user which has the closest match to the subject face. It also
 * outputs the confidence level of the claimed match. Caller can react
 * differently depending on the confidence level, such as asking the
 * user to confirm its identity if the confidence level is not high.
 * This gives the opportunity for this new face to be registered also
 * to improve accuracy later.
 *
 * INPUT:        handle       Handle to QCFF instance created previously.
 *               face_index   Zero-based index to indicate which detected
 *                            face is to be used for matching.
 * OUTPUT:       p_user_id    The ID of the user having the closest match
 *                            against the subject face. This is only
 *                            meaningful when the return value of the
 *                            function call is QCFF_RET_SUCCESSFUL.
 *                            In any other case, this output argument
 *                            is left untouched.
 *               p_confidence The confidence level of the match. Same with
 *                            p_user_id, it is valid only when return value
 *                            of this call is QCFF_RET_SUCCESSFUL.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 *               QCFF_RET_NO_MATCH
 ************************************************************************/
int qcff_identify_usr(qcff_handle_t handle, uint32_t face_index,
        uint32_t *p_user_id,
//qcff_confidence_t   *p_confidence)
        uint32_t *p_confidence) {
    qcff_t *p_qcff = (qcff_t *) handle;
    int rc;
    INT32 score, num_users_returned;

    if (!p_qcff || !p_user_id || face_index >= p_qcff->num_faces
            || !p_confidence)
        return QCFF_RET_INVALID_PARM;

    if (p_qcff->num_registered_users == 0)
        return QCFF_RET_NO_MATCH;

    /* Extract feature */
    rc = qcff_extract_feature(p_qcff, face_index, p_qcff->hfr);
    if (QCFF_RET_SUCCESS != rc)
        return rc;

    /* Identify the most probable user */
    if (FACEPROC_NORMAL
            != FACEPROC_FR_Identify(p_qcff->hfr, p_qcff->hal, 1,
                    (INT32*) p_user_id, &score, &num_users_returned))
        return QCFF_RET_FAILURE;

    /* Check score against threshold */
    if (!num_users_returned || (score < default_params.FR_THRESHOLD))
        return QCFF_RET_NO_MATCH;

    /* Map score to confidence
     if (score >= default_params.HIGH_CONFIDENCE_MARK)
     *p_confidence = QCFF_CONFIDENCE_HIGH;
     else if (score >= default_params.LOW_CONFIDENCE_MARK)
     *p_confidence = QCFF_CONFIDENCE_MEDIUM;
     else
     p_confidence = QCFF_CONFIDENCE_LOW;
     */
    *p_confidence = score / 10;

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_get_num_ex_usrs
 *
 * This function queries for the total number of registered users.
 *
 * INPUT:        handle       Handle to QCFF instance created previously.
 * OUTPUT:       p_num_users  The number of previously registered users.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_num_ex_usrs(qcff_handle_t handle, uint32_t *p_num_users) {
    qcff_t *p_qcff = (qcff_t *) handle;

    if (!p_qcff || !p_num_users)
        return QCFF_RET_INVALID_PARM;

    if (FACEPROC_NORMAL
            != FACEPROC_FR_GetRegisteredUserNum(p_qcff->hal,
                    (INT32*) p_num_users))
        return QCFF_RET_FAILURE;

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_reset_usr_data
 *
 * This functions resets all registration/user data.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_reset_usr_data(qcff_handle_t handle) {
    qcff_t *p_qcff = (qcff_t *) handle;

    if (!p_qcff)
        return QCFF_RET_INVALID_PARM;

    if (FACEPROC_NORMAL != FACEPROC_FR_ClearAlbum(p_qcff->hal))
        return QCFF_RET_FAILURE;

    p_qcff->num_registered_users = 0;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_get_usr_data_size
 *
 * For recognition to be useful, typically the user data needs to be
 * remembered across sessions. QCFF provides a set of functions to
 * allow easy storing and restoring of its user data bank. To save the
 * user data, QCFF provides the qcff_get_usr_data function to serialize
 * the user data and writes it to a buffer. The caller can then saves
 * what is in the buffer to persistent storage such as the file system.
 * This function (qcff_get_usr_data_size) queries the size of the user
 * data when serialized. This should be called to prepare for the memory
 * buffer for the saving of user data.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 * OUTPUT:       p_size     The size needed to hold the serialized the
 *                          user data.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_usr_data_size(qcff_handle_t handle, uint32_t *p_size) {
    qcff_t *p_qcff = (qcff_t *) handle;

    if (!p_qcff || !p_size)
        return QCFF_RET_INVALID_PARM;

    if (FACEPROC_NORMAL
            != FACEPROC_FR_GetSerializedAlbumSize(p_qcff->hal,
                    (UINT32*) p_size)) {
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_get_usr_data
 *
 * For recognition to be useful, typically the user data needs to be
 * remembered across sessions. QCFF provides a set of functions to
 * allow easy storing and restoring of its user data bank. To save the
 * user data, QCFF provides the qcff_get_usr_data function to serialize
 * the user data and writes it to a buffer. The caller can then saves
 * what is in the buffer to persistent storage such as the file system.
 * Caller should use qcff_get_usr_data_size in advance to query the size
 * of the user data when serialized in order to allocate for the memory
 * buffer big enough to hold the serialized user data.
 *
 * INPUT:        handle               Handle to QCFF instance created
 *                                    previously.
 *               p_buffer             The buffer to be used for holding
 *                                    the serialized user data.
 *               num_bytes_in_buffer  The total number of bytes
 *                                    available in the buffer.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_get_usr_data(qcff_handle_t handle, uint8_t *p_buffer,
        uint32_t num_bytes_in_buffer) {
    qcff_t *p_qcff = (qcff_t *) handle;

    if (!p_qcff || !p_buffer || !num_bytes_in_buffer)
        return QCFF_RET_INVALID_PARM;

    if (FACEPROC_NORMAL
            != FACEPROC_FR_SerializeAlbum(p_qcff->hal, p_buffer,
                    num_bytes_in_buffer))
        return QCFF_RET_FAILURE;

    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_set_usr_data
 *
 * This functions takes previously serialized user data (through
 * qcff_get_usr_data) and restore the user data bank from it.
 *
 * INPUT:        handle              Handle to QCFF instance created
 *                                   previously.
 *               num_bytes_in_buffer The number of bytes in the buffer.
 *               p_buffer            The buffer holding the serialized
 *                                   user data.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_set_usr_data(qcff_handle_t handle, uint32_t num_bytes_in_buffer,
        uint8_t *p_buffer) {
    qcff_t *p_qcff = (qcff_t *) handle;
    INT32 num_users, rc;
    FR_ERROR error;

    if (!p_qcff || !num_bytes_in_buffer || !p_buffer) {
        return QCFF_RET_INVALID_PARM;
    }

    /* Delete previously created album first */
    if (FACEPROC_NORMAL != FACEPROC_FR_DeleteAlbumHandle(p_qcff->hal))
        return QCFF_RET_FAILURE;

    /* Restore album based on the provided buffer */
    p_qcff->hal = FACEPROC_FR_RestoreAlbum((UINT8*) p_buffer,
            (UINT32) num_bytes_in_buffer, &error);
    if (FR_NORMAL != error)
        return QCFF_RET_FAILURE;

    rc = FACEPROC_FR_GetRegisteredUserNum(p_qcff->hal, &num_users);
    if (FACEPROC_NORMAL != rc)
        return QCFF_RET_FAILURE;

    QCFF_LOG("Album restored successfully: %d users found", num_users);

    p_qcff->num_registered_users = (uint32_t) num_users;
    return QCFF_RET_SUCCESS;
}

/*************************************************************************
 * qcff_destroy
 *
 * This function destroys and performs clean up associated with the
 * QCFF instance previously created.
 *
 * INPUT:        handle     Handle to QCFF instance created previously.
 *
 * RETURN VALUE: QCFF_RET_SUCCESS
 *               QCFF_RET_INVALID_PARM
 ************************************************************************/
int qcff_destroy(qcff_handle_t *p_handle) {
    INT32 ret;
    qcff_t *p_qcff = (qcff_t *) *p_handle;

    if (!p_qcff)
        return QCFF_RET_FAILURE;

    /* Delete Album Handle */
    if (p_qcff->hal) {
        ret = FACEPROC_FR_DeleteAlbumHandle(p_qcff->hal);
        p_qcff->hal = NULL;
    }
    /* Delete Facial Feature Handle */
    if (p_qcff->hfr) {
        ret = FACEPROC_FR_DeleteFeatureHandle(p_qcff->hfr);
        p_qcff->hfr = NULL;
    }
    /* Delete Contour Result Handle */
    if (p_qcff->hct_result) {
        ret = FACEPROC_CT_DeleteResultHandle(p_qcff->hct_result);
        p_qcff->hct_result = NULL;
    }
    /* Delete Contour Detection Handle */
    if (p_qcff->hct) {
        ret = FACEPROC_CT_DeleteHandle(p_qcff->hct);
        p_qcff->hct = NULL;
    }
    /* Delete GazeBlink Result Handle */
    if (p_qcff->hgb_result) {
        ret = FACEPROC_GB_DeleteResultHandle(p_qcff->hgb_result);
        p_qcff->hgb_result = NULL;
    }
    /* Delete GazeBlink Detection Handle */
    if (p_qcff->hgb) {
        ret = FACEPROC_GB_DeleteHandle(p_qcff->hgb);
        p_qcff->hgb = NULL;
    }
    /* Delete Smile Estimation Result Handle */
    if (p_qcff->hsm_result) {
        ret = FACEPROC_SM_DeleteResultHandle(p_qcff->hsm_result);
        p_qcff->hsm_result = NULL;
    }
    /* Delete Smile Estimation Handle */
    if (p_qcff->hsm) {
        ret = FACEPROC_SM_DeleteHandle(p_qcff->hsm);
        p_qcff->hsm = NULL;
    }
    /* Delete Parts Detection Result Handle */
    if (p_qcff->hpt_result) {
        ret = FACEPROC_PT_DeleteResultHandle(p_qcff->hpt_result);
        p_qcff->hpt_result = NULL;
    }
    /* Delete Parts Detection Handle */
    if (p_qcff->hpt) {
        ret = FACEPROC_PT_DeleteHandle(p_qcff->hpt);
        p_qcff->hpt = NULL;
    }
    /* Delete Detection Result handle */
    if (p_qcff->hdt_result) {
        ret = FACEPROC_DeleteDtResult(p_qcff->hdt_result);
        p_qcff->hdt_result = NULL;
    }
    /* Delete Detection Handle */
    if (p_qcff->hdt) {
        ret = FACEPROC_DeleteDetection(p_qcff->hdt);
        p_qcff->hdt = NULL;
    }
    /* Delete local frame buffer */
    if (p_qcff->p_local_frame) {
        free(p_qcff->p_local_frame);
        p_qcff->p_local_frame = NULL;
    }

    free((void*) p_qcff);
    *p_handle = NULL;
    QCFF_LOG("qcff_destroy");
    return QCFF_RET_SUCCESS;
}

/************************************************************************
 * Main exposed wrapper functions below
 ***********************************************************************/
/* Helper functions */
static int qcff_config_dt(qcff_t *p_qcff, qcff_config_t *p_cfg) {
    UINT32 an_still_angle[POSE_TYPE_COUNT];
    UINT8 major, minor;
    RECT nil_edge = { -1, -1, -1, -1 };
    int rc = QCFF_RET_SUCCESS;

//an_still_angle[POSE_FRONT] = p_qcff->frontal_rot;
//an_still_angle[POSE_HALF_PROFILE] = p_qcff->half_profile_rot;
//an_still_angle[POSE_PROFILE] = p_qcff->profile_rot;

    an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_1 | ANGLE_2 | ANGLE_3 | ANGLE_4
            | ANGLE_8 | ANGLE_9 | ANGLE_10 | ANGLE_11 | ANGLE_5 | ANGLE_6
            | ANGLE_7;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_11 | ANGLE_0 | ANGLE_1;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;

    /* Create Face-Engine FD handle */
    p_qcff->hdt = FACEPROC_CreateDetection();
    if (!p_qcff->hdt) {
        QCFF_LOG("FACEPROC_CreateDetection failed");
        return QCFF_RET_FAILURE;
    }

    /* Set best Face-Engine detection mode for video */
    if (p_qcff->mode == QCFF_MODE_STILL)
        rc = (int) FACEPROC_SetDtMode(p_qcff->hdt, DT_MODE_DEFAULT);
    else
        rc = (int) FACEPROC_SetDtMode(p_qcff->hdt, DT_MODE_MOTION3);

    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_SetDtMode failed %d", rc);
        return QCFF_RET_FAILURE;
    }

    /* Set search density */
    rc = (int) FACEPROC_SetDtStep(p_qcff->hdt, default_params.SEARCH_DENSITY,
            default_params.SEARCH_DENSITY);
    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_SetDtStep failed %d", rc);
        return QCFF_RET_FAILURE;
    }

    /* Set Omrom Detection Angles */
    rc = (int) FACEPROC_SetDtAngle(p_qcff->hdt, an_still_angle,
            ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_SetDtAngle failed %d", rc);
        return QCFF_RET_FAILURE;
    }

    rc = (int) FACEPROC_SetDtDirectionMask(p_qcff->hdt, FALSE);
    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_SetDtDirectionMask failed %d", rc);
        return QCFF_RET_FAILURE;
    }

    /* Minimum face size to be detected should be at most half the
     height of the input frame */
    if (default_params.MIN_FACE_SIZE > p_cfg->height / 2) {
        QCFF_LOG("default_params.MIN_FACE_SIZE > p_cfg->height / 2");
        return QCFF_RET_INVALID_PARM;
    }

    /* Set the max and min face size for detection */
    /* Maximum face size is set to the minimum of frame width and height */
    rc = (int) FACEPROC_SetDtFaceSizeRange(p_qcff->hdt,
            default_params.MIN_FACE_SIZE,
            p_cfg->width > p_cfg->height ?
                    p_cfg->height / p_cfg->downscale_factor :
                    p_cfg->width / p_cfg->downscale_factor);
    if (rc != FACEPROC_NORMAL) {
        QCFF_LOG("FACEPROC_SetDtFaceSizeRange failed %d", rc);
        return QCFF_RET_FAILURE;
    }

    /* Create Face-Engine result handle */
    p_qcff->hdt_result = FACEPROC_CreateDtResult(
            default_params.MAX_FACE_TO_DETECT,
            (default_params.MAX_FACE_TO_DETECT / 2));
    if (!(p_qcff->hdt_result)) {
        QCFF_LOG("FACEPROC_CreateDtResult failed");
        return QCFF_RET_FAILURE;
    }
    return QCFF_RET_SUCCESS;
}

static int qcff_config_pt(qcff_t *p_qcff) {
    p_qcff->hpt = FACEPROC_PT_CreateHandle();
    if (!p_qcff->hpt) {
        QCFF_LOG("FACEPROC_PT_CreateHandle failed");
        return QCFF_RET_FAILURE;
    }

    p_qcff->hpt_result = FACEPROC_PT_CreateResultHandle();
    if (!p_qcff->hpt_result) {
        QCFF_LOG("FACEPROC_PT_CreateResultHandle failed");
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

static int qcff_config_sm(qcff_t *p_qcff) {
    p_qcff->hsm = FACEPROC_SM_CreateHandle();
    if (!p_qcff->hsm) {
        QCFF_LOG("FACEPROC_SM_CreateHandle failed");
        return QCFF_RET_FAILURE;
    }

    p_qcff->hsm_result = FACEPROC_SM_CreateResultHandle();
    if (!p_qcff->hsm_result) {
        QCFF_LOG("FACEPROC_SM_CreateResultHandle failed");
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

static int qcff_config_gb(qcff_t *p_qcff) {
    p_qcff->hgb = FACEPROC_GB_CreateHandle();
    if (!p_qcff->hgb) {
        QCFF_LOG("FACEPROC_GB_CreateHandle failed");
        return QCFF_RET_FAILURE;
    }

    p_qcff->hgb_result = FACEPROC_GB_CreateResultHandle();
    if (!p_qcff->hgb_result) {
        QCFF_LOG("FACEPROC_SM_CreateResultHandle failed");
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

static int qcff_config_fr(qcff_t *p_qcff) {
    p_qcff->hfr = FACEPROC_FR_CreateFeatureHandle();
    if (!p_qcff->hfr) {
        QCFF_LOG("FACEPROC_FR_CreateFeatureHandle failed");
        return QCFF_RET_FAILURE;
    }
    p_qcff->hal = FACEPROC_FR_CreateAlbumHandle(
            default_params.MAX_REGISTERED_USERS,
            default_params.MAX_DATA_PER_USER);
    if (!p_qcff->hal) {
        QCFF_LOG("FACEPROC_FR_CreateAlbumHandle failed");
        return QCFF_RET_FAILURE;
    }
    p_qcff->num_registered_users = 0;
    return QCFF_RET_SUCCESS;
}

static int qcff_config_ct(qcff_t *p_qcff) {
    p_qcff->hct = FACEPROC_CT_CreateHandle();
    if (!p_qcff->hct) {
        QCFF_LOG("FACEPROC_CT_CreateHandle failed");
        return QCFF_RET_FAILURE;
    }

    p_qcff->hct_result = FACEPROC_CT_CreateResultHandle();
    if (!p_qcff->hct_result) {
        QCFF_LOG("FACEPROC_CT_CreateResultHandle failed");
        return QCFF_RET_FAILURE;
    }

    return QCFF_RET_SUCCESS;
}

static void qcff_translate_face_info_to_rect(FACEINFO *p_info,
        qcff_face_rect_t *p_rect) {
    int32_t left, right, top, bottom;

    p_rect->top_left.x = p_info->ptLeftTop.x;
    p_rect->top_left.y = p_info->ptLeftTop.y;
    p_rect->top_right.x = p_info->ptRightTop.x;
    p_rect->top_right.y = p_info->ptRightTop.y;
    p_rect->bottom_left.x = p_info->ptLeftBottom.x;
    p_rect->bottom_left.y = p_info->ptLeftBottom.y;
    p_rect->bottom_right.x = p_info->ptRightBottom.x;
    p_rect->bottom_right.y = p_info->ptRightBottom.y;

    left = MIN4(p_rect->top_left.x, p_rect->top_right.x,
            p_rect->bottom_left.x, p_rect->bottom_right.x);
    top = MIN4(p_rect->top_left.y, p_rect->top_right.y,
            p_rect->bottom_left.y, p_rect->bottom_right.y);
    right = MAX4(p_rect->top_left.x, p_rect->top_right.x,
            p_rect->bottom_left.x, p_rect->bottom_right.x);
    bottom = MAX4(p_rect->top_left.y, p_rect->top_right.y,
            p_rect->bottom_left.y, p_rect->bottom_right.y);

    p_rect->bounding_box.x = left;
    p_rect->bounding_box.y = top;
    p_rect->bounding_box.dx = right - left;
    p_rect->bounding_box.dy = bottom - top;
}

static int qcff_extract_feature(qcff_t *p_qcff, uint32_t face_index,
        HFEATURE hfr) {
    /* Set face location to parts detection handle */
    if (FACEPROC_NORMAL
            != FACEPROC_PT_SetPositionFromHandle(p_qcff->hpt,
                    p_qcff->hdt_result, face_index))
        return QCFF_RET_FAILURE;

    /* Do parts detection */
    if (FACEPROC_NORMAL
            != FACEPROC_PT_DetectPoint(p_qcff->hpt,
                    (RAWIMAGE *) p_qcff->p_local_frame,
                    p_qcff->frame_width / p_qcff->downscale_factor,
                    p_qcff->frame_height / p_qcff->downscale_factor,
                    p_qcff->hpt_result))
        return QCFF_RET_FAILURE;

    /* Extract feature */
    if (FACEPROC_NORMAL
            != FACEPROC_FR_ExtractFeatureFromPtHdl(hfr,
                    (RAWIMAGE*) p_qcff->p_local_frame,
                    p_qcff->frame_width / p_qcff->downscale_factor,
                    p_qcff->frame_height / p_qcff->downscale_factor,
                    p_qcff->hpt_result))
        return QCFF_RET_FAILURE;

    return QCFF_RET_SUCCESS;
}

/*
 * Date of addition: Sept 17, 2013
 * To keep the Favcial Recog. Confidence threshold customisable.
 */
int qcff_setThreshold(int32_t threshold) {
    if (threshold < 0 || threshold > 100) {
        return QCFF_RET_FAILURE;
    } else {
        default_params.FR_THRESHOLD = threshold * 10;
        return QCFF_RET_SUCCESS;
    }
}
//7th May eye detection commented
