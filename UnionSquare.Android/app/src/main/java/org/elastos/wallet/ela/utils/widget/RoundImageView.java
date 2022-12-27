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

package org.elastos.wallet.ela.utils.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.support.v7.widget.AppCompatImageView;
import android.util.AttributeSet;

import org.elastos.wallet.R;

public class RoundImageView extends AppCompatImageView {

    private static final int DEFAULT_BORDER_WIDTH = 0;
    private static final int DEFAULT_BORDER_COLOR = Color.WHITE;


    private int mBorderColor;
    private int mBorderWidth;

    private Paint paintImage;
    private Paint paintBorder;
    private float mRadius;
    private boolean mIsCircle;

    public RoundImageView(final Context context) {
        this(context, null);
    }

    public RoundImageView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RoundImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        this.setScaleType(ScaleType.FIT_XY);
        TypedArray ta = context.obtainStyledAttributes(attrs,
                R.styleable.RoundImageView, defStyle, 0);
        mRadius = ta.getDimensionPixelSize(R.styleable.RoundImageView_radius, 0);
        mIsCircle = ta.getBoolean(R.styleable.RoundImageView_circle, false);
        mBorderColor = ta.getColor(R.styleable.RoundImageView_border_color, DEFAULT_BORDER_COLOR);
        mBorderWidth = ta.getDimensionPixelSize(R.styleable.RoundImageView_border_width, DEFAULT_BORDER_WIDTH);
        ta.recycle();
        paintImage = new Paint();
        paintImage.setAntiAlias(true);
        paintBorder = new Paint();
        paintBorder.setAntiAlias(true);
    }


    @Override
    public void onDraw(Canvas canvas) {
        int viewWidth = canvas.getWidth() - getPaddingLeft() - getPaddingRight();
        int viewHeight = canvas.getHeight() - getPaddingTop() - getPaddingBottom();
        Bitmap image = drawableToBitmap(getDrawable());
        Bitmap reSizeImage = reSizeImage(image, viewWidth, viewHeight);
        if(null==reSizeImage) return;
        int imgWidth = reSizeImage.getWidth();
        int imgHight = reSizeImage.getHeight();

        paintBorder.setColor(mBorderColor);
        paintBorder.setStrokeWidth(mBorderWidth);
        paintBorder.setStyle(Paint.Style.FILL_AND_STROKE);

        if (mIsCircle) {
            canvas.drawCircle(viewWidth / 2, viewHeight / 2, (Math.min(viewWidth, viewHeight) / 2) - mBorderWidth / 2, paintBorder);
            BitmapShader bitmapShader = new BitmapShader(reSizeImage, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            paintImage.setShader(bitmapShader);
            canvas.translate(mBorderWidth, mBorderWidth);
            canvas.drawCircle(imgWidth / 2, imgHight / 2, Math.min(imgWidth, imgHight) / 2, paintImage);

        } else {
            RectF rectB = new RectF(mBorderWidth / 2, mBorderWidth / 2, viewWidth - mBorderWidth / 2, viewHeight - mBorderWidth / 2);
            canvas.drawRoundRect(rectB, mRadius, mRadius, paintBorder);

            BitmapShader bitmapShader = new BitmapShader(reSizeImage, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            paintImage.setShader(bitmapShader);

            RectF rect = new RectF(0, 0, imgWidth, imgHight);
            float radius = mBorderWidth == 0 ? mRadius : 0;
            canvas.translate(mBorderWidth, mBorderWidth);
            canvas.drawRoundRect(rect, radius, radius, paintImage);
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = MeasureSpec.getSize(widthMeasureSpec);
        int height = MeasureSpec.getSize(heightMeasureSpec);
        setMeasuredDimension(width, height);
    }

    private Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable == null) {
            return null;
        } else if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }

        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicHeight(),
                drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);
        return bitmap;
    }

    private Bitmap reSizeImage(Bitmap bitmap, int newWidth, int newHeight) {
        try {
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();

            int sideL = Math.min(width, height);
            int x = 0;
            int y = 0;
            if (width > height) {
                x = (width - height) / 2;
            } else {
                y = (height - width) / 2;
            }

            Bitmap bitmapSquare = Bitmap.createBitmap(bitmap, x, y, sideL, sideL);

            width = bitmapSquare.getWidth();
            height = bitmapSquare.getHeight();

            float scaleHeight = (float) (newHeight - (mBorderWidth * 2)) / height;
            float scaleWidth = (float) (newWidth - (mBorderWidth * 2)) / width;
            Matrix matrix = new Matrix();
            matrix.postScale(scaleWidth, scaleHeight);
            return Bitmap.createBitmap(bitmapSquare, 0, 0, width, height, matrix, true);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }
}
