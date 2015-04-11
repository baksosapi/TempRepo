/* ======================================================================
 *  Copyright © 2013 Qualcomm Technologies, Inc. All Rights Reserved.
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
 * @file:   GalleryProcessing.java
 *
 */
package com.qti.elements.sdk.samples.galleryprocessing;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.qti.elements.sdk.fpr.FaceData;
import com.qti.elements.sdk.fpr.FacialProcessing;

public class GalleryProcessing extends Activity {

        private static final String TAG = "GalleryProcessing";
        private final int PIC_FROM_GALLERY = 1;
        private long time;
        private FacialProcessing fp;
        private ImageView image;
        private TextView text;

        //static makes the values stay even during screen rotation
        private static String textToWrite = "";
        private static Bitmap currentDisplayedImage = null;
        private int[] colors = {
                        Color.RED, Color.BLUE,
                        Color.GREEN, Color.YELLOW,
                        Color.CYAN, Color.MAGENTA
        };
        @Override
        protected void onCreate(Bundle savedInstanceState) {
                super.onCreate(savedInstanceState);
                setContentView(R.layout.activity_image_conversion);

                image = (ImageView)findViewById(R.id.picture);
                text = (TextView)findViewById(R.id.text);
                text.setTextColor(Color.WHITE);

                // This handles keeping the images and text
                // for when the screen is rotated
                if(currentDisplayedImage!=null)
                        image.setImageBitmap(currentDisplayedImage);

                if(textToWrite.length()>0)
                        text.setText(textToWrite);

                fp = FacialProcessing.getInstance();
                //this is automatically done by set bitmap
                //fp.setProcessingMode(FP_MODES.FP_MODE_STILL);

                Button selectButton = (Button)findViewById(R.id.select_picture_button);
                selectButton.setOnClickListener(new OnClickListener(){
                        @Override
                        public void onClick(View v) {
                                textToWrite = "";
                                Intent gallery = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                                startActivityForResult(gallery, PIC_FROM_GALLERY);
                        }
                });
        }

        @Override
        protected void onDestroy(){
                if(fp!=null) fp.release();
                super.onDestroy();
        }

        @Override
        protected void onActivityResult(int requestCode, int resultCode, Intent data){
                super.onActivityResult(requestCode, resultCode, data);
                if (requestCode == PIC_FROM_GALLERY && resultCode == RESULT_OK && data!=null) {
            Uri selectedImage = data.getData();
            String[] filePathColumn = { MediaStore.Images.Media.DATA };

            Cursor cursor = getContentResolver().query(selectedImage, filePathColumn, null, null, null);
            cursor.moveToFirst();

            int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
            String picturePath = cursor.getString(columnIndex);
            cursor.close();

            time = SystemClock.currentThreadTimeMillis();
            //make it mutable so we can draw on it later
            Options bitmapOptions = new Options();
            bitmapOptions.inMutable = true;
            Bitmap bmp = BitmapFactory.decodeFile(picturePath, bitmapOptions);
            time = SystemClock.currentThreadTimeMillis()-time;

            textToWrite += "Image Dimensions:\n" + bmp.getWidth() +"x" + bmp.getHeight()+"\n";
            textToWrite += "Decode Bitmap:" + time + " ms\n";
            analyzeAndDisplayImage(bmp);
        }
        }

        private void analyzeAndDisplayImage(Bitmap bmp){
                time = SystemClock.currentThreadTimeMillis();
                boolean analyzed = fp.setBitmap(bmp);
                time = SystemClock.currentThreadTimeMillis() - time;
                textToWrite += "Process Bitmap:"+time+" ms\n";

                if(!analyzed)
                        textToWrite+="setBitmapFailed.\n";
                else{
                        textToWrite += "Number of Faces:" + fp.getNumFaces()+"\n";

                        // Image resources are immutable
                        //bmp = convertToMutable(getBaseContext(), bmp);

                        Paint paint = new Paint();
                        Canvas canvas = new Canvas(bmp);
                        int x, y;
                        float pixelDensity = getResources().getDisplayMetrics().density;
                        FaceData[] faces = fp.getFaceData();
                        for(int i = 0; i < fp.getNumFaces(); i++){
                                paint.setColor(colors[i%6]);
                                FaceData f = faces[i];

                                Rect r = f.rect;
                                paint.setStyle(Paint.Style.STROKE);
                                paint.setStrokeWidth(r.width()/75*pixelDensity);
                                canvas.drawRect(r, paint);

                                paint.setStyle(Paint.Style.FILL);
                                x = f.leftEye.x;
                                y = f.leftEye.y;
                                canvas.drawCircle(x, y, r.width()/50*pixelDensity,paint);
                                x = f.rightEye.x;
                                y = f.rightEye.y;
                                canvas.drawCircle(x, y, r.width()/50*pixelDensity, paint);
                                x = f.mouth.x;
                                y = f.mouth.y;
                                canvas.drawCircle(x, y, r.width()/25*pixelDensity, paint);
                        }
                }
                currentDisplayedImage = bmp;
                image.setImageBitmap(bmp);
                text.setText(textToWrite);
        }

}
