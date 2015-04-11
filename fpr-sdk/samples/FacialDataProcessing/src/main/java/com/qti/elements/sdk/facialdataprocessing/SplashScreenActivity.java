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
 * @file:   SplashScreenActivity.java
 *
 */


package com.qti.elements.sdk.facialdataprocessing;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

public class SplashScreenActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.splash);
        //rest of the code

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                /* Create an Intent that will start the Menu-Activity. */
                Intent mainIntent = new Intent(SplashScreenActivity.this, CameraPreviewActivity.class);
                SplashScreenActivity.this.startActivity(mainIntent);
                SplashScreenActivity.this.finish();
            }
        }, 1500);
    }

}
