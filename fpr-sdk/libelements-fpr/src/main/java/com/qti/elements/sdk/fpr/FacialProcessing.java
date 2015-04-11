/* =========================================================================
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =========================================================================
 * @file    FacialProcessing.java
 *
 */
package com.qti.elements.sdk.fpr;

import java.util.ArrayList;
import java.util.EnumSet;

import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.Log;


/**
 * The FacialProcessing class is used to execute the facial feature scanning on an image as well as performing facial recognition queries.
 * <P>
 * The facial features scanning include:
 * <ul>
 * <li>Face Detection
 * <li>Blink Detection
 * <li>Smile Detection
 * <li>Gaze Tracking
 * <li>Face Orientation
 * </ul>
 * The facial processing features are exposed in a post processing model so that
 * any image, either a video stream or static image, may be processed and if faces
 * are detected, then face data may be extracted and acted upon.  Images must be
 * formatted as YUV or bitmap.
 * <P>
 * The processing framework may be configured by setting one of two modes to optimize
 * the image scanning process to reduce processing time or increase accuracy.
 * The modes are FP_MODE_STILL and FP_MODE_VIDEO.
 * <P>
 * FP_MODE_STILL is the most accurate scanning mode and is typically used for still
 * images and low frame rate video frames.
 * FP_MODE_VIDEO is the most optimized scanning mode and is typically used for processing
 * high frame rate video and camera preview frames. In this mode, the first frame
 * is completely scanned and any faces detected are then tracked in each subsequent
 * frame.  Then on every fifteenth frame, the entire image will be scanned
 * again and the process repeated until no further frames are input.  Face tracking
 * consumes two thirds fewer cycles to process than a full image scan.
 * <P>
 * Another way to reduce the facial processing overhead is to instruct the framework to
 * limit the data set you wish to extract from each face. This is done by passing an
 * EnumSet of all the facial features you want processed in the getFaceData() method.
 * By default, the framework will generate the complete data set for all faces by
 * simply calling the getFaceData with no parameters.
 * The feature enum constants are as follows:
 * <ul>
 * <li>FACE_RECT
 * <li>FACE_COORDINATES
 * <li>FACE_SMILE
 * <li>FACE_GAZE
 * <li>FACE_BLINK
 * <li>FACE_ORIENTATION
 * <li>FACE_IDENTIFICATION
 * </ul>
 *
 * Objects of this class cannot be cloned.
 * Facial Processing class is not thread safe, hence this class is implemented
 * as a singleton.
 * When the facial processing object is no longer needed, call the release() function
 * to dispose off the object.
 *
 * Sample code:
 * ...
 * <pre>
 * <code>
 * // check to see if the feature is supported
 *      facialProcessingFeatureSupported = FacialProcessing.isFeatureSupported(FEATURE_LIST.FEATURE_FACIAL_PROCESSING);
 *      if (facialProcessingFeatureSupported) {
 *          // instantiate the facial processing object
 *          facialProcessingObj = FacialProcessing.getInstance();
 *          if (facialProcessingObj == null) {
 *              Log.e("errorTAG", "Unable to get Facial Processing object instance");
 *          } else {
 *              // set the processing mode
 *              processingModeSet = facialProcessingObj.setProcessingMode(FP_MODES.FP_MODE_STILL);
 *              if (processingModeSet) {
 *                  // pass in an image or frame
 *                  setFrameResult = facialProcessingObj.setFrame(yuvData,  imageWidth, imageHeight, true, PREVIEW_ROTATION_ANGLE.ROT_0);
 *                  Log.v("setFrame", "setFrame returned "+setFrameResult);
 *                  // get the number of faces detected
 *                  numFaces = facialProcessingObj.getNumFaces();
 *                  if (numFaces > 0) {
 *                      // retrieve all the face data
 *                      faceDataArray = facialProcessingObj.getFaceData();
 *                      // retrieve only the smile and blink data
 *                      try{
 *                          faceSmileArray = facialProcessingObj.getFaceData(EnumSet.of(FP_DATA.FACE_SMILE, FP_DATA.FACE_BLINK));
 *                      }
 *                      catch(IllegalArgumentException e){
 *                      }
 *                      // now we can extract the data from the array
 *                      for (int i = 0; i < numFaces; i++ ) {
 *                          gazePoint[i] = faceDataArray[i].getEyeGazePoint();
 *                          smileValue[i] = faceDataArray[i].getSmileValue();
 *                          leftEyeBlink[i] = faceDataArray[i].getLeftEyeBlink();
 *                          rightEyeBlink[i] = faceDataArray[i].getRightEyeBlink();
 *                          horizontalGazeAngle[i] = faceDataArray[i].getEyeHorizontalGazeAngle();
 *                          verticalGazeAngle[i] = faceDataArray[i].getEyeVerticalGazeAngle();
 *                          faceYaw[i] = faceDataArray[i].getYaw();
 *                          facePitch[i] = faceDataArray[i].getPitch();
 *                          faceRoll[i] = faceDataArray[i].getRoll();
 *                          //face
 *                          leftEye[i] = faceDataArray[i].leftEye;
 *                          rightEye[i] = faceDataArray[i].rightEye;
 *                          mouth[i] = faceDataArray[i].mouth;
 *                          faceRect[i] = faceDataArray[i].rect;
 *                      }
 *                      // release the object when you are done
 *                      facialProcessingObj.release();
 *                  } else {
 *                      Log.v("numFaces", "getNumFaces returned "+numFaces);
 *                      facialProcessingObj.release();
 *                  }
 *              } else {
 *                  Log.e(errorTAG, "could not set the processing mode");
 *                  facialProcessingObj.release();
 *              }
 *          }
 *      } else {
 *          // do something else, the feature isn't supported
 *      }
 * ...
 * </code>
 * </pre>
 */
