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

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONObject;
import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.NotFoundException;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.RGBLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.HybridBinarizer;
import com.google.zxing.qrcode.QRCodeWriter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;


public class QRCodeUtils {

    public static QRCodeCreateListener cListener;
    public static QRCodeDecodeListener dListener;

    public interface QRCodeCreateListener {
        void onCreateSuccess(Bitmap bitmap);
    }

    public interface QRCodeDecodeListener {
        void onDecodeSuccess(String result);
    }

    public interface ErrorListenner {
        void errorSuccess(String str);
    }

    // 生成QR图
    public static void createImage(String text, int w, int h, Bitmap logo, QRCodeCreateListener listener) {
        try {
            Bitmap scaleLogo = getScaleLogo(logo, w, h);
            int offsetX = (w - scaleLogo.getWidth()) / 2;
            int offsetY = (h - scaleLogo.getHeight()) / 2;
            Hashtable<EncodeHintType, String> hints = new Hashtable<>();
            hints.put(EncodeHintType.CHARACTER_SET, "utf-8");
            BitMatrix bitMatrix = new QRCodeWriter().encode(text,
                    BarcodeFormat.QR_CODE, w, h, hints);
            int[] pixels = new int[w * h];
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    if (x >= offsetX && x < offsetX + scaleLogo.getWidth() && y >= offsetY && y < offsetY + scaleLogo.getHeight()) {
                        int pixel = scaleLogo.getPixel(x - offsetX, y - offsetY);
                        if (pixel == 0) {
                            if (bitMatrix.get(x, y)) {
                                pixel = 0xff000000;
                            } else {
                                pixel = 0xffffffff;
                            }
                        }
                        pixels[y * w + x] = pixel;
                    } else {
                        if (bitMatrix.get(x, y)) {
                            pixels[y * w + x] = 0xff000000;
                        } else {
                            pixels[y * w + x] = 0xffffffff;
                        }
                    }
                }
            }
            Bitmap bitmap = Bitmap.createBitmap(w, h,
                    Bitmap.Config.RGB_565);
            bitmap.setPixels(pixels, 0, w, 0, 0, w, h);
            listener.onCreateSuccess(bitmap);
//            return bitmap;
        } catch (WriterException e) {
            e.printStackTrace();
        }
