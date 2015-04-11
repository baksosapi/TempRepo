/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    FaceData.java
 *
 */
package com.qti.elements.sdk.fpr;

import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;

/**
 * Face objects are used to store the facial data elements returned by the
 * facial processing engine. These data elements include:
 * <ul>
 * <li>Gaze Point
 * <li>Horizontal Gaze Angle
 * <li>Vertical Gaze Angle
 * <li>Left Eye Blink
 * <li>Right Eye Blink
 * <li>Face Pitch
 * <li>Face Roll
 * <li>Face Yaw
 * <li>Smile Value
 * <li>Recognition Confidence Value
 * <li>Person ID
 * </ul>
 * Facial data elements can only be extracted via the exposed FaceData methods.
 * The following facial data coordinates are public and can be extracted directly
 * from the Face object:
 * <ul>
 * <li>leftEye
 * <li>rightEye
 * <li>mouth
 * <li>rect
 * </ul>
 * The constructor for this object is called from the FacialProcessing.getFaceData method.
 */
public class FaceData {
    private static final String TAG                  = "QCFace";

    private int                 smileDegree;
    private int                 lEyeBlink;
    private int                 rEyeBlink;
    private int                 eyeHorizontalGazeAngle;
    private int                 eyeVerticalGazeAngle;

    private int                 yaw;
    private int                 pitch;
    private int                 roll;

    /**
     * The X,Y coordinates of the center of the left eye in relation to the resolution
     * of the raw image data
     */
    public Point                leftEye;
    /**
     * The X,Y coordinates of the center of the right eye in relation to the resolution
     * of the raw image data
     */
    public Point                rightEye;
    /**
     * The X,Y coordinates of the center of the mouth in relation to the resolution
     * of the raw image data
     */
    public Point                mouth;
    /**
     * The X,Y coordinates of the rectangle bounding the detected face in relation
     * to the resolution of the raw image data
     */
    public Rect                 rect;

    private boolean             isMirrored           = false;
    private int                 rotationAngleDegrees = 0;

    private int[][]             sinCosValues         = {// sin,cos
                                                     { 1, 0 }, // 90
            { 0, -1 }, // 180
            { -1, 0 }                               // 270
                                                     };

    private int                 recognitionConfidenceValue;  //used only for face recognition, otherwise NOT_PROCESSED
    private int                 personId;      //if face is recognised, then will have a valid id, or else -1 or NOT_PROCESSED

    /*
     * Constructor with default access specifier, this will restrict instantiation from other packages.
     */
    FaceData(boolean isMirrored, int rotationAngle) {
        smileDegree = FacialProcessingConstants.FP_NOT_PROCESSED;
        lEyeBlink = FacialProcessingConstants.FP_NOT_PROCESSED;
        rEyeBlink = FacialProcessingConstants.FP_NOT_PROCESSED;
        eyeHorizontalGazeAngle = FacialProcessingConstants.FP_NOT_PROCESSED;
        eyeVerticalGazeAngle = FacialProcessingConstants.FP_NOT_PROCESSED;

        leftEye = null;
        rightEye = null;
        rect = null;
        mouth = null;

        yaw = pitch = roll = FacialProcessingConstants.FP_NOT_PROCESSED;
        this.isMirrored = isMirrored;
        this.rotationAngleDegrees = rotationAngle;

        personId = FacialProcessingConstants.FP_NOT_PROCESSED;
        recognitionConfidenceValue = FacialProcessingConstants.FP_NOT_PROCESSED;
    }


    protected void setSmileValue(int smile) {
        this.smileDegree = smile;
    }


    /**
     * Returns the smile value for the detected face.
     * The smile value range is 0 to 100.
     * The lower the value, the less likely it is that the face is smiling.
     * There is no negative value for frown detection.
     *
     * @return smileDegree The smileDegree variable is of type integer.
     */
    public int getSmileValue() {
        return smileDegree;
    }


    protected void setBlinkValues(int leftEyeBlink, int rightEyeBlink) {
        this.lEyeBlink = leftEyeBlink;
        this.rEyeBlink = rightEyeBlink;
    }


    /**
     * Returns the left eye blink value for the detected face.
     * The blink value range is 0 to 100.
     * The lower the value, the less likely it is that the eye is blinking.
     *
     * @return lEyeBlink The lEyeBlink variable is of type integer.
     */
    public int getLeftEyeBlink() {
        // return isMirrored ? rEyeBlink : lEyeBlink;
        return lEyeBlink;
    }


