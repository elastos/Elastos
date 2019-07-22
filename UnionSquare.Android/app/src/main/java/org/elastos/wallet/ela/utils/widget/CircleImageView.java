package org.elastos.wallet.ela.utils.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.support.annotation.Nullable;
import android.support.v7.widget.AppCompatImageView;
import android.util.AttributeSet;
import android.util.Log;

public class CircleImageView extends AppCompatImageView {
    private float width;
    private float height;
    private float radius;
    private Paint paint;
    private Matrix matrix;

    public CircleImageView(Context context) {
        this(context, null);
    }

    public CircleImageView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CircleImageView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        paint = new Paint();
        paint.setAntiAlias(true);   //设置抗锯齿
        matrix = new Matrix();      //初始化缩放矩阵
    }

    /**
     * 测量控件的宽高，并获取其内切圆的半径
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        width = getMeasuredWidth();
        height = getMeasuredHeight();
        radius = Math.min(width, height) / 2;
        setMeasuredDimension((int) height, (int) height);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Drawable drawable = getDrawable();
        if (drawable == null) {
            super.onDraw(canvas);
            return;
        }
        if (drawable instanceof BitmapDrawable) {
            paint.setShader(initBitmapShader((BitmapDrawable) drawable));//将着色器设置给画笔
            canvas.drawCircle(width / 2, height / 2, radius, paint);//使用画笔在画布上画圆
            return;
        }
        super.onDraw(canvas);
    }

    /**
     * 获取ImageView中资源图片的Bitmap，利用Bitmap初始化图片着色器,通过缩放矩阵将原资源图片缩放到铺满整个绘制区域，避免边界填充
     */
    private BitmapShader initBitmapShader(BitmapDrawable drawable) {
        Bitmap bitmap = drawable.getBitmap();
        float scale = Math.max(width / bitmap.getWidth(), height / bitmap.getHeight());
        Log.d("???0", width + "//" + height);
        Log.d("???1", bitmap.getWidth() + "//" + bitmap.getHeight());
        Log.d("???2", bitmap.getWidth() * scale + "//" + bitmap.getHeight() * scale);
        int wi = bitmap.getWidth();
        int hi = bitmap.getHeight();

        //等比例放大
        matrix.postScale(scale, scale);
        bitmap = Bitmap.createBitmap(bitmap, 0, 0, wi, hi, matrix, true);
        Log.d("???3", bitmap.getWidth() + "//" + bitmap.getHeight());
        //截取放大后的图片 1中心截取  2 截取的宽和高为所放置位置控件宽和高
        bitmap = Bitmap.createBitmap(bitmap, (int) ((wi * scale - width) / 2), (int) ((hi * scale - height) / 2), (int) width, (int) height);//从想开始去with宽度绘制  x>0 x+width<原宽度
        BitmapShader bitmapShader = new BitmapShader(bitmap, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);

        // matrix.setScale(scale, scale);//将图片宽高等比例缩放，避免拉伸 这种方式只适合正方形
        // bitmapShader.setLocalMatrix(matrix);
        return bitmapShader;
    }

}
