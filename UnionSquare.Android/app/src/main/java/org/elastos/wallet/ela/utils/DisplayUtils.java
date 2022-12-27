/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.utils;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Rect;

/**
 * 显示工具类
 *
 * @author Created by jiangyujiang on 2016/9/7.
 */
public class DisplayUtils {
    private static int mScreenWidth;
    private static int mScreenHeight;
    private static float mScreenDensity;
    private static int mStatusBarHeight;
    private static int mNavigationBarHeight;

    /**
     * 获取手机屏幕高度
     *
     * @param context 上下文
     * @return 屏幕的高度
     */
    public static int getScreenHeight(Context context) {
        if (mScreenHeight <= 0) {
            mScreenHeight = context.getResources().getDisplayMetrics().heightPixels;
        }
        return mScreenHeight;
    }

    /**
     * 获取手机屏幕宽度
     *
     * @param context 上下文
     * @return 屏幕的宽度
     */
    public static int getScreenWidth(Context context) {
        if (mScreenWidth <= 0) {
            mScreenWidth = context.getResources().getDisplayMetrics().widthPixels;
        }
        return mScreenWidth;
    }

    /**
     * 获取手机屏幕像素密度
     *
     * @param context 上下文
     * @return 像素密度
     */
    public static float getDensity(Context context) {
        if (mScreenDensity <= 0) {
            mScreenDensity = context.getResources().getDisplayMetrics().density;
        }
        return mScreenDensity;
    }

    /**
     * 获取状态栏高度
     *
     * @param context 上下文
     * @return 状态栏高度，如果界面没有呈现将返回0
     */
    public static int getStatusBarHeight(Activity context) {
        if (mStatusBarHeight <= 0) {
            Resources resources = context.getResources();
            int resourceId = resources.getIdentifier("status_bar_height", "dimen", "android");
            if (resourceId > 0) {
                try {
                    mStatusBarHeight = resources.getDimensionPixelSize(resourceId);
                } catch (Resources.NotFoundException e) {
                    e.printStackTrace();
                }
            }
            // 直接获取失败，尝试从显示视图中获取，必须在界面显示之后才能获取到值
            if (mStatusBarHeight <= 0) {
                Rect frame = new Rect();
                context.getWindow().getDecorView().getWindowVisibleDisplayFrame(frame);
                mStatusBarHeight = frame.top;
            }
        }
        return mStatusBarHeight;
    }

    public static int getNavigationBarHeight(Activity context) {
        if (mNavigationBarHeight <= 0) {
            Resources resources = context.getResources();
            int resourceId = resources.getIdentifier("navigation_bar_height", "dimen", "android");
            if (resourceId > 0) {
                try {
                    mNavigationBarHeight = resources.getDimensionPixelSize(resourceId);
                } catch (Resources.NotFoundException e) {
                    e.printStackTrace();
                }
            }
        }
        return mNavigationBarHeight;
    }

    /**
     * dp转px
     *
     * @param context 上下文
     * @param dp      相对像素密度值
     * @return 像素值
     */
    public static int dp2px(Context context, float dp) {
        return (int) (dp * getDensity(context) + 0.5f);
    }

}