public class FacialProcessing {

    private static final String TAG                       = "Facial_Processing";
    private static int totalNumPeople = 0;
    private static boolean isAllZeroFlag = false;

    public enum FEATURE_LIST {
        /**
         * This is a feature constant, to be used with the isFeatureSupported() method.
         *
         */
        FEATURE_FACIAL_PROCESSING(1),
        /**
         * This is a feature constant, to be used with the isFeatureSupported() method.
         * @return
         *
         */
        FEATURE_FACIAL_RECOGNITION(2);
        //keep adding new features here, with value x such that x = 2^(n), where n = 0,1,2,....

        private int value;

        private FEATURE_LIST(int value){
            this.value = value;
        }

        protected int getValue(){
            return value;
        }
    };

    private static final int PREVIEW_FRAME_WIDTH = 480;
    private static final int PREVIEW_FRAME_HEIGHT = 480;
    private static final int NUM_POINTS_IN_PARTS = 12;                      //from qcff, number of coordinates received from native lib for a face
    private static final int START_LOC_LEFTEYE_IN_PARTS = 0;
    private static final int START_LOC_RIGHTEYE_IN_PARTS = 2;
    private static final int START_LOC_MOUTH_IN_PARTS = 4;

    private float scaleX = 1.0f;        // Normalization factor for the x-co-ordinates. Initially 1 (i.e. no normalization)
    private float scaleY = 1.0f;        // Normalization factor for the y-co-ordinates. Initially 1 (i.e. no normalization)

    private static final int CONFIG_DOWNSCALE_FACTOR = 1;                   //need to pass to config to native

    public enum FP_MODES {
        /**
         * Use this to configure the facial processing framework to the video mode.  This will set
         * the facial processing scan to only perform complete image scans on every fifteenth frame
         * while tracking detected faces in between in order to reduce the overhead of scanning for
         * faces where the image input is a set of image frames, like that coming from the camera
         * preview.
         */
        FP_MODE_VIDEO(0),       //default mode, most likely whole search mode, new faces are scanned after some intervals
        /**
         * Use this to configure the facial processing framework to still mode.  This will set
         * the facial processing framework to completely scan each image for faces.  This is best when
         * passing in static images for processing.
         */
        FP_MODE_STILL(1);       //picture mode, every frame is scanned for faces.

        private int value;

        private FP_MODES(int value){
            this.value = value;
        }

        protected int getValue(){
            return value;
        }
    };


    public enum FP_DATA {
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the face rectangle
         * coordinates for each face.
         * The face rectangle is a box that can be drawn around each detected face in the image.
         */
        FACE_RECT, // on always
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the eye and
         * mouth coordinates for each face.
         */
        FACE_COORDINATES, // = 0x1;
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the smile
         * value for each face.
         */
        FACE_SMILE, // = 0x10;
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the gaze
         * angles for both eyes, for each face.
         */
        FACE_GAZE, // = 0x100;
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the blink
         * value for both eyes, for each face.
         */
        FACE_BLINK, // = 0x1000;
        /**
         * Set this constant in your EnumSet when using getFaceData() to generate the face
         * orientation values: yaw, pitch and roll, for each face.
         */
        FACE_ORIENTATION, // = 0x10000;
        /**
         * Set this constant in your EnumSet when using getFaceData() to identify all the faces in the
         * frame.
         */
        FACE_IDENTIFICATION, // =0x100000

    };


    /**
     * This enum is used to denote the rotation angle that is required to align
     * the preview data with the preview on screen.
     * Facial data is rotated in a clockwise direction to the following angles:
     * <ul>
     * <li> 0, indicates no rotation is needed
     * <li> 90
     * <li> 180
     * <li> 270
     * </ul>
     */
    public enum PREVIEW_ROTATION_ANGLE {
        ROT_0(0),
        ROT_90(90),
        ROT_180(180),
        ROT_270(270);

        private int value;

        private PREVIEW_ROTATION_ANGLE(int value){
            this.value = value;
        }

        protected int getValue(){
            return value;
        }
    };


    private static int          facialprocHandle          = 0;
    private static int          featuresSupported         = 0;            // this will accumulate supported features

    private int previewFrameWidth = PREVIEW_FRAME_WIDTH;
    private int previewFrameHeight = PREVIEW_FRAME_HEIGHT;

    private boolean isMirrored = false;

    private int userPreferredMode = FP_MODES.FP_MODE_VIDEO.getValue();
    private int presentMode  = FP_MODES.FP_MODE_VIDEO.getValue();
    private int rotationAngleDegrees = PREVIEW_ROTATION_ANGLE.ROT_0.getValue();


    //private static int instanceCounter = 0;                        //keeps track of how many callers using the instance
    private static FacialProcessing myInstance = null;             //only instance to be shared by all callers


    public Object clone() throws CloneNotSupportedException {
        throw new CloneNotSupportedException();
    }

    /**
     * This method checks for the support of the facial processing sdk as a whole.
     *
     * @param NONE
     * @return true if Qualcomm's camera SDK is supported, else false
     */
    private static boolean isSupported() {
        /*
         * Nothing specific to check which is common for all camera features.
         * If anything turns up relatively common for all camera features, add here.
         */
        /*
         * Looks like checking existence of libraries needs to be done here, after hackathon discussion
         */

        // making sure the base library exists in the java lib.
        // For now, no other checks required.
        try {
            System.loadLibrary("mmcamera_faceproc");
        } catch (Exception e) {
            android.util.Log.e(TAG, "Base library load failed. The device doesn't have the required library.");
            facialprocHandle = 0;
            return false;
        }
        catch(UnsatisfiedLinkError e){
            android.util.Log.e(TAG, "Base library load failed. The device doesn't have the required library.");
            facialprocHandle = 0;
            return false;
        }
     // making sure libfacialproc_jni.so is downloaded and included in the project
        try {
            System.loadLibrary("facialproc_jni");
        } catch (Exception e) {
            android.util.Log.e(TAG, "Failed to load facialproc_jni. Make sure that libfacialproc_jni.so is included in your project.");
            facialprocHandle = 0;
            return false;
        }
        catch(UnsatisfiedLinkError e){
            e.printStackTrace();
            android.util.Log.e(TAG, "Failed to load facialproc_jni. Make sure that libfacialproc_jni.so is included in your project.");
            facialprocHandle = 0;
            return false;
        }
        return true;
    }