    /**
     * Returns the right eye blink value for the detected face.
     * The blink value range is 0 to 100.
     * The lower the value, the less likely it is that the eye is blinking.
     *
     * @return rEyeBlink The rEyeBlink variable is of type integer.
     */
    public int getRightEyeBlink() {
        // return isMirrored ? lEyeBlink : rEyeBlink;
        return rEyeBlink;
    }


    protected void setEyeGazeAngles(int hGazeAngle, int vGazeAngle) {
        this.eyeHorizontalGazeAngle = hGazeAngle;
        this.eyeVerticalGazeAngle = vGazeAngle;
    }


    /**
     * Returns the horizontal gaze angle value.
     * The value range is -30 to 30.
     * A negative value indicates the eyes are looking to the left of the camera.
     * A value of 0 indices the eyes are looking directly at the camera in the
     * horizontal plane.
     * A positive value indicates the eyes are looking to the right of the camera.
     *
     * @return eyeHorizontalGazeAngle The eyeHorizontalGazeAngle variable is of type integer.
     */
    public int getEyeHorizontalGazeAngle() {
        // return eyeHorizontalGazeAngle * ( isMirrored ? -1 : 1 );
        return eyeHorizontalGazeAngle;
    }


    /**
     * Returns the vertical gaze angle value.
     * The value range is -20 to 20.
     * A negative value indicates the eyes are looking below the camera.
     * A value of 0 indicates the eyes are looking directly at the camera in the
     * vertical plane.
     * A positive value indicates the eyes are looking above to the camera.
     *
     * @return eyeVerticalGazeAngle The eyeVerticalGazeAngle variable is of type integer.
     */
    public int getEyeVerticalGazeAngle() {
        return eyeVerticalGazeAngle;
    }


    /**
     * Returns the gaze point the eyes are looking toward in relation to the
     * position of the camera.
     * The value of the gaze point ranges from -1 to 1, expressed to the hundredth
     * decimal place.
     * The point values represent an X,Y coordinate where 0,0 represents the center
     * of the eye in a two dimensional plane.
     *
     * @return PointF Returns the X,Y gaze coordinate expressed as a floating point value
     */
    public PointF getEyeGazePoint() {
        if(eyeHorizontalGazeAngle == FacialProcessingConstants.FP_NOT_PROCESSED){
            return null;
        }
        if (eyeHorizontalGazeAngle > 90 || eyeHorizontalGazeAngle < -90 || eyeVerticalGazeAngle > 90 || eyeVerticalGazeAngle < -90) {
            FacialProcessing.Log.e(TAG, "Gaze angles out of range Hor: " + eyeHorizontalGazeAngle + " Ver: " + eyeVerticalGazeAngle);
            return null;
        }

        float gazeX = eyeHorizontalGazeAngle;
        float gazeY = eyeVerticalGazeAngle;

        // convert a degree value to radians as expected by math.sin and math.cos
        float xRadians = (float) (gazeX / 180.0 * Math.PI);
        float yRadians = (float) (gazeY / 180.0 * Math.PI);
        float xraw = (float) Math.sin(xRadians);
        float yraw = (float) -Math.sin(yRadians);

        // Get the radians
        float rawRadians = (float) Math.atan(yraw / xraw);
        if (xraw < 0) {
            rawRadians += Math.PI;
        }
        float length = (float) (Math.sqrt(xraw * xraw + yraw * yraw) / .6046);
        if (xraw == 0) {
            rawRadians = (float) Math.PI / 2;
            if (yraw < 0) {
                rawRadians += Math.PI;
            }
        } else {
            rawRadians = (float) Math.atan(yraw / xraw);
            if (xraw < 0) {
                rawRadians += Math.PI;
            }
        }
        PointF p = new PointF();
        p.x = (float) (Math.cos(rawRadians) * length);
        p.y = (float) (Math.sin(rawRadians) * length);
        return p;
    }


    protected void setYaw(int yaw) {
        this.yaw = yaw;
    }


    protected void setPitch(int pitch) {
        this.pitch = pitch;
    }


    protected void setRoll(int roll) {
        this.roll = roll;
    }


    /**
     * This API returns the value of how much a detected face is looking
     * left/right. Factors in the mirrored state of the camera but any use of
     * this value needs to factor in any display orientation rotation degrees of
     * the camera manually.
     *
     * @return value representing how much a face is looking left/right on scale
     *         of -180 to 179
     */
    public int getYaw() {
        // return yaw * (isMirrored ? -1 : 1);
        return yaw;
    }


