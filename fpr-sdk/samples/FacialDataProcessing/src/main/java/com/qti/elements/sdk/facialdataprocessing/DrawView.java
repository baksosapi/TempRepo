/* ======================================================================
 *  Copyright � 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 *  QTI Proprietary and Confidential.
 *  =====================================================================
 *
 *  Licensed by Qualcomm, Inc. under the Snapdragon SDK for Android license.
 *
 *  You may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    https://developer.qualcomm.com/snapdragon-sdk-license
 *
 *  This Qualcomm software is supplied to you by Qualcomm Inc. in consideration
 *  of your agreement to the licensing terms.  Your use, installation, modification
 *  or redistribution of this Qualcomm software constitutes acceptance of these terms.
 *  If you do not agree with these terms, please do not use, install, modify or
 *  redistribute this Qualcomm software.
 *
 *  Qualcomm grants you a personal, non-exclusive license, under Qualcomm's
 *  copyrights in this original Qualcomm software, to use, reproduce, modify
 *  and redistribute the Qualcomm Software, with or without modifications, in
 *  source and/or binary forms; provided that if you redistribute the Qualcomm
 *  Software in its entirety and without modifications, you must retain this
 *  notice and the following text and disclaimers in all such redistributions
 *  of the Qualcomm Software.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file:   DrawView.java
 *
 */


package com.qti.elements.sdk.facialdataprocessing;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.PorterDuff.Mode;
import android.graphics.Rect;
import android.hardware.Camera;
import android.view.SurfaceView;
import com.qti.elements.sdk.fpr.FaceData;

public class DrawView extends SurfaceView {

    public FaceData[] mFaceArray;
    boolean _inFrame;                       // Boolean to see if there is any faces in the frame
    int mSurfaceWidth;
    int mSurfaceHeight;
    int cameraPreviewWidth;
    int cameraPreviewHeight;
    boolean mLandScapeMode;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    private Paint leftEyeBrush = new Paint();
    private Paint rightEyeBrush = new Paint();
    private Paint mouthBrush = new Paint();
    private Paint rectBrush = new Paint();


    public DrawView(Context context, FaceData[] faceArray, boolean inFrame, int surfaceWidth, int surfaceHeight, Camera cameraObj, boolean landScapeMode) {
        super(context);

        setWillNotDraw(false);                                 // This call is necessary, or else the draw method will not be called.
        mFaceArray = faceArray;
        _inFrame = inFrame;
        mSurfaceWidth = surfaceWidth;
        mSurfaceHeight = surfaceHeight;
        mLandScapeMode = landScapeMode;
        if (cameraObj != null) {
            cameraPreviewWidth = cameraObj.getParameters().getPreviewSize().width;
            cameraPreviewHeight = cameraObj.getParameters().getPreviewSize().height;
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {

        if (_inFrame)                            // If the face detected is in frame.
        {
            for (int i = 0; i < mFaceArray.length; i++) {
                leftEyeBrush.setColor(Color.RED);
                canvas.drawCircle(mFaceArray[i].leftEye.x * scaleX, mFaceArray[i].leftEye.y * scaleY, 5f, leftEyeBrush);

                rightEyeBrush.setColor(Color.GREEN);
                canvas.drawCircle(mFaceArray[i].rightEye.x * scaleX, mFaceArray[i].rightEye.y * scaleY, 5f, rightEyeBrush);

                mouthBrush.setColor(Color.WHITE);
                canvas.drawCircle(mFaceArray[i].mouth.x * scaleX, mFaceArray[i].mouth.y * scaleY, 5f, mouthBrush);

                rectBrush.setColor(Color.YELLOW);
                rectBrush.setStyle(Paint.Style.STROKE);
                canvas.drawRect(mFaceArray[i].rect.left * scaleX, mFaceArray[i].rect.top * scaleY, mFaceArray[i].rect.right * scaleX, mFaceArray[i].rect.bottom * scaleY, rectBrush);
            }
        } else {
            canvas.drawColor(0, Mode.CLEAR);
        }
    }


}