    /**
     * Use this method to determine if an facial processing feature is supported
     * by the device at runtime.  If the feature is not supported then use fallback
     * functionality or disable the dependent feature in the application.
     *
     *@param featureId Features will have defined constants through the enum FEATURE_LIST
     * that can be used for this method.  Constant name will include the prefix "FEATURE"
     * so as to be easily recognizable.
     *
     *@return true if the feature is supported, else false.
     */
    /*
     * Don't add anything that is common to all features. Do that in isSupported().
     * All specific feature checks should be done in their respective switch cases.
     *
     * HOW is this done in old SDK?
     * 1) Check for system prop "ro.qc.sdk.camera.facialproc" in build.prop
     * 2) Camera.parameters checked for "qc-camera-features".
     * 3) The return value, converted to int is checked for feature indication.
     * 4) If doing this, the SDK will need Camera object even to check feature existence.
     */
    public static boolean isFeatureSupported(FEATURE_LIST featureId) {
        // Check if SDK is supported
        boolean isSDKSupported = isSupported();

        // If SDK is supported, check if the feature is supported
        if (isSDKSupported == true) {

            switch (featureId) {
                case FEATURE_FACIAL_RECOGNITION:
                case FEATURE_FACIAL_PROCESSING: {
                    /*
                     * This had check for libraries existence, moved to isSupported() after hackathon discussion.
                     * Probably, add version check here.
                     */
                    return true;
                }

                default: {
                    facialprocHandle = 0;
                    android.util.Log.e(TAG, "featureId passed is not valid");
                    return false;
                }
            }
        }
        // if SDK itself is not supported, no feature will be checked and return false
        else{
            return false;
        }
    }


    /*
     * Creates a new facial processing object and loads the necessary libraries
     * needed to scan and process images for facial data.
     *
     * You must call {@link release()} when you are done using the facial processing object.
     *
     * @throws {@link InstantiationException} if the SDK cannot be supported due to the
     * absence of required libraries.
     */
    private FacialProcessing() throws InstantiationException{

        featuresSupported = featuresSupported | (isFeatureSupported(FEATURE_LIST.FEATURE_FACIAL_PROCESSING) ? FEATURE_LIST.FEATURE_FACIAL_PROCESSING.getValue() : 0);

        //if no feature is supported, then make the constructor throw an exception
        if(featuresSupported == 0){
            Log.e(TAG, "No features supported, libraries missing");
            InstantiationException excption = new InstantiationException("Required libraries missing");
            throw excption;
        }

        // Facial proc block
        {
            // if facial proc is not supported, then no need to load libraries and instantiate other fields.
            // Set handle to 0 and return.
            if ((FEATURE_LIST.FEATURE_FACIAL_PROCESSING.getValue() & featuresSupported) == 0) {
                facialprocHandle = 0;
            }

            // if all libaries exist, then get the handle, otherwise, keep handle 0.
            initialize();
            facialprocHandle = create();
            scaleX = 1.0f;
            scaleY = 1.0f;
            userPreferredMode = FP_MODES.FP_MODE_VIDEO.getValue();
            presentMode  = FP_MODES.FP_MODE_VIDEO.getValue();
            rotationAngleDegrees = PREVIEW_ROTATION_ANGLE.ROT_0.getValue();
            if(facialprocHandle != 0){
                config(facialprocHandle, previewFrameWidth, previewFrameHeight, CONFIG_DOWNSCALE_FACTOR);
            }
            else{
                Log.e(TAG, "Handle creation failed");
                InstantiationException excption = new InstantiationException("Handle creation failed");
                throw excption;
            }
        }
    }


    /**
     * Creates a new facial processing object and loads the necessary libraries
     * needed to scan and process images for facial data. However, if an instance of the object
     * is already in use, then null is returned.
     *
     * You must call {@link release()} when you are done using the facial processing object.
     * @return instance of FacialProcessing else returns null if instantiation fails or an
     * instance is already in use.
     */
    /*
     * We have this since facialprocessing doesnt support multiple instantiation.
     * A single object will be used for all calls.
     * If getInstance() is called when the instance is already in use, second caller gets a null.
     */
    public static FacialProcessing getInstance(){
        //if instance doesnt exist, instantiate and return.
        if(myInstance == null){
            try{
                myInstance = new FacialProcessing();
            }
            catch(InstantiationException e){
                e.printStackTrace();
                return null;
            }
            return myInstance;
        }
        //if instance exists, return null since it is already in use.
        else {
            Log.e(TAG, "Instance already in use");
            return null;
        }
    }


    /**
     * Normalizes/scales the co-ordinates to the desired dimension.
     * Call this method in case the display dimension is different from the frame/image dimension passed through
     * {@link setFrame()} or {@link setBitmap()} methods.
     * If this method is not called, then normalization is not performed.
     * This method should be invoked once for every {@link getInstance()}, and anytime the display dimensions change.
     * @param surfaceWidth the width of the display surface
     * @param surfaceHeight the height of the display surface
     */
    public void normalizeCoordinates(int surfaceWidth, int surfaceHeight){

        if(this.rotationAngleDegrees==0 || this.rotationAngleDegrees==180)
        {
                scaleX =(float)surfaceWidth/this.previewFrameWidth;
                scaleY =(float)surfaceHeight/this.previewFrameHeight;
        }
        else
        {
                scaleX =(float)surfaceWidth/this.previewFrameHeight;
                scaleY =(float)surfaceHeight/this.previewFrameWidth;
        }
    }


