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
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.NumberPicker;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.utils.ScreenUtil;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

public class TextConfigDataPicker extends DatePicker {

    public TextConfigDataPicker(Context context) {
        super(context);
    }

    public TextConfigDataPicker(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TextConfigDataPicker(Context context, AttributeSet attrs, int defStyleAttr) {
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

    private void updateView(View view1) {
        if (view1 instanceof ViewGroup) {
            ViewGroup viewGroup = (ViewGroup) view1;
            List<NumberPicker> numberPickers = findNumberPicker(viewGroup);
            for (NumberPicker np : numberPickers) {
                for (EditText editText : findEditText1(np)) {
                    editText.setTextColor(getResources().getColor(R.color.white));
                    editText.setTextSize(16);
                    editText.setGravity(Gravity.CENTER);
                    resizeNumberPicker(np);
                    np.setWrapSelectorWheel(false);//是否循环
                    np.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);//是否可编辑

                    np.setOnScrollListener(new NumberPicker.OnScrollListener() {
                        @Override
                        public void onScrollStateChange(NumberPicker view, int scrollState) {
                            view.performClick();//刷新选中状态
                        }
                    });
                }
                try {
                    //设置分割线大小颜色
                    Field mSelectionDivider = getFile("mSelectionDivider");
                    mSelectionDivider.set(np, new ColorDrawable(getResources().getColor(R.color.pickline)));
                    //mSelectionDivider.set(np, 1);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }



    private List<EditText> findEditText1(ViewGroup viewGroup) {
        List<EditText> npList = new ArrayList<EditText>();
        View child = null;
        if (null != viewGroup) {
            for (int i = 0; i < viewGroup.getChildCount(); i++) {
                child = viewGroup.getChildAt(i);
                if (child instanceof EditText) {
                    npList.add((EditText) child);

                } else if (child instanceof LinearLayout) {
                    List<EditText> result = findEditText1((ViewGroup) child);
                    if (result.size() > 0) {
                        return result;
                    }
                }
            }
        }
        return npList;
    }

    private EditText findEditText(NumberPicker np) {
        if (null != np) {
            for (int i = 0; i < np.getChildCount(); i++) {
                View child = np.getChildAt(i);

                if (child instanceof EditText) {
                    return (EditText) child;
                }
            }
        }

        return null;
    }


    /**
     * 得到viewGroup 里面的numberpicker组件
     */
    private List<NumberPicker> findNumberPicker(ViewGroup viewGroup) {
        List<NumberPicker> npList = new ArrayList<NumberPicker>();
        View child = null;
        if (null != viewGroup) {
            for (int i = 0; i < viewGroup.getChildCount(); i++) {
                child = viewGroup.getChildAt(i);
                if (child instanceof NumberPicker) {
                    npList.add((NumberPicker) child);

                } else if (child instanceof LinearLayout) {
                    List<NumberPicker> result = findNumberPicker((ViewGroup) child);
                    if (result.size() > 0) {
                        return result;
                    }
                }
            }
        }
        return npList;
    }

    public void updateUI(ViewGroup viewGroup) {
        List<NumberPicker> npList = findNumberPicker(viewGroup);
        for (NumberPicker np : npList) {

            for (EditText editText : findEditText1(np)) {
                editText.setTextColor(getResources().getColor(R.color.white));
                editText.setTextSize(16);
                editText.setGravity(Gravity.CENTER);
                resizeNumberPicker(np);
                np.setWrapSelectorWheel(false);//是否循环
                np.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);//是否可编辑

                np.setOnScrollListener(new NumberPicker.OnScrollListener() {
                    @Override
                    public void onScrollStateChange(NumberPicker view, int scrollState) {
                        view.performClick();//刷新选中状态
                    }
                });
            }
            try {
                //设置分割线大小颜色
                Field mSelectionDivider = getFile("mSelectionDivider");
                mSelectionDivider.set(np, new ColorDrawable(getResources().getColor(R.color.pickline)));
                mSelectionDivider.set(np, 1);
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }

    /**
     * 调整numberpicker大小
     */
    private void resizeNumberPicker(NumberPicker np) {
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(ScreenUtil.dp2px(np.getContext(), 100), ViewGroup.LayoutParams.WRAP_CONTENT);
        params.setMargins(ScreenUtil.dp2px(np.getContext(), 5), 0, ScreenUtil.dp2px(np.getContext(), 5), 0);
        np.setLayoutParams(params);
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
}
