/* =========================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    FaceEngineConstants.java
 *
 */

package com.qti.elements.sdk.fpr;

public class FacialProcessingConstants {

        /**
         * Expect this result when the operation of updating a face to an existing
         * person in the album or setting setting the face recognition confidence
         * are successful
         */
        public static final int FP_SUCCESS = 0;
        /**
         * Expect this result when the operation of adding a new person to the the
         * album or updating a new face to an existing person fails because no face
         * is detected in the image/frame
         */
        public static final int FP_NO_FACE_DETECTED_ERROR = -1;
        /**
         * This result is returned when the facial processing engine has suffered a
         * catastrophic failure
         */
        public static final int FP_INTERNAL_ERROR = -2;
        /**
         * Expect this result when the operation of adding a new person fails
         * because a similar face already exists in the album
         */
        public static final int FP_FACE_EXIST_ERROR = -3;
        /**
         * Expect this result when the operation of setting the face recognition
         * confidence level fails because the input value is not between 0 and 100
         */
        public static final int FP_CONFIDENCE_OUT_OF_RANGE_ERROR = -4;
        /**
         * Expect this result when operation of getting person id or face
         * recognition confidence fails because the person is not yet added to the
         * recognition album
         */
        public static final int FP_PERSON_NOT_REGISTERED = -111;
        /**
         * Expect this result when the Enum value for the face data is not specified
         */
        public static final int FP_NOT_PROCESSED = -999;

}