    /**
     * This API returns the value of how much a detected face has been tilted.
     * Factors in the mirrored state of the camera but any use of this value
     * needs to factor in any display orientation rotation degrees of the camera
     * manually.
     *
     * @return value representing how much a face has been tilted on scale of
     *         -180 to 179
     */
    public int getRoll() {
        // return roll * (isMirrored ? -1 : 1);
        return roll;
    }


    /**
     * This API returns the value of how much a detected face is looking up/down
     *
     * @return value representing how much a face is looking up/down on scale of
     *         -180 to 179
     */
    public int getPitch() {
        return pitch;
    }


    protected void doRotationNMirroring(int prevWidth, int prevHeight) {
        // sequence is dicey
        if (true == isMirrored) {
            doMirroring(prevWidth);
        }
        if (0 != rotationAngleDegrees) {
            doRotation(prevWidth, prevHeight);
        }
    }


    private void doRotation(int prevWidth, int prevHeight) {

        // rotate all values clockwise
        Point temp = new Point(0, 0);

        int xCenter = prevWidth / 2;
        int yCenter = prevHeight / 2;

        int cos = 1;
        int sin = 0;
        int xDiff = 0;
        int yDiff = 0;
        switch (rotationAngleDegrees) {
        case 90:
            sin = sinCosValues[0][0];
            cos = sinCosValues[0][1];
            xDiff = 0;
            yDiff = prevHeight;
            xDiff = xDiff - xCenter;
            yDiff = yDiff - yCenter;
            temp.x = (int) ((xDiff * cos) - (yDiff * sin));
            temp.y = (int) ((xDiff * sin) + (yDiff * cos));
            xDiff = temp.x + xCenter;
            yDiff = temp.y + yCenter;
            break;
        case 180:
            sin = sinCosValues[1][0];
            cos = sinCosValues[1][1];
            break;
        case 270:
            sin = sinCosValues[2][0];
            cos = sinCosValues[2][1];
            xDiff = prevWidth - 1;
            yDiff = 0;
            xDiff = xDiff - xCenter;
            yDiff = yDiff - yCenter;
            temp.x = (int) ((xDiff * cos) - (yDiff * sin));
            temp.y = (int) ((xDiff * sin) + (yDiff * cos));
            xDiff = temp.x + xCenter;
            yDiff = temp.y + yCenter;
            break;
        }

        if(leftEye != null){
            // left eye
            leftEye.x = leftEye.x - xCenter;
            leftEye.y = leftEye.y - yCenter;
            temp.x = (int) ((leftEye.x * cos) - (leftEye.y * sin));
            temp.y = (int) ((leftEye.x * sin) + (leftEye.y * cos));
            leftEye.x = temp.x + xCenter - xDiff;
            leftEye.y = temp.y + yCenter - yDiff;


            // right eye
            rightEye.x = rightEye.x - xCenter;
            rightEye.y = rightEye.y - yCenter;
            temp.x = (int) ((rightEye.x * cos) - (rightEye.y * sin));
            temp.y = (int) ((rightEye.x * sin) + (rightEye.y * cos));
            rightEye.x = temp.x + xCenter - xDiff;
            rightEye.y = temp.y + yCenter - yDiff;

            // mouth
            mouth.x = mouth.x - xCenter;
            mouth.y = mouth.y - yCenter;
            temp.x = (int) ((mouth.x * cos) - (mouth.y * sin));
            temp.y = (int) ((mouth.x * sin) + (mouth.y * cos));
            mouth.x = temp.x + xCenter - xDiff;
            mouth.y = temp.y + yCenter - yDiff;
        }

        // rect
        int left = 0, top = 0;
        left = rect.left - xCenter;
        top = rect.top - yCenter;
        temp.x = (int) ((left * cos) - (top * sin));
        temp.y = (int) ((left * sin) + (top * cos));
        left = temp.x + xCenter - xDiff;
        top = temp.y + yCenter - yDiff;

        int right = 0, bottom = 0;
        right = rect.right - xCenter;
        bottom = rect.bottom - yCenter;
        temp.x = (int) ((right * cos) - (bottom * sin));
        temp.y = (int) ((right * sin) + (bottom * cos));
        right = temp.x + xCenter - xDiff;
        bottom = temp.y + yCenter - yDiff;

        if (left < right) {
            rect.left = left;
            rect.right = right;
        } else {
            rect.left = right;
            rect.right = left;
        }
        if (top < bottom) {
            rect.top = top;
            rect.bottom = bottom;
        } else {
            rect.top = bottom;
            rect.bottom = top;
        }

        //roll value correction
        switch(rotationAngleDegrees){
        case 90:
            roll += rotationAngleDegrees;
            if(roll > 180){
                int diff = roll - 180;
                roll = diff - 180;
            }
            break;

        case 180:
            if (roll <= 0) {
                roll += rotationAngleDegrees;
            } else {
                roll -= rotationAngleDegrees;
            }
            break;

        case 270:
            roll -= 90;
            if(roll < -179){
                int diff = -179 - roll;
                roll = 179 + diff;
            }
            break;
        }
        //end roll value

        //gaze angles
        switch(rotationAngleDegrees){
        case 90:
            int tempInt = -eyeHorizontalGazeAngle;
            eyeHorizontalGazeAngle = eyeVerticalGazeAngle;
            eyeVerticalGazeAngle = tempInt;
            break;

        case 180:
            eyeHorizontalGazeAngle = -eyeHorizontalGazeAngle;
            eyeVerticalGazeAngle = -eyeVerticalGazeAngle;
            break;

        case 270:
            tempInt = eyeHorizontalGazeAngle;
            eyeHorizontalGazeAngle = -eyeVerticalGazeAngle;
            eyeVerticalGazeAngle = tempInt;
            break;
        }
        //end gaze angles
    }


