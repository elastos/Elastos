package org.elastos.wallet.ela.utils.widget;

import android.support.v4.view.ViewPager;
import android.view.View;

public class ScaleTransformer implements ViewPager.PageTransformer {
    private static final float MIN_SCALE = 0.8f;
    //private static final float MIN_ALPHA = 0.5f;

    @Override
    public void transformPage(View page, float position) {
        if (position < -1 || position > 1) {
            //page.setAlpha(MIN_ALPHA);
            //page.setScaleX(MIN_SCALE);
            page.setScaleY(MIN_SCALE);
        } else if (position <= 1) { // [-1,1]
            //具体实现，此时page在界面的左边并且已经不显示在当前界面
            //float scaleFactor = Math.max(MIN_SCALE, 1 - Math.abs(position));
            if (position < 0) {
                float scaleX = 1 + 0.2f * position;
                //Log.d("google_lenve_fb", "transformPage: scaleX:" + scaleX);

                page.setScaleY(scaleX);
            } else {
                float scaleX = 1 - 0.2f * position;

                page.setScaleY(scaleX);
            }
           // page.setAlpha(MIN_ALPHA + (scaleFactor - MIN_SCALE) / (1 - MIN_SCALE) * (1 - MIN_ALPHA));
        }
    }
}