//        return null;
    }

    private static Bitmap getScaleLogo(Bitmap logo, int w, int h) {
        if (logo == null) return null;
        Matrix matrix = new Matrix();
        float scaleFactor = Math.min(w * 1.0f / 5 / logo.getWidth(), h * 1.0f / 5 / logo.getHeight());
        matrix.postScale(scaleFactor, scaleFactor);
        Bitmap result = Bitmap.createBitmap(logo, 0, 0, logo.getWidth(), logo.getHeight(), matrix, true);
        return result;
    }


    public static final Map<DecodeHintType, Object> HINTS = new EnumMap<>(DecodeHintType.class);

    static {
        List<BarcodeFormat> allFormats = new ArrayList<>();
        allFormats.add(BarcodeFormat.AZTEC);
        allFormats.add(BarcodeFormat.CODABAR);
        allFormats.add(BarcodeFormat.CODE_39);
        allFormats.add(BarcodeFormat.CODE_93);
        allFormats.add(BarcodeFormat.CODE_128);
        allFormats.add(BarcodeFormat.DATA_MATRIX);
        allFormats.add(BarcodeFormat.EAN_8);
        allFormats.add(BarcodeFormat.EAN_13);
        allFormats.add(BarcodeFormat.ITF);
        allFormats.add(BarcodeFormat.MAXICODE);
        allFormats.add(BarcodeFormat.PDF_417);
        allFormats.add(BarcodeFormat.QR_CODE);
        allFormats.add(BarcodeFormat.RSS_14);
        allFormats.add(BarcodeFormat.RSS_EXPANDED);
        allFormats.add(BarcodeFormat.UPC_A);
        allFormats.add(BarcodeFormat.UPC_E);
        allFormats.add(BarcodeFormat.UPC_EAN_EXTENSION);

        HINTS.put(DecodeHintType.POSSIBLE_FORMATS, allFormats);
        HINTS.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    /**
     * 解析本地图片二维码。
     *
     * @param picturePath 要解析的二维码图片本地路径
     * @return 返回二维码图片里的内容 或 null
     */
    public static void DecodeQRCode(String picturePath, QRCodeDecodeListener listener, ErrorListenner errorListenner) {
        DecodeQRCode(getDecodeAbleBitmap(picturePath), listener, errorListenner);
    }

    /**
     * 解析bitmap二维码。
     *
     * @param bitmap 要解析的二维码图片
     * @return 返回二维码图片里的内容 或 null
     */
    public static void DecodeQRCode(Bitmap bitmap, QRCodeDecodeListener listener, ErrorListenner errorListenner) {
        try {
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            int[] pixels = new int[width * height];
            bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
            RGBLuminanceSource source = new RGBLuminanceSource(width, height, pixels);
            Result result = new MultiFormatReader().decode(new BinaryBitmap(new HybridBinarizer(source)), HINTS);
            listener.onDecodeSuccess(result.getText());
//            return result.getText();
        } catch (Exception e) {
//            return null;
            errorListenner.errorSuccess(e.toString());
        }
    }

    /**
     * 将本地图片文件转换成可解码二维码的 Bitmap。为了避免图片太大，这里对图片进行了压缩。
     *
     * @param picturePath 本地图片文件路径
     * @return
     */
    public static Bitmap getDecodeAbleBitmap(String picturePath) {
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(picturePath, options);
            int sampleSize = options.outHeight / 400;
            if (sampleSize <= 0)
                sampleSize = 1;
            options.inSampleSize = sampleSize;
            options.inJustDecodeBounds = false;

            return BitmapFactory.decodeFile(picturePath, options);
        } catch (Exception e) {
            return null;
        }
    }


    /**
     * 生成二维码图片
     *
     * @param content 原始数据
     * @param width   指定图片宽度
     * @param height  指定图片高度
     * @return 生成的二维码图片
     */
    public static Bitmap createQrCodeBitmap(String content, int width, int height) {
        //核心类， 获取ZXing中的二维码生成类QRCodeWriter
        QRCodeWriter writer = new QRCodeWriter();

        //设置信息的编码格式
        Hashtable<EncodeHintType, String> hints = new Hashtable<>();
        hints.put(EncodeHintType.CHARACTER_SET, "utf-8");

        try {
            /**
             * 调用encode方法之后，会将字符串以二维码的形式显示在矩阵当中
             * 通过调用矩阵的get(x, y)来判断出当前点是否有像素
             */
            BitMatrix bitMatrix = writer.encode(content, BarcodeFormat.QR_CODE, width, height, hints);
            //   bitMatrix = deleteWhite(bitMatrix);//删除白边
            //声明数组用来保存二维码所需要显示颜色值
            width = bitMatrix.getWidth();
            height = bitMatrix.getHeight();
            int[] pixels = new int[width * height];

            //循环遍历矩阵中所有的像素点，并分配黑白颜色值
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    if (bitMatrix.get(j, i)) {//说明当前点有像素
                        pixels[width * i + j] = Color.BLACK;
                    } else {
                        pixels[width * i + j] = Color.WHITE;
                    }
                }
            }

            //根据填充好的颜色值数组，生成新的二维码Bitmap对象并返回
            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
            bitmap.setPixels(pixels, 0, width, 0, 0, width, height);
            return bitmap;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 生成带Logo的二维码
     *
     * @param qrBitmap   二维码图片
     * @param logoBitmap Logo图片
     * @return
     */
    private Bitmap addLogo(Bitmap qrBitmap, Bitmap logoBitmap) {
        int qrBitmapWidth = qrBitmap.getWidth();
        int qrBitmapHeight = qrBitmap.getHeight();
        int logoBitmapWidth = logoBitmap.getWidth();
        int logoBitmapHeight = logoBitmap.getHeight();
        Bitmap blankBitmap = Bitmap.createBitmap(qrBitmapWidth, qrBitmapHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(blankBitmap);
        canvas.drawBitmap(qrBitmap, 0, 0, null);
        // canvas.save(Canvas.ALL_SAVE_FLAG);
        canvas.save();
        float scaleSize = 1.0f;
        while ((logoBitmapWidth / scaleSize) > (qrBitmapWidth / 5) || (logoBitmapHeight / scaleSize) > (qrBitmapHeight / 5)) {
            scaleSize *= 2;
        }
        float sx = 1.0f / scaleSize;
        canvas.scale(sx, sx, qrBitmapWidth / 2, qrBitmapHeight / 2);
        canvas.drawBitmap(logoBitmap, (qrBitmapWidth - logoBitmapWidth) / 2, (qrBitmapHeight - logoBitmapHeight) / 2, null);
        canvas.restore();
        return blankBitmap;
    }


    //////////////////////////////////////
    public static Result decodeImage(String path) {
        Bitmap bitmap = decodeSampledBitmapFromFile(path, 400, 400);
        if (bitmap == null) {
            return null;
        } else {
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            int[] pixels = new int[width * height];
            bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
            PlanarYUVLuminanceSource source1 = new PlanarYUVLuminanceSource(getYUV420sp(width, height, bitmap), width, height, 0, 0, width, height, false);
            BinaryBitmap binaryBitmap = new BinaryBitmap(new HybridBinarizer(source1));
            HashMap hints = new HashMap();
            hints.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
            hints.put(DecodeHintType.CHARACTER_SET, "UTF-8");

            try {
                return (new MultiFormatReader()).decode(binaryBitmap, hints);
            } catch (NotFoundException var9) {
                var9.printStackTrace();
                return null;
            }
        }
    }

    public static Bitmap decodeSampledBitmapFromFile(String imgPath, int reqWidth, int reqHeight) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(imgPath, options);
        options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight);
        options.inJustDecodeBounds = false;
        return BitmapFactory.decodeFile(imgPath, options);