    private void doMirroring(int width) {
        // yaw, roll, horizontal gaze
        if(yaw != FacialProcessingConstants.FP_NOT_PROCESSED)
        {
            yaw = -yaw;
            roll = -roll;
        }
        if(eyeHorizontalGazeAngle != FacialProcessingConstants.FP_NOT_PROCESSED)
        {
            eyeHorizontalGazeAngle = -eyeHorizontalGazeAngle;
        }

        // mirror blink values
        if(lEyeBlink != FacialProcessingConstants.FP_NOT_PROCESSED)
        {
            int tempInt = lEyeBlink;
            lEyeBlink = rEyeBlink;
            rEyeBlink = tempInt;
        }

        // now flip point coordinates.. newx = width - oldx - 1
        if(leftEye != null){
            leftEye.x = width - leftEye.x - 1;
            rightEye.x = width - rightEye.x - 1;
            mouth.x = width - mouth.x - 1;
            {
                Point temp = leftEye;
                leftEye = rightEye;
                rightEye = temp;
            }
        }
        // now flip rect
        {
            int tempInt = width - rect.left - 1;
            rect.left = width - rect.right - 1;
            rect.right = tempInt;
        }
    }


    protected void normalizeCoordinates(float scaleX, float scaleY){
        if(leftEye != null){
            // Normalize Left Eye Co-ordinate
            leftEye.x =(int) ((float)leftEye.x*scaleX);
            leftEye.y =(int) ((float)leftEye.y*scaleY);

            // Normalize Right Eye Co-ordinate
            rightEye.x =(int) ((float)rightEye.x*scaleX);
            rightEye.y =(int) ((float)rightEye.y*scaleY);

            // Normalize mouth Co-ordinate
            mouth.x =(int) ((float)mouth.x*scaleX);
            mouth.y =(int) ((float)mouth.y*scaleY);
        }

        // Normalize rect Co-ordinate
        rect.left =(int) ((float)rect.left*scaleX);
        rect.top =(int)  ((float)rect.top*scaleY);
        rect.right =(int) ((float)rect.right*scaleX);
        rect.bottom =(int)  ((float)rect.bottom*scaleY);
    }

    /**
     * This API returns the personID for the face in the recognition album that matches the input face from the processed frame or bitmap.
     *
     * @return personID or FaceData.NOT_FOUND(-111) if there is no match
     *
     */
    public int getPersonId(){
        return personId;
    }


    /**
     * This API returns the value representing the confidence in the face input in the frame or bitmap matching a face in the recognition album.
     * The value will range from 0 to 100.  The lower the value, the less confident the software is that the faces match
     *
     * @return recognitionConfidenceValue or FaceData.NOT_FOUND(-111) if there is no match
     *
     */
    public int getRecognitionConfidence() {
        return recognitionConfidenceValue;
    }

    /*
     * This API sets the faceID that is returned from the native libraries.
     */
    protected void setPersonId(int personId){
        this.personId=personId;
    }

    /*
     * This API sets the confidence value that is returned from the native libraries.
     */
    protected void setRecognitionConfidence(int recognitionConfidenceValue){
        this.recognitionConfidenceValue= recognitionConfidenceValue;
    }
}