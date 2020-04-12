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

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.net.Uri;
import android.os.Environment;
import android.text.TextUtils;
import android.view.View;

import org.elastos.wallet.ela.base.BaseActivity;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class ShareUtil {

    /**
     * 截取View
     *
     * @param view
     * @return
     */
    public static Bitmap viewShot1(View view) {

        float scalex = view.getScaleX();
        float scaley = view.getScaleY();
        //得到缩放后webview内容的高度
        int webViewHeight = (int) (view.getHeight() * scalex);
        int wedth = (int) (view.getWidth() * scaley);
        Bitmap bitmap = Bitmap.createBitmap(wedth, webViewHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        //绘制
        view.draw(canvas);
        return bitmap;
    }

    /**
     * 分享
     **/
    public static void fxPic(BaseActivity context, View webView) {
        Bitmap bgimg0 = viewShot1(webView);
        Intent share_intent = new Intent();
        share_intent.setAction(Intent.ACTION_SEND);//设置分享行为
        share_intent.setType("image/*");  //设置分享内容的类型
        share_intent.putExtra(Intent.EXTRA_STREAM, saveBitmap(context, bgimg0, DateUtil.getCurrentData()));
        //创建分享的Dialog
        share_intent = Intent.createChooser(share_intent, "Share");
        context.startActivity(share_intent);

    }

    /**
     * 将图片存到本地
     */
    private static Uri saveBitmap(BaseActivity context, Bitmap bm, String picName) {
        //
        try {
            File file1 = context.getExternalFilesDir("img");
            File f = new File(file1, "/" + picName + ".jpg");
            if (!f.exists()) {
                f.getParentFile().mkdirs();
                f.createNewFile();
            }
            FileOutputStream out = new FileOutputStream(f);
            bm.compress(Bitmap.CompressFormat.PNG, 90, out);
            out.flush();
            out.close();
            Uri uri = Uri.fromFile(f);
            return uri;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (bm != null) {
                bm.recycle();
            }

        }
        return null;
    }
}