    /**
     * Sets the image to be scanned and processed.
     *
     * The typical use case for this method is for processing camera preview frames
     * where the image is mirrored.
     *
     * The preview data is not always in sync with the display preview and when
     * this happens, the framework needs to know the angle to which the facial data
     * coming from the scan of the preview data should be rotated in order to
     * be in sync with the display.  If no rotation is performed, the data will
     * be out of sync with the display.
     * Additionally, if the preview data and the display preview are out of sync
     * rotationally, they may also be of differing resolutions which means the
     * facial data must also be normalized to the display resolution.
     *
     * @param yuvData The image to be processed in a byte array.
     * @param frameWidth The width of the image
     * @param frameHeight The height of the image
     * @param isMirrored Allows you to specify if the image being passed in is mirrored
     * or not.
     * Set to true if the image is mirrored and false otherwise.
     * @param PREVIEW_ROTATION_ANGLE The angle to which the facial data will be rotated
     * in a clockwise direction. Pass in ROT_0 if no rotation is required.
     * @return returns false if image data is not processed, true otherwise.
     *
     */
    /*
     * This method will store the preview size values in the class storage variables and configure() is called IF new values are passed.
     * If the values are same as earlier ones, then configure() method is not called.
     * Calculations of values is NOT done here.
     */
    public boolean setFrame(byte[] yuvData, int frameWidth, int frameHeight, boolean isMirrored, PREVIEW_ROTATION_ANGLE rotationAngle){
        if (facialprocHandle == 0 || myInstance == null){
            return false;
        }
        int length = frameWidth * frameHeight * 3 / 2;
        if(yuvData.length != length || yuvData.length==0){      // Check for yuvData array length >0. Because if yuvData.length = 0 and user passes frameWidth and frameHeight =0
                                                                                                                // then we still want setFrame to fail
            android.util.Log.e(TAG, "Length of the frame does not match the dimensions passed");
            return false;
        }
        this.isMirrored = isMirrored;
        if(rotationAngle!=null)
        {
                this.rotationAngleDegrees = rotationAngle.getValue();
        }
        else
        {
                Log.e(TAG, "setFrame(): Rotation Angle equals NULL");
                return false;
        }


        if(presentMode != this.userPreferredMode)
        {
                presentMode = this.userPreferredMode;
                setMode(facialprocHandle, presentMode);
        }

        if( (frameHeight != this.previewFrameHeight) || frameWidth != this.previewFrameWidth ) {
            this.previewFrameHeight = frameHeight;
            this.previewFrameWidth = frameWidth;
            config(facialprocHandle, previewFrameWidth, previewFrameHeight, CONFIG_DOWNSCALE_FACTOR);
        }

        setFrame(facialprocHandle, yuvData);
        return true;
    }

   /**
    * Sets the static image to be scanned and processed.
    * The typical use case for this method is processing static images.
    * The framework will automatically use the FP_MODE_STILL mode when processing
    * bitmap images so it is not necessary to set the processing mode yourself.
    *
    * @param bitmap The image to be processed in bitmap format. Any image that needs to be processed has to be converted to bitmap
    * before passing it to this API.
    * @return returns false if image data is not processed, true otherwise.
    *
    */

    /*
     * This method will store the preview size values in the class storage variables and configure() is called IF new values are passed.
     * If the values are same as earlier ones, then configure() method is not called.
     * Calculations of values is NOT done here.
     */
    public boolean setBitmap(Bitmap bitmap){
        if (facialprocHandle == 0 || myInstance == null || bitmap == null){
            return false;
        }

        //if(this.presentMode == FP_MODES.FP_MODE_VIDEO.getValue())
        //{
        //      presentMode = FP_MODES.FP_MODE_STILL.getValue();
                setMode(facialprocHandle, FP_MODES.FP_MODE_STILL.getValue());
        //}
        int bitmapWidth = bitmap.getWidth();
        int bitmapHeight = bitmap.getHeight();
        Log.v(TAG, "Bitmap dimension wxh "+bitmapWidth +"x"+bitmapHeight);

        byte[] frameData = bitmapToYuv(bitmap);

        return setFrame(frameData, bitmapWidth, bitmapHeight, false, PREVIEW_ROTATION_ANGLE.ROT_0);

    }

    /*
     * Converts the inputed Bitmap image into a YUV Image.
     */
    private byte[] bitmapToYuv(Bitmap bmp) {

        int width = bmp.getWidth();
                int height = bmp.getHeight();
                int yArraySize = width*height;
                //width*height for the y values, half of that for the uv values
                byte[] yuv = new byte[yArraySize*3/2];

                int r, g, b;
                int yIndex;
                byte y;
                int color;
                int doubleI, doubleJ;
                int j;

                //Go through the bitmap and take 2x2 squares at a time,
                //converting to YUV format
                for(int i = 0; i < height/2; i++){
                        for(j = 0; j < width/2; j++){

                                //doubleI = 2*i, gj = 2*j
                                doubleI = i<<1; doubleJ = j <<1;

                                //2*i*width + 2*j
                                yIndex = doubleI*width+doubleJ;

                                //top left pixel
                                color = bmp.getPixel(doubleJ, doubleI);
                                r =  (color >>> 16) & 0xFF;
                                g = (color >>> 8) & 0xFF;
                                b = color& 0xFF;

                                y = (byte) (( (66*r + 129*g +  25*b + 128) >>> 8) +  16);

                                yuv[yIndex] = y;

                                //top right pixel
                                color = bmp.getPixel(doubleJ+1, doubleI);
                                r =  (color >>> 16) & 0xFF;
                                g = (color >>> 8) & 0xFF;
                                b = color& 0xFF;

                                y = (byte) (( (66*r + 129*g +  25*b + 128) >>> 8) +  16);

                                yuv[yIndex+1] = y;

                                //bottom left pixel
                                color = bmp.getPixel(doubleJ, doubleI+1);
                                r =  (color >>> 16) & 0xFF;
                                g = (color >>> 8) & 0xFF;
                                b = color& 0xFF;

                                y = (byte) (( (66*r + 129*g +  25*b + 128) >>> 8) +  16);

                                yuv[yIndex+width] = y;

                                //bottom right pixel
                                color = bmp.getPixel(doubleJ+1, doubleI+1);
                                r =  (color >>> 16) & 0xFF;
                                g = (color >>> 8) & 0xFF;
                                b = color& 0xFF;

                                y = (byte) (( (66*r + 129*g +  25*b + 128) >>> 8) +  16);

                                yuv[yIndex+width+1] = y;
                        }
                }

                return yuv;

        }


