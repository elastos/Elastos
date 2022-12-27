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

package org.elastos.wallet.ela.utils.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.util.AttributeSet;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.utils.ScreenUtil;

import java.math.BigDecimal;

public class CircleProgressView extends View {
    private Paint paint;
    private float pointX;
    private float pointY;
    private float radius;


    public void setProgress(float progress) {
        this.progress = progress;
        invalidate();
    }

    private float progress = 0;

    public CircleProgressView(Context context) {
        this(context, null);
    }

    public CircleProgressView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CircleProgressView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public CircleProgressView(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();

    }

    private void init() {
        paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setTextSize(ScreenUtil.dp2px(getContext(), 12));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        paint.setColor(ContextCompat.getColor(getContext(), R.color.whiter15));
        drawCircle(canvas, radius);
        paint.setColor(ContextCompat.getColor(getContext(), R.color.white));
        drawArc(canvas);
        paint.setColor(ContextCompat.getColor(getContext(), R.color.blue11));
        drawCircle(canvas, radius - ScreenUtil.dp2px(getContext(), 2));
        drawText(canvas);
    }


    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int width = getMeasuredWidth();
        int height = getMeasuredHeight();
        radius = Math.min(width, height) / 2.0f;
        pointX = width / 2.0f;
        pointY = height / 2.0f;
    }

    private void drawCircle(Canvas canvas, float radius) {

        canvas.drawCircle(pointX, pointY, radius, paint);
    }

    private void drawArc(Canvas canvas) {

        RectF rect = new RectF(0, 0, radius * 2, radius * 2);
        float angle = new BigDecimal(progress).multiply(new BigDecimal(360)).setScale(2, BigDecimal.ROUND_DOWN).floatValue();
        canvas.drawArc(rect, 270, angle, true, paint);
    }

    private void drawText(Canvas canvas) {

        String text = new BigDecimal(progress).multiply(new BigDecimal(100)).setScale(2, BigDecimal.ROUND_DOWN).floatValue() + "%";

        paint.setTextSize(ScreenUtil.dp2px(getContext(), 11));
        //测量字符串长度
        float textLength = paint.measureText(text);

        paint.setColor(ContextCompat.getColor(getContext(), R.color.white));
        //把文本画在圆心居中
        canvas.drawText(text, radius - textLength / 2, radius+5, paint);
    }
}