//        return BitmapFactory.decodeFile(imgPath);
    }

    public static int calculateInSampleSize(BitmapFactory.Options options, int reqWidth, int reqHeight) {
        int height = options.outHeight;
        int width = options.outWidth;
        int inSampleSize = 1;
        if (height > reqHeight || width > reqWidth) {
            int halfHeight = height / 2;

            for (int halfWidth = width / 2; halfHeight / inSampleSize > reqHeight && halfWidth / inSampleSize > reqWidth; inSampleSize *= 2) {
                ;
            }
        }

        return inSampleSize;
    }

    private static byte[] yuvs;

    public static byte[] getYUV420sp(int inputWidth, int inputHeight, Bitmap scaled) {
        int[] argb = new int[inputWidth * inputHeight];
        scaled.getPixels(argb, 0, inputWidth, 0, 0, inputWidth, inputHeight);
        int requiredWidth = inputWidth % 2 == 0 ? inputWidth : inputWidth + 1;
        int requiredHeight = inputHeight % 2 == 0 ? inputHeight : inputHeight + 1;
        int byteLength = requiredWidth * requiredHeight * 3 / 2;
        if (yuvs != null && yuvs.length >= byteLength) {
            Arrays.fill(yuvs, (byte) 0);
        } else {
            yuvs = new byte[byteLength];
        }

        encodeYUV420SP(yuvs, argb, inputWidth, inputHeight);
        scaled.recycle();
        return yuvs;
    }

    private static void encodeYUV420SP(byte[] yuv420sp, int[] argb, int width, int height) {
        int frameSize = width * height;
        int yIndex = 0;
        int uvIndex = frameSize;
        int argbIndex = 0;

        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                int R = (argb[argbIndex] & 16711680) >> 16;
                int G = (argb[argbIndex] & '\uff00') >> 8;
                int B = argb[argbIndex] & 255;
                ++argbIndex;
                int Y = (66 * R + 129 * G + 25 * B + 128 >> 8) + 16;
                int U = (-38 * R - 74 * G + 112 * B + 128 >> 8) + 128;
                int V = (112 * R - 94 * G - 18 * B + 128 >> 8) + 128;
                Y = Math.max(0, Math.min(Y, 255));
                U = Math.max(0, Math.min(U, 255));
                V = Math.max(0, Math.min(V, 255));
                yuv420sp[yIndex++] = (byte) Y;
                if (j % 2 == 0 && i % 2 == 0) {
                    yuv420sp[uvIndex++] = (byte) V;
                    yuv420sp[uvIndex++] = (byte) U;
                }
            }
        }

    }

    private static BitMatrix deleteWhite(BitMatrix matrix) {
        int[] rec = matrix.getEnclosingRectangle();
        int resWidth = rec[2] + 1;
        int resHeight = rec[3] + 1;

        BitMatrix resMatrix = new BitMatrix(resWidth, resHeight);
        resMatrix.clear();
        for (int i = 0; i < resWidth; i++) {
            for (int j = 0; j < resHeight; j++) {
                if (matrix.get(i + rec[0], j + rec[1]))
                    resMatrix.set(i, j);
            }
        }
        return resMatrix;
    }

    /**
     * 生成二维码图片
     *
     * @param content 原始数据
     * @param width   指定图片宽度
     * @param height  指定图片高度
     * @param type    二维码的类型
     * @return 生成的二维码图片
     */
    public static List<Bitmap> createMulQrCodeBitmap(String content, int width, int height,
                                                     int type, int transType) {
        List<Bitmap> bitmaps = new ArrayList<>();
        int factor = 350;
        int i = 0;
        while (i < content.length()) {
            int max = i + factor <= content.length() ? i + factor : content.length();
            String tempContent = content.substring(i, max);
            String jsonObject = getQrJson(0, "MultiQrContent", (int) Math.ceil(content.length() / (factor * 1f))
                    , (int) Math.ceil(max / (factor * 1f)), tempContent, MD5Utils.md5Encode(content), type, "ELA", transType);
            Bitmap bitmap = createQrCodeBitmap(jsonObject, width, height);
            bitmaps.add(bitmap);
            i = max;
        }
        return bitmaps;
    }

    private static String getQrJson(int version, String name, int total, int index, String data, String md5, int type, String subWallet, int transType) {

        QrBean qrBean = new QrBean();
        qrBean.setVersion(version);
        qrBean.setName(name);
        qrBean.setTotal(total);
        qrBean.setIndex(index);
        qrBean.setData(data);
        qrBean.setMd5(md5);
        QrBean.ExtraBean extraBean = new QrBean.ExtraBean();
        qrBean.setExtra(extraBean);
        extraBean.setType(type);
        if (TextUtils.isEmpty(subWallet)) {
            subWallet = "ELA";
        }
        extraBean.setSubWallet(subWallet);
        extraBean.setTransType(transType);


        return JSONObject.toJSONString(qrBean);

    }


    public static Bitmap createQrCodeBitmap(String content, int width, int height, int type, String chainID, int transType) {
        String jsonObject = getQrJson(0, "MultiQrContent", 1
                , 1, content, MD5Utils.md5Encode(content), type, chainID, transType);
        return createQrCodeBitmap(jsonObject, width, height);
    }
}