        /**
     * Use to query the framework for the number of faces detected in the image
     * last input through {@link setFrame()} or {@link setBitmap()}.
     *
     * @return The number of faces detected
     *
     */
    public int getNumFaces() {
        if (facialprocHandle == 0 || myInstance == null){
            return 0;
        }
        int[] facesData = getCompleteInfos( facialprocHandle, true,    //Rect info
                false,            //Eyes, mouth loc
                false,                                                 //Other coordinate info, not used
                false,            //Yaw, pitch, roll
                false,                  //Smile value/degree
                false,                   //Gaze points
                false);                 //Blink info

        if(facesData == null || facesData.length == 0) {
            Log.v(TAG, "getFaceData: No info");
            return 0; //exit point
        }

        int numElemsPerFace = 12;       // Starting by default with compulsory 4 cells of Face Rect plus 8 cells for other unwanted face data = 12.

        int totalNumFaces = facesData.length/numElemsPerFace;
        return totalNumFaces;
        //return getNumFaces(facialprocHandle);
    }


    /**
     * Use to request an array of Face objects, one for each face detected in the
     * input image.  Use this method to request the calculation of all facial data
     * points. This includes:
     * <ul>
     * <li>FACE_RECT
     * <li>FACE_COORDINATES
     * <li>FACE_SMILE
     * <li>FACE_GAZE
     * <li>FACE_BLINK
     * <li>FACE_ORIENTATION
     * <li>FACE_IDENTIFICATION
     * </ul>
     *
     * @return Returns an array of Face objects, one for each face detected.
     * If no face is detected then this will return NULL.
     *
     */
    public FaceData[] getFaceData() {
        if (facialprocHandle == 0 || myInstance == null) {
            return null;
        } else {
            EnumSet<FP_DATA> set = EnumSet.allOf(FP_DATA.class);
            try{
                return getFaceData(set);
            }
            catch(Exception e){
                return null;
            }
        }
    }


