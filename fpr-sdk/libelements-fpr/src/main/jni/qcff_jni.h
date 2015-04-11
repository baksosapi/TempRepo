/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    qcff_jni.h
 *
 */
#include "qcff_native.h"

typedef struct
{
        void* library;
        int (*qcff_get_version)       (uint32_t *, uint32_t*);
    int (*qcff_create)            (qcff_handle_t *);
    int (*qcff_config)            (qcff_handle_t, qcff_config_t *);
    int (*qcff_set_detect_rot)    (qcff_handle_t, uint32_t, uint32_t, uint32_t);
    int (*qcff_set_frame)         (qcff_handle_t, uint8_t *);
    int (*qcff_set_mode)          (qcff_handle_t, qcff_mode_t);
    int (*qcff_set_engine)        (qcff_handle_t, qcff_engine_t);
    int (*qcff_get_num_faces)     (qcff_handle_t, uint32_t *);
    int (*qcff_get_rects)         (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_face_rect_t *);
    int (*qcff_get_parts)         (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_face_parts_t *);
    int (*qcff_get_parts_ex)      (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_face_parts_ex_t *);
    int (*qcff_get_smiles)        (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, uint32_t *);
    int (*qcff_get_eye_opens)     (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_eye_open_deg_t *);
    int (*qcff_get_gaze_degs)     (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_gaze_deg_t *);
    int (*qcff_get_directions)    (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_face_dir_t *);
//    int (*qcff_get_eye_detection) (qcff_handle_t, uint32_t, uint32_t *, qcff_eye_t *);    //eye detection
    int (*qcff_get_complete_info) (qcff_handle_t, uint32_t, uint32_t *, uint32_t *, qcff_complete_face_info_t *);
    int (*qcff_create_feature_cache)  (qcff_handle_t, uint32_t, qcff_face_feature_t *);
    int (*qcff_destroy_feature_cache) (qcff_face_feature_t);
    int (*qcff_reg_new_usr)           (qcff_handle_t, qcff_face_feature_t, uint32_t *);
    int (*qcff_reg_ex_usr)            (qcff_handle_t, qcff_face_feature_t, uint32_t);
    int (*qcff_remove_ex_usr)         (qcff_handle_t, uint32_t);
    int (*qcff_identify_usr)      (qcff_handle_t, uint32_t, uint32_t *, uint32_t *);
    int (*qcff_get_num_ex_usrs)   (qcff_handle_t, uint32_t *);
    int (*qcff_reset_usr_data)    (qcff_handle_t);
    int (*qcff_get_usr_data_size) (qcff_handle_t, uint32_t *);
    int (*qcff_get_usr_data)      (qcff_handle_t, uint8_t *, uint32_t);
    int (*qcff_set_usr_data)      (qcff_handle_t, uint32_t, uint8_t *);
    int (*qcff_destroy)           (qcff_handle_t *);
    int (*qcff_setThreshold)    (int32_t);

} funcs_t;

//7th May - Eye detection commented

