/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    qcff_jni.c
 *
 */

#include <stdlib.h>
#include <jni.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

#include "qcff_jni.h"

#define NUM_FACES_SUPPORTED 64                  //Changd from 20, should be configurable later
#define LOG(msg)   __android_log_print(ANDROID_LOG_DEBUG, "QCFF", msg);
#define QCFF_LOG(fmt, args...)     __android_log_print(ANDROID_LOG_DEBUG, "QCFF", fmt, ##args)

uint32_t face_indices[NUM_FACES_SUPPORTED];

funcs_t gLib = {0};


jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_initialize( JNIEnv* env,
                                                    jobject this )
{
        int i;
        for (i = 0; i < NUM_FACES_SUPPORTED; i++)
                face_indices[i] = i;

    if (gLib.library == NULL)
    {
        *(void**)&gLib.qcff_get_version       = &qcff_get_version;
        *(void**)&gLib.qcff_create            = &qcff_create;
        *(void**)&gLib.qcff_config            = &qcff_config;
        *(void**)&gLib.qcff_set_detect_rot    = &qcff_set_detect_rot;
        *(void**)&gLib.qcff_set_frame         = &qcff_set_frame;
        *(void**)&gLib.qcff_set_mode          = &qcff_set_mode;
        *(void**)&gLib.qcff_get_num_faces     = &qcff_get_num_faces;
        *(void**)&gLib.qcff_get_rects         = &qcff_get_rects;
        *(void**)&gLib.qcff_get_parts         = &qcff_get_parts;
        *(void**)&gLib.qcff_get_parts_ex      = &qcff_get_parts_ex;
        *(void**)&gLib.qcff_get_smiles        = &qcff_get_smiles;
        *(void**)&gLib.qcff_get_eye_opens     = &qcff_get_eye_opens;
        *(void**)&gLib.qcff_get_gaze_degs     = &qcff_get_gaze_degs;
        //*(void**)&gLib.qcff_get_eye_detection = &qcff_get_eye_detection;  //eye detection
        *(void**)&gLib.qcff_get_directions    = &qcff_get_directions;
        *(void**)&gLib.qcff_get_complete_info = &qcff_get_complete_info;
        *(void**)&gLib.qcff_create_feature_cache  = &qcff_create_feature_cache;
        *(void**)&gLib.qcff_destroy_feature_cache = &qcff_destroy_feature_cache;
        *(void**)&gLib.qcff_reg_new_usr       = &qcff_reg_new_usr;
        *(void**)&gLib.qcff_reg_ex_usr        = &qcff_reg_ex_usr;
        *(void**)&gLib.qcff_remove_ex_usr     = &qcff_remove_ex_usr;
        *(void**)&gLib.qcff_identify_usr      = &qcff_identify_usr;
        *(void**)&gLib.qcff_get_num_ex_usrs   = &qcff_get_num_ex_usrs;
        *(void**)&gLib.qcff_reset_usr_data    = &qcff_reset_usr_data;
        *(void**)&gLib.qcff_get_usr_data_size = &qcff_get_usr_data_size;
        *(void**)&gLib.qcff_get_usr_data      = &qcff_get_usr_data;
        *(void**)&gLib.qcff_set_usr_data      = &qcff_set_usr_data;
        *(void**)&gLib.qcff_destroy           = &qcff_destroy;
        *(void**)&gLib.qcff_setThreshold      = &qcff_setThreshold;
    }
    return 0;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getMajorVersion( JNIEnv* env,
                                                            jobject this )
{
    uint32_t major, minor;
    gLib.qcff_get_version(&major, &minor);
    return (jint)major;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getMinorVersion( JNIEnv* env,
                                                            jobject this )
{
    uint32_t major, minor;
    gLib.qcff_get_version(&major, &minor);
    return (jint)minor;
}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_deinitialize( JNIEnv* env,
                                                      jobject this )
{

}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_create( JNIEnv* env,
                                                jobject this )
{
        qcff_handle_t handle;
        int rc = gLib.qcff_create(&handle);
        if (QCFF_FAILED(rc))
        {
                return 0;
        }
        return (jint)handle;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_config( JNIEnv* env,
                                                jobject this,
                                                jint handle,
                                                jint width,
                                                jint height,
                                                jint downscale_factor )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        if (h)
        {
            qcff_config_t config;
            config.width  = width;
            config.height = height;
            config.downscale_factor = downscale_factor;
            rc = gLib.qcff_config(h, &config);
            QCFF_LOG("QCCameraConfig returned %d",  (uint32_t)rc);
        }
        if (QCFF_RET_SUCCESS != rc)
        {
                return -1;
        }
        return 0;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_setDetectableRotation( JNIEnv* env,
                                                                  jobject this,
                                                                  jint handle,
                                                                  jint frontal,
                                                                  jint half_profile,
                                                                  jint profile )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        if (h)
        {
            rc = gLib.qcff_set_detect_rot(h, frontal, half_profile, profile);
        }
        if (QCFF_RET_SUCCESS != rc)
        {
                return -1;
        }
        return 0;
}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_setFrame( JNIEnv* env,
                                                  jobject this,
                                                  jint handle,
                                                  jbyteArray frame_array)
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_SUCCESS;
        if (h)
        {
            jbyte* frame;
            jboolean is_copy;

            frame = (*env)->GetByteArrayElements(env, frame_array, &is_copy);
            rc = gLib.qcff_set_frame(h, frame);
            /*if(stillFrameSizeChanged)
            {
                LOG("Called twice");
                rc = gLib.qcff_set_frame(h, frame);
            }*/
            QCFF_LOG("SetFrame returned %d",  (uint32_t)rc);
            (*env)->ReleaseByteArrayElements(env, frame_array, frame, JNI_ABORT);
        }

}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_setMode( JNIEnv* env,
                                                    jobject this,
                                                    jint handle,
                                                    jint mode)
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        if (h)
        {
            rc = gLib.qcff_set_mode(h, (qcff_mode_t)mode);
        }
        QCFF_LOG("returned %d",  (uint32_t)rc);
}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_setEngine( JNIEnv* env,
                                                      jobject this,
                                                      jint handle,
                                                      jint engine)
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        if (h)
        {
            rc = gLib.qcff_set_engine(h, (qcff_engine_t)engine);
        }
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getNumFaces( JNIEnv* env,
                                                     jobject this,
                                                     jint handle )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_SUCCESS;
        uint32_t num_faces;
        if (h)
        {
                rc = gLib.qcff_get_num_faces(h, &num_faces);
        }
        return num_faces;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getRects( JNIEnv* env,
                                                  jobject this,
                                                  jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS;
    uint32_t i;
    qcff_face_rect_t rects[NUM_FACES_SUPPORTED];
    uint32_t num_returned;
    jboolean is_copy;

    if (h)
    {
        rc = gLib.qcff_get_rects(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, rects);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, num_returned * 4);
        jint pArray[NUM_FACES_SUPPORTED * 4];

        for (i = 0; i < num_returned; i++)
        {
                pArray[i*4]   = rects[i].bounding_box.x;
                pArray[i*4+1] = rects[i].bounding_box.y;
                pArray[i*4+2] = rects[i].bounding_box.dx;
                pArray[i*4+3] = rects[i].bounding_box.dy;
        }
        (*env)->SetIntArrayRegion(env, newArray, 0, 4*num_returned, pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getParts( JNIEnv* env,
                                                     jobject this,
                                                     jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS, j;
    uint32_t i;
    qcff_face_parts_t parts[NUM_FACES_SUPPORTED];
    uint32_t num_returned;
    jboolean is_copy;

    if (h)
    {
        rc = gLib.qcff_get_parts(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, parts);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, num_returned * sizeof(qcff_face_parts_t) / sizeof(int));
        jint pArray[NUM_FACES_SUPPORTED * sizeof(qcff_face_parts_t)/sizeof(int)];

        for (i = 0; i < num_returned; i++)
        {
                for (j = 0; j < QCFF_PARTS_MAX; j++)
                {
                        pArray[i * QCFF_PARTS_MAX * 2 + j * 2]     = parts[i].parts[j].x;
                        pArray[i * QCFF_PARTS_MAX * 2 + j * 2 + 1] = parts[i].parts[j].y;
                }
        }
        (*env)->SetIntArrayRegion(env, newArray, 0, num_returned * sizeof(qcff_face_parts_t) / sizeof(int), pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getPartsEx( JNIEnv* env,
                                                       jobject this,
                                                       jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS, j;
    uint32_t i;
    qcff_face_parts_ex_t parts[NUM_FACES_SUPPORTED];
    uint32_t num_returned;
    jboolean is_copy;

    if (h)
    {
        rc = gLib.qcff_get_parts_ex(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, parts);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, num_returned * sizeof(qcff_face_parts_ex_t) / sizeof(int));
        jint pArray[NUM_FACES_SUPPORTED * sizeof(qcff_face_parts_ex_t)/sizeof(int)];

        for (i = 0; i < num_returned; i++)
        {
                for (j = 0; j < QCFF_PARTS_EX_MAX; j++)
                {
                        pArray[i * QCFF_PARTS_EX_MAX * 2 + j * 2]     = parts[i].parts[j].x;
                        pArray[i * QCFF_PARTS_EX_MAX * 2 + j * 2 + 1] = parts[i].parts[j].y;
                }
        }
        (*env)->SetIntArrayRegion(env, newArray, 0, num_returned * sizeof(qcff_face_parts_ex_t) / sizeof(int), pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getSmileDegrees( JNIEnv* env,
                                                            jobject this,
                                                            jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS;
    uint32_t i;
    uint32_t smiles[NUM_FACES_SUPPORTED];
    uint32_t num_returned;
    jboolean is_copy;

    if (h)
    {
        rc = gLib.qcff_get_smiles(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, smiles);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, num_returned);
        jint pArray[NUM_FACES_SUPPORTED];

        for (i = 0; i < num_returned; i++)
                pArray[i]   = smiles[i];

        (*env)->SetIntArrayRegion(env, newArray, 0, num_returned, pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getEyeBlinks( JNIEnv* env,
                                                         jobject this,
                                                         jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS;
    uint32_t i;
    qcff_eye_open_deg_t eye_opens[NUM_FACES_SUPPORTED];
    uint32_t num_returned;

    if (h)
    {
        rc = gLib.qcff_get_eye_opens(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, eye_opens);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, 2 * num_returned);
        jint pArray[NUM_FACES_SUPPORTED * 2];

        for (i = 0; i < num_returned; i++)
        {
                pArray[2*i]   = eye_opens[i].left;
                pArray[2*i+1] = eye_opens[i].right;
        }

        (*env)->SetIntArrayRegion(env, newArray, 0, 2 * num_returned, pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getEyeGazes( JNIEnv* env,
                                                        jobject this,
                                                        jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS;
    uint32_t i;
    qcff_gaze_deg_t gazes[NUM_FACES_SUPPORTED];
    uint32_t num_returned;

    if (h)
    {
        rc = gLib.qcff_get_gaze_degs(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, gazes);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, 2 * num_returned);
        jint pArray[NUM_FACES_SUPPORTED*2];

        for (i = 0; i < num_returned; i++)
        {
                pArray[2*i]   = gazes[i].left_right;
                pArray[2*i+1] = gazes[i].up_down;
        }

        (*env)->SetIntArrayRegion(env, newArray, 0, 2 * num_returned, pArray);
        return newArray;
    }

    return NULL;
}

/*jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getEyeDetection( JNIEnv* env,
                                                            jobject this,
                                                            jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS, i;
    qcff_eye_t results[3];
    uint32_t num_returned;

    if (h)
    {
        rc = gLib.qcff_get_eye_detection(h, 2, &num_returned, results);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, 7 * num_returned);
        jint pArray[7*2];

        for (i = 0; i < num_returned; i++)
        {
                pArray[7*i]   = results[i].confidence;
                pArray[7*i+1] = results[i].rect.x;
                pArray[7*i+2] = results[i].rect.y;
                pArray[7*i+3] = results[i].rect.dx;
                pArray[7*i+4] = results[i].rect.dy;
                pArray[7*i+5] = results[i].roll;
                pArray[7*i+6] = results[i].yaw;
        }

        (*env)->SetIntArrayRegion(env, newArray, 0, 7 * num_returned, pArray);
        return newArray;
    }

    return NULL;
}*/ //eye detection

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getFaceDirections( JNIEnv* env,
                                                              jobject this,
                                                              jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS;
    uint32_t i;
    qcff_face_dir_t dirs[NUM_FACES_SUPPORTED];
    uint32_t num_returned;

    if (h)
    {
        rc = gLib.qcff_get_directions(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, dirs);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, 3 * num_returned);
        jint pArray[NUM_FACES_SUPPORTED*3];

        for (i = 0; i < num_returned; i++)
        {
                pArray[3*i]   = dirs[i].up_down_in_degree;
                pArray[3*i+1] = dirs[i].left_right_in_degree;
                pArray[3*i+2] = dirs[i].roll_in_degree;
        }

        (*env)->SetIntArrayRegion(env, newArray, 0, 3 * num_returned, pArray);
        return newArray;
    }

    return NULL;
}

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_getCompleteInfos( JNIEnv* env,
                                                             jobject this,
                                                             jint handle,
                                                             jboolean get_rects,
                                                             jboolean get_parts,
                                                             jboolean get_parts_ex,
                                                             jboolean get_dirs,
                                                             jboolean get_smiles,
                                                             jboolean get_gazes,
                                                             jboolean get_eye_opens)
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_SUCCESS, j;
    uint32_t i;
    qcff_complete_face_info_t cinfo;
    qcff_face_rect_t rects[NUM_FACES_SUPPORTED];
    qcff_face_parts_t parts[NUM_FACES_SUPPORTED];
    qcff_face_parts_ex_t parts_ex[NUM_FACES_SUPPORTED];
    qcff_face_dir_t dirs[NUM_FACES_SUPPORTED];
    qcff_eye_open_deg_t eye_opens[NUM_FACES_SUPPORTED];
    uint32_t smiles[NUM_FACES_SUPPORTED];
    qcff_gaze_deg_t gazes[NUM_FACES_SUPPORTED];

    uint32_t num_elements = 0;
    uint32_t num_returned;

    if (h)
    {
        cinfo.p_rects = (get_rects) ? rects : NULL;
        cinfo.p_parts = (get_parts) ? parts : NULL;
        cinfo.p_parts_ex = (get_parts_ex) ? parts_ex : NULL;
        cinfo.p_directions = (get_dirs) ? dirs : NULL;
        cinfo.p_smile_degrees = (get_smiles) ? smiles : NULL;
        cinfo.p_eye_open_degrees = (get_eye_opens) ? eye_opens : NULL;
        cinfo.p_gaze_degrees = (get_gazes) ? gazes : NULL;

        num_elements += (get_rects) ? sizeof(qcff_face_rect_t) / sizeof(int) : 0;
        num_elements += (get_parts) ? sizeof(qcff_face_parts_t) / sizeof(int) : 0;
        num_elements += (get_parts_ex) ? sizeof(qcff_face_parts_ex_t) / sizeof(int) : 0;
        num_elements += (get_dirs) ? sizeof(qcff_face_dir_t) / sizeof(int) : 0;
        num_elements += (get_smiles) ? 1 : 0;
        num_elements += (get_eye_opens) ? sizeof(qcff_eye_open_deg_t) / sizeof(int) : 0;
        num_elements += (get_gazes) ? sizeof(qcff_gaze_deg_t) / sizeof(int) : 0;

        rc = gLib.qcff_get_complete_info(h, NUM_FACES_SUPPORTED, face_indices, &num_returned, &cinfo);
    }
    if (QCFF_RET_SUCCESS == rc && num_returned > 0)
    {
        jintArray newArray = (*env)->NewIntArray(env, num_elements * num_returned);
        jint pArray[NUM_FACES_SUPPORTED*num_elements];
        jint *pDst = pArray;

        for (i = 0; i < num_returned; i++)
        {
                if (get_rects)
                {
                                *pDst++ = rects[i].bounding_box.x;
                                *pDst++ = rects[i].bounding_box.y;
                                *pDst++ = rects[i].bounding_box.dx;
                                *pDst++ = rects[i].bounding_box.dy;
                }
                if (get_parts)
                {
                        for (j = 0; j < QCFF_PARTS_MAX; j++)
                        {
                            *pDst++ = parts[i].parts[j].x;
                            *pDst++ = parts[i].parts[j].y;
                        }
                }
                if (get_parts_ex)
                {
                        for (j = 0; j < QCFF_PARTS_EX_MAX; j++)
                                {
                                *pDst++ = parts_ex[i].parts[j].x;
                                *pDst++ = parts_ex[i].parts[j].y;
                                }
                }

                if (get_dirs)
                {
                        *pDst++ = dirs[i].up_down_in_degree;
                                *pDst++ = dirs[i].left_right_in_degree;
                                *pDst++ = dirs[i].roll_in_degree;
                }

                if (get_smiles)
                {
                        *pDst++ = smiles[i];
                }

                if (get_gazes)
                {
                        *pDst++ = gazes[i].left_right;
                *pDst++ = gazes[i].up_down;
                }

                if (get_eye_opens)
                {
                        *pDst++ = eye_opens[i].left;
                *pDst++ = eye_opens[i].right;
                }
        }

        (*env)->SetIntArrayRegion(env, newArray, 0, num_elements * num_returned, pArray);
        return newArray;
    }
    else{
        QCFF_LOG("Complete info returns %d n  face count %d", rc, num_returned);
    }

    return NULL;
}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_destroy( JNIEnv* env,
                                                                                                 jobject this,
                                                                                                 jint handle )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        if (h)
        {
            gLib.qcff_destroy(&h);
        }
}


jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getPrimaryFaceId( JNIEnv* env,
                                                             jobject this,
                                                             jint handle )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        jint face_id = -1;
        uint32_t num_faces = 0;

        if (h)
        {
                rc = gLib.qcff_get_num_faces(h, &num_faces);
                if (QCFF_SUCCEEDED(rc))
                {
                    if (num_faces > 1)
                        face_id = 0;
                    else
                    {
                            qcff_face_rect_t* rects = (qcff_face_rect_t*)malloc(num_faces * sizeof(qcff_face_rect_t));
                            if (rects)
                            {
                                uint32_t* indices = (uint32_t *)malloc(num_faces * sizeof(uint32_t));
                                if (indices)
                                {
                                    uint32_t i;
                                    uint32_t num_faces_returned;
                                    for (i = 0; i < num_faces; i++)
                                        indices[i] = i;
                                        rc = gLib.qcff_get_rects(h, num_faces, indices, &num_faces_returned, rects);
                                        if (QCFF_SUCCEEDED(rc))
                                        {
                                            uint32_t largest = 0;
                                            for (i = 0; i < num_faces_returned; i++)
                                            {
                                                uint32_t face_size = rects[i].bounding_box.dx * rects[i].bounding_box.dy;
                                                if (face_size > largest)
                                                {
                                                    largest = face_size;
                                                    face_id = i;
                                                }
                                            }
                                        }
                                        free(indices);
                                        }
                                        free(rects);
                                }
                        }
                }
        }

        return face_id;
}

/*jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_identifyFace( JNIEnv* env,
                                                         jobject this,
                                                         jint handle,
                                                         jint face_idx )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_FAILURE;
    uint32_t i;

    if (h)
    {
        jintArray newArray = (*env)->NewIntArray(env, 2);
        int pArray[] = {-1, -1};

        rc = gLib.qcff_identify_usr(h, face_idx, pArray, pArray+1);
        if (QCFF_RET_NO_MATCH == rc)
        {
                pArray[0] = -1;
                pArray[1] = 20;
        }
        (*env)->SetIntArrayRegion(env, newArray, 0, 2, pArray);

        if (QCFF_RET_FAILURE != rc)
                return newArray;
    }

    return NULL;
}*/

jintArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_identifyPerson( JNIEnv* env,
                                                         jobject this,
                                                         jint handle,
                                                         jint face_idx )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        uint32_t i;

        if (h)
        {
                jintArray newArray = (*env)->NewIntArray(env, 4);
                int pArray[4];

                rc = gLib.qcff_identify_usr(h, face_idx, pArray, pArray+2);
                if (QCFF_RET_NO_MATCH == rc)
                {
                                pArray[0] = -1;
                                pArray[1] = 20;
                }
                (*env)->SetIntArrayRegion(env, newArray, 0, 4, pArray);

                if (QCFF_RET_FAILURE != rc)
                                return newArray;
        }
    return NULL;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getFaceFeature( JNIEnv* env,
                                                           jobject this,
                                                           jint handle,
                                                           jint face_idx )
{
    qcff_face_feature_t feature;
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_FAILURE;

    if (h)
    {
        rc = gLib.qcff_create_feature_cache(h, face_idx, &feature);
    }
    if (QCFF_RET_SUCCESS != rc)
    {
        return -1;
    }
    return (jint)feature;
}

void
Java_com_qti_elements_sdk_fpr_FacialProcessing_deleteFaceFeature( JNIEnv* env,
                                                              jobject this,
                                                              jint feature )
{
        LOG("deleteFaceFeature!!");
    gLib.qcff_destroy_feature_cache((qcff_face_feature_t)feature);
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_addPerson( JNIEnv* env,
                                                            jobject this,
                                                            jint handle,
                                                            jint feature )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        uint32_t new_user_id;

        if (h)
        {
                rc = gLib.qcff_reg_new_usr(h, (qcff_face_feature_t)feature, &new_user_id);
        }
        if (QCFF_RET_SUCCESS != rc)
        {
                return -1;
        }
        return (jint)new_user_id;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_updatePerson( JNIEnv* env,
                                                                 jobject this,
                                                                 jint handle,
                                                                 jint feature,
                                                                 jint user_id )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;

        if (h)
        {
                rc = gLib.qcff_reg_ex_usr(h, (qcff_face_feature_t)feature, user_id);
        }
        if (QCFF_RET_SUCCESS != rc)
        {
                return -1;
        }
        return QCFF_RET_SUCCESS;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_removePerson( JNIEnv* env,
                                                               jobject this,
                                                               jint handle,
                                                               jint user_id )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_FAILURE;

    if (h)
    {
        rc = gLib.qcff_remove_ex_usr(h, user_id);
    }
    if (QCFF_RET_SUCCESS != rc)
    {
        return -1;
    }
    return QCFF_RET_SUCCESS;
}

jbyteArray
Java_com_qti_elements_sdk_fpr_FacialProcessing_serializeAlbum( JNIEnv* env,
                                                        jobject this,
                                                        jint handle )
{
    qcff_handle_t h = (qcff_handle_t)handle;
    int rc = QCFF_RET_FAILURE;

        if (h)
        {
                uint32_t size;
                jbyteArray newArray;

                rc = gLib.qcff_get_usr_data_size(h, &size);
                if (QCFF_RET_SUCCESS == rc)
                {
                        jbyte *pArray = (jbyte *) malloc(size);
                        if (pArray)
                        {
                                rc = gLib.qcff_get_usr_data(h, pArray, size);
                                if (QCFF_RET_SUCCESS == rc)
                                {
                                        newArray = (*env)->NewByteArray(env, size);
                                        (*env)->SetByteArrayRegion(env, newArray, 0, size, pArray);
                                        free(pArray);
                                        return newArray;
                                }
                        }
                }
        }
    return NULL;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_deserializeAlbum( JNIEnv* env,
                                                        jobject this,
                                                        jint handle,
                                                        jint buf_size,
                                                        jbyteArray data )
{
        qcff_handle_t h = (qcff_handle_t)handle;
        int rc = QCFF_RET_FAILURE;
        if (h)
        {
           uint8_t* p_user_data;
           jboolean is_copy;

            p_user_data = (*env)->GetByteArrayElements(env, data, &is_copy);
            rc = gLib.qcff_set_usr_data(h, buf_size, p_user_data);
            (*env)->ReleaseByteArrayElements(env, data, p_user_data, JNI_ABORT);
            if (QCFF_RET_SUCCESS == rc)
                return QCFF_RET_SUCCESS;
        }
    return -1;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_resetAlbum( JNIEnv* env,
                                                          jobject this,
                                                          jint handle)
{
        qcff_handle_t h = (qcff_handle_t)handle;
        jint rc = QCFF_RET_FAILURE;
        if (h)
        {
            rc = gLib.qcff_reset_usr_data(h);
        }
        if(rc != QCFF_RET_SUCCESS)
                return -1;
        else
                return QCFF_RET_SUCCESS;
}

jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getVersion( JNIEnv*  env,
                                                    jobject  this )
{
        return 2;
}


jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_setConfidenceValue( JNIEnv*  env,
                                                    jobject  this, jint threshold )
{
        if(threshold < 0 || threshold > 100)
        {
                return QCFF_RET_FAILURE;
        }
        else
        {
                jint rc = QCFF_RET_FAILURE;
                rc = gLib.qcff_setThreshold(threshold);

                if(rc != QCFF_RET_SUCCESS)
                        return -1;
                else
                        return QCFF_RET_SUCCESS;
        }
}

// added for testing
jint
Java_com_qti_elements_sdk_fpr_FacialProcessing_getNumberOfPeople( JNIEnv* env,
                                                                   jobject this,
                                                                   jint handle)
{
    qcff_handle_t h = (qcff_handle_t)handle;
    jint rc = QCFF_RET_FAILURE;
    jint count = -1;
    if (h)
    {
        rc = gLib.qcff_get_num_ex_usrs (h, &count);
    }
    if(rc != QCFF_RET_SUCCESS)
        return -1;
    else
        return count;
}