    /**
     * Use to request an array of Face object, one for each face detected in the
     * input image.  Use this method to select the specific facial data points and
     * prevent unwanted data points from being calculated.  This will allow for less
     * calculation overhead.
     *
     * @param faceFeatureData Desired facial data points are passed as an EnumSet
     * parameter.  Use any combination of the following constants as part of the EnumSet:
     * <ul>
     * <li>FACE_RECT
     * <li>FACE_COORDINATES
     * <li>FACE_SMILE
     * <li>FACE_GAZE
     * <li>FACE_BLINK
     * <li>FACE_ORIENTATION
     * <li>FACE_IDENTIFICATION
     * </ul>
     * FACE_RECT is always calculated so it is not necessary to include it in the EnumSet.
     *
     * @return Returns an array of Face objects, one array for each face detected.
     * If no face is detected then this will return NULL.
     * @exception IllegalArgumentException if the passed EnumSet is invalid or null is passed.
     */
    /*
     * Returns null if handle is invalid or if number of faces is 0.
     * If faces exist, then get the values based on feature info sent by user, fill up the data structure for faces and return.
     * Condition to pass in to native method will be (faceFeatureData & <featurename> != 0).
     * After getting all info, set them up into the structure.
     * When reading from the structure, features are arranged sequentially but they dont have fixed location.
     * If a feature was not requested, it wont be included so so not skip the values in the int array.
     */
    public FaceData[] getFaceData(EnumSet<FP_DATA> dataSet) throws IllegalArgumentException{

        if(dataSet == null){
            throw new IllegalArgumentException();
        }
        if (facialprocHandle == 0 || myInstance == null) {
            Log.v(TAG, "getFaceData: Invalid handle");
            return null; //exit point
        }
        else {
            int numFaces = getNumFaces(facialprocHandle);
            if(numFaces == 0 || dataSet == null) {
                Log.v(TAG, "getFaceData: No faces");
                return null; //exit point
            }

            //Do a validity check of the passed value and inform the user if it is invalid.
            //However, get the face rect since it is default.
            //Same as ((faceFeatureData&FACE_COORDINATES)!=0) && ((faceFeatureData&FACE_ORIENTATION)!=0)
            //&& ((faceFeatureData&FACE_SMILE)!=0) && ((faceFeatureData&FACE_GAZE)!=0) && ((faceFeatureData&FACE_BLINK)!=0)
            //This check no more needed since we migrated to enumset
            /*if( (faceFeatureData>FACE_RECT) && ((faceFeatureData&FACE_ALL_FEATURES)==0) ){
                android.util.Log.e(TAG, "Invalid faceFeatureData value given to getFaceData");
            }*/

            int[] facesData = getCompleteInfos( facialprocHandle, true,    //Rect info
                    dataSet.contains(FP_DATA.FACE_COORDINATES),            //Eyes, mouth loc
                    false,                                                 //Other coordinate info, not used
                    dataSet.contains(FP_DATA.FACE_ORIENTATION),            //Yaw, pitch, roll
                    dataSet.contains(FP_DATA.FACE_SMILE),                  //Smile value/degree
                    dataSet.contains(FP_DATA.FACE_GAZE),                   //Gaze points
                    dataSet.contains(FP_DATA.FACE_BLINK));                 //Blink info

            if(facesData == null || facesData.length == 0) {
                Log.v(TAG, "getFaceData: No info");
                return null; //exit point
            }

            int numElemsPerFace = 12;   // Starting by default with compulsory 4 cells of Face Rect plus 8 cells for other unwanted face data = 12.
            if(dataSet.contains(FP_DATA.FACE_COORDINATES))
            {
                numElemsPerFace+=24;
            }
            if(dataSet.contains(FP_DATA.FACE_ORIENTATION))
            {
                numElemsPerFace+=3;
            }
            if(dataSet.contains(FP_DATA.FACE_SMILE))
            {
                numElemsPerFace++;
            }
            if(dataSet.contains(FP_DATA.FACE_GAZE))
            {
                numElemsPerFace+=2;
            }
            if(dataSet.contains(FP_DATA.FACE_BLINK))
            {
                numElemsPerFace+=2;
            }
            ArrayList<FaceData> faceDataList = new ArrayList<FaceData>((facesData.length)/numElemsPerFace);
            int j = 0;

            //do for each face
            try{ //I fear for arrayindex out of bounds, for j.
                for(int i = 0; i < (facesData.length)/numElemsPerFace; i++) {

                                FaceData face = new FaceData(isMirrored, rotationAngleDegrees);

                        //get rect info
                        face.rect = new Rect(facesData[j], facesData[j+1], facesData[j]+facesData[j+2], facesData[j+1]+facesData[j+3]);
                        j+=4;

                        //get eyes, mouth coodinates. Take only the first 6 values denoting center of two eyes and mouth. Ignore the rest
                        if( dataSet.contains(FP_DATA.FACE_COORDINATES) ) {

                            face.leftEye = new Point(facesData[j+START_LOC_LEFTEYE_IN_PARTS], facesData[j+START_LOC_LEFTEYE_IN_PARTS+1]);

                            face.rightEye = new Point(facesData[j+START_LOC_RIGHTEYE_IN_PARTS],  facesData[j+START_LOC_RIGHTEYE_IN_PARTS+1]);

                            face.mouth = new Point(facesData[j+START_LOC_MOUTH_IN_PARTS], facesData[j+START_LOC_MOUTH_IN_PARTS+1]);
                            j+=(NUM_POINTS_IN_PARTS<<1);          //do this to ignore the rest
                        }


                        //get yaw, pitch, roll
                        if( dataSet.contains(FP_DATA.FACE_ORIENTATION) ) {
                            face.setPitch(facesData[j]);
                            face.setYaw(facesData[j+1]);
                            face.setRoll(facesData[j+2]);
                            j+=3;
                        }

                        //get smile value
                        if( dataSet.contains(FP_DATA.FACE_SMILE) ) {
                            face.setSmileValue(facesData[j]);
                            j++;
                        }

                        //get gaze angles
                        if( dataSet.contains(FP_DATA.FACE_GAZE) ) {
                            face.setEyeGazeAngles(facesData[j], facesData[j+1]);
                            j+=2;
                        }

                        //get blink
                        if( dataSet.contains(FP_DATA.FACE_BLINK) ) {
                            face.setBlinkValues(facesData[j], facesData[j+1]);
                            j+=2;
                        }
                        if(dataSet.contains(FP_DATA.FACE_IDENTIFICATION))
                        {
                                int [] faceRecogData = identifyPerson(facialprocHandle, i);// native jni call
                            if(faceRecogData!=null)
                            {
                                if(faceRecogData[0] == -1)
                                {
                                        face.setPersonId(FacialProcessingConstants.FP_PERSON_NOT_REGISTERED);
                                        face.setRecognitionConfidence(FacialProcessingConstants.FP_PERSON_NOT_REGISTERED);
                                }
                                else
                                {
                                        face.setPersonId(faceRecogData[0]);
                                        face.setRecognitionConfidence(faceRecogData[2]);
                                }
                            }
                            else
                            {
                                face.setPersonId(FacialProcessingConstants.FP_PERSON_NOT_REGISTERED);
                                face.setRecognitionConfidence(FacialProcessingConstants.FP_PERSON_NOT_REGISTERED);
                                Log.e(TAG, "identifyPersonEnum: faceRecogData[] equals NULL");
                            }
                        }

                        face.doRotationNMirroring(previewFrameWidth, previewFrameHeight);

                        face.normalizeCoordinates(scaleX, scaleY);

                        faceDataList.add(face);
                        }



            }
            catch(Exception e){
               // Log.e(TAG, e.getMessage());
                e.printStackTrace();
                faceDataList.clear();
                faceDataList = null;
                return null;
            }
            return faceDataList.toArray( new FaceData[faceDataList.size()]);
        }
    }

