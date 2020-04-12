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
import android.graphics.drawable.ColorDrawable;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.NumberPicker;

import org.elastos.wallet.R;

import java.lang.reflect.Field;

public class TextConfigNumberPicker extends NumberPicker {

    public TextConfigNumberPicker(Context context) {
        super(context);
    }

    public TextConfigNumberPicker(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TextConfigNumberPicker(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    @Override
    public void addView(View child) {
        this.addView(child, null);


    }

    @Override
    public void addView(View child, ViewGroup.LayoutParams params) {
        this.addView(child, 0, params);
    }

    @Override
    public void addView(View child, int index, ViewGroup.LayoutParams params) {
        super.addView(child, index, params);
        updateView(child);
    }

    private void updateView(View view) {
        if (view instanceof EditText) {
            //设置通用文字的颜色和大小
            ((EditText) view).setTextColor(getResources().getColor(R.color.whiter));
            ((EditText) view).setTextSize(13);
        }
        try {
            //设置分割线大小颜色
            Field mSelectionDivider = this.getFile("mSelectionDivider");
            mSelectionDivider.set(this, new ColorDrawable(getResources().getColor(R.color.pickline)));
           // mSelectionDivider.set(this, 1);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    //反射获取控件 mSelectionDivider mInputText当前选择的view
    public Field getFile(String fieldName) {
        try {
            //设置分割线的颜色值
            Field pickerFields = NumberPicker.class.getDeclaredField(fieldName);
            pickerFields.setAccessible(true);
            return pickerFields;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    //设置选中控件的style  注意一点要在setOnValueChangedListener或 setOnScrollChangeListener中调
    // 用picker.performClick();两种情况动画效果不同
    //同时注意他的代用位置 要在当前控件初始化之后 否则获得不了
    public void setMInputStyle(Float size) {
        Field mInputText = this.getFile("mInputText");
        try {
            ((EditText) mInputText.get(this)).setTextSize(size);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