    /**
     * Description: Use this API to train the system by 'adding' faces to the album. Make sure to call FacialProcessing.setFrame() or FacialProcessing.setBitmap before calling
     * addPerson(). This API takes in a valid faceIndex. If adding that face was successful then it will return a PersonId or else it will return an error code. FaceIndex corresponds
     * to the array index of the FaceData array that is returned by doing a getFaceData call. Adding a person will be successful if that person have NOT already been added to the album.
     * If we want to train the system with more images of that face then use the updatePerson API. If multiple faces with different illuminations and facial expressions are registered, then
     * recognition quality will increase.
     *
     * @param faceIndex - Array index of the FaceData array
     * @return personId - Unique number for each person in the album
     * @throws IllegalArgumentException
     */
    public int addPerson(int faceIndex) throws IllegalArgumentException{
        FaceData [] faceDataArray = getFaceData();
        if(faceDataArray!=null)
        {
                int numFaces = faceDataArray.length;
                if(faceIndex<0 || faceIndex > (numFaces-1))
                {
                        Log.e(TAG, "addPerson(): Invalid Face Index");
                        throw new IllegalArgumentException();
                }
                else
                {
                        int [] faceRecogData = identifyPerson(facialprocHandle, faceIndex);
                        if(faceRecogData[0] == -1)// Success ! Face does not exists.
                {
                    int faceFeature = getFaceFeature(facialprocHandle, faceIndex);// native jni call
                    int personId = addPerson(facialprocHandle, faceFeature);// native jni call
                    if(personId == -1)// addPerson Failed
                    {
                        Log.e(TAG, "addPerson(): Adding a person failed internally");
                        return FacialProcessingConstants.FP_INTERNAL_ERROR;// Internal Error
                    }
                    else//SUCCESS !
                    {
                        Log.d(TAG, "addPerson(): User image added successfully");
                        return personId;
                    }
                }
                else // Fail! Face exists
                {
                    Log.e(TAG, "addPerson(): Face already exists");
                    return FacialProcessingConstants.FP_FACE_EXIST_ERROR; // Error code for face exists.
                }
                }
        }
        else
        {
                Log.e(TAG, "addPerson(): No face detected");
                return FacialProcessingConstants.FP_NO_FACE_DETECTED_ERROR;
        }
    }

    /**
     * Description: Use this API to add more images of an existing person from the album. This API takes in valid PersonId and a valid faceIndex. If you are adding a face image for the first time then
     * use addPerson API. Maximum number of faces that can be added per faceID is 10.
     *
     * @param PersonId - Unique number for each person in the album
     * @param faceIndex - Array index of the FaceData array
     * @return  FacialProcessingConstants.FP_SUCCESS(0) - If the updating a person to the album was successful
     *                  FacialProcessingConstants.FP_NO_FACE_DETECTED_ERROR(-1) - If no face was detected in the frame
     *                  FacialProcessingConstants.FP_INTERNAL_ERROR(-2) - If updating a person failed internally     *
     * @throws IllegalArgumentException
     */
    public int updatePerson(int personId, int faceIndex)throws IllegalArgumentException{

        FaceData [] faceDataArray = getFaceData();
        if(faceDataArray!=null) // Check to see if any face is detected in the frame
        {
                if(personId<0)  // Invalid person inputed
                {
                        Log.e(TAG, "updatePerson(): Invalid personId");
                        throw new IllegalArgumentException();
                }
                int numFaces = faceDataArray.length;
                if(faceIndex<0 || faceIndex >(numFaces-1))      // Invalid face index inputed
                {
                        Log.e(TAG, "updatePerson(): Invalid faceIndex");
                        throw new IllegalArgumentException();
                }
                else
                {
                        int faceFeature = getFaceFeature(facialprocHandle, faceIndex); // native jni call
                        int result = updatePerson(facialprocHandle, faceFeature, personId);
                        if(-1 == result)
                        {
                                Log.e(TAG, "updatePerson(): Updating person failed internally");
                    return FacialProcessingConstants.FP_INTERNAL_ERROR;
                        }
                        else
                        {
                                Log.d(TAG, "updatePerson(): Successful");
                                return FacialProcessingConstants.FP_SUCCESS;
                        }
                }
        }
        else
        {
                Log.e(TAG, "updatePerson(): No face detected");
            return FacialProcessingConstants.FP_NO_FACE_DETECTED_ERROR;   // No Face Detected
        }
    }

    /**
     * Description: Use this API to delete a specific user/personId from the album. Calling this API will delete all the images associated with the personId provided. If the
     * person is deleted successfully then it will return TRUE else FALSE.
     *
     * @param personId - The unique number of the person to be deleted from the album
     * @return True - If the given PersonIds was successfully deleted
     *         False - If the given PersonIds was NOT successfully deleted
     */
    public boolean deletePerson(int personId) throws IllegalArgumentException{

        if(personId<0)
        {
                Log.e(TAG, "deletePerson(): Invalid faceId");
                throw new IllegalArgumentException();
        }
        if(facialprocHandle!=0)
        {
            int result = removePerson(facialprocHandle, personId);
            if(result == -1)
            {
                Log.e(TAG, "deletePerson(): Deleting person failed internally");
                return false;
            }
            else
            {
                Log.d(TAG, "deletePerson(): Deleting person failed internally");
                return true;
            }
        }
        else
        {
            Log.e(TAG, "deletePerson(): Deleting person failed internally");
            return false;
        }
    }

    /**
     * Description: Use this API to delete ALL the users/personIds from the album. If reset Album was successful then it will return TRUE else FALSE
     *
     * @return True - If all users were successfully deleted from the album
     *         False - If reseting the album failed
     */
    public boolean resetAlbum() {
            if(facialprocHandle!=0)
            {
                int result = resetAlbum(facialprocHandle);
                if(result == -1)
                {
                Log.e(TAG, "resetAlbum: Reset album failed internally");
                return false;
                }
                else
                {
                Log.d(TAG, "resetAlbum: Reset album successful");
                return true;
                }
            }
            else
            {
            Log.e(TAG, "resetAlbum: Reset album failed internally");
            return false;
            }
    }

    /**
     * Description: Use this API to convert your album in a byte array so that you can store it on the local system storage and use it at a later stage
     * @return - A valid byte array of the album or NULL if the serialization failed.
     */
    public byte[] serializeRecogntionAlbum() {
        if(facialprocHandle!=0)
        {
            Log.d(TAG, "serializeAlbum: Serializing album successful");
            return serializeAlbum(facialprocHandle);
        }
        else
        {
            Log.e(TAG, "serializeAlbum: serializing album failed due to internal error");
            return null;
        }
    }

    /**
     * Description: Use this API to convert a valid album byte array created by {@link serializeAlbum()} API into a valid Facial Processing album.
     *
     * @param albumBuffer - Album data returned by serializeRecognitionAlbum()[@link] API
     * @return - TRUE if the album byte array was successfully converted into an album
     *           FALSE if the conversion was unsuccessful.
     */
    public boolean deserializeRecognitionAlbum(byte[] albumBuffer) throws IllegalArgumentException {
        if(albumBuffer==null)
        {
            Log.e(TAG, "deserializeAlbum(): Input buffer = NULL");
            throw new IllegalArgumentException();
        }
        if(facialprocHandle!=0)
        {
            int result = deserializeAlbum(facialprocHandle, albumBuffer.length, albumBuffer);
            if(result == -1)
            {
                Log.e(TAG, "deserializeAlbum: Deserializing album failed internally");
                return false;
            }
            else
            {
                Log.d(TAG, "deserializeAlbum: Deserializing album successful");
                return true;
            }
        }
        else
        {
            Log.e(TAG, "deserializeAlbum: Deserializing album failed internally");
            return false;
        }
    }

    /**
     * Description: Use this API to set the confidence value with which the person/face should be identified. Higher the confidence value means higher the facial identification bar.
     *
     * @param confidenceValue - An integer value between 0 and 100
     * @return - FacialProcessingConstants.FP_SUCCESS(0)- If the confidence value was set successfully.
     *        OR FacialProcessingConstants.FP_CONFIDENCE_OUT_OF_RANGE_ERROR(-4)- If the input confidence value is out of range.
     */
    public int setRecognitionConfidence(int confidenceValue)throws IllegalArgumentException{
        if(confidenceValue>0 && confidenceValue <100)
        {
            setConfidenceValue(confidenceValue);
            return FacialProcessingConstants.FP_SUCCESS;
        }
        else
        {
                throw new IllegalArgumentException();
        }
    }

    /**
     * Description: Use this API to get the number of people stored in the currently-loaded album
     *
     * @return -The number of people that have been stored in the album; zero if the album is empty
     */
        public int getAlbumPersonCount() {
            return getNumberOfPeople(facialprocHandle);
        }


    /**
     * This method will release the facial processor
     *
     */
    public void release() {
        if(myInstance != null) {
            destroy(facialprocHandle);
            deinitialize();
            myInstance = null;
        }
    }

    /**
     * Use to configure the scanning mode for the facial processing engine.
     * The default mode is FP_MODE_VIDEO.
     *
     * @param mode Specify one of two available modes:
     * <ul>
     * <li>FP_MODE_VIDEO: This is the most optimized scanning mode and is typically used when processing high frame rate camera preview frames or video
     * <li>FP_MODE_STILL: This is the most accurate scanning mode and is typically used when processing still images or low frame rate video
     * </ul>
     *
     * @return true if the mode is set successfully, else false.
     *
     */
    /*
     * Check if the mode passed is valid.
     * If valid then call native setMode().
     * As per docs of the base lib used, default mode is Video.
     */
    public boolean setProcessingMode(FP_MODES mode) {
        if(facialprocHandle == 0 || myInstance == null){
            android.util.Log.e(TAG, "setMode failed. Mode: "+mode+", handle: "+facialprocHandle);
            return false;
        }
        else{
            //as per doc, when switching to video mode, clear some data structures, which seems to be handled in jni.
            this.userPreferredMode = mode.getValue();
            presentMode = this.userPreferredMode;
            setMode(facialprocHandle, this.userPreferredMode);
            Log.d(TAG, "Mode set");
            return true;
        }
    }

    /*
     * Method to get the Facial Processing Handle so that it can be accessed in Facial Recognition class.
     */
    protected int getHandle(){
        return facialprocHandle;
    }


    /* Native Functions */
    private native int initialize();
    private native void deinitialize();
    private native void config(int handle, int width, int height, int downscaleFactor);
    private native void setFrame(int handle, byte[] frame);
    private native int getNumFaces(int handle);
    private native int create();
    private native void destroy(int handle);
    private native void setMode(int handle, int mode);
    private native int[] getCompleteInfos(int handle, boolean getRect, boolean getCoOrd,
            boolean getCoOrdEx, boolean getOrientation, boolean getSmileValue, boolean getGaze, boolean getBlink);

    // Facial Recognition Native calls
    private native int [] identifyPerson(int handle, int faceId);
    private native int getFaceFeature(int handle, int faceId);
    private native int addPerson(int handle, int faceFeatureId);
    private native int updatePerson(int handle, int faceFeatureId, int faceId);
    private native int removePerson(int handle, int faceFeatureId);
    private native int resetAlbum(int handle);
    private native byte [] serializeAlbum(int handle);
    private native int deserializeAlbum(int handle, int bufferSize, byte[] byteArray);
    private native int setConfidenceValue(int confidenceValue);
    private native int getNumberOfPeople(int handle);


    protected static class Log {
        static final boolean LOG_ENABLED = true; //set this to false to stop logging

        protected static void i(String tag, String text){
            if(LOG_ENABLED){
                android.util.Log.i(tag, text);
            }
        }

        protected static void d(String tag, String text){
            if(LOG_ENABLED){
                android.util.Log.d(tag, text);
            }
        }

        protected static void e(String tag, String text){
            if(LOG_ENABLED){
                android.util.Log.e(tag, text);
            }
        }

        protected static void v(String tag, String text){
            if(LOG_ENABLED){
                android.util.Log.v(tag, text);
            }
        }
    }

}
