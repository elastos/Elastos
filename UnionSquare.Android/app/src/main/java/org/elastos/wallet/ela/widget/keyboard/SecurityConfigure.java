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

package org.elastos.wallet.ela.widget.keyboard;

import android.graphics.Color;

/**
 * 安全键盘相关配置
 *
 * @author yidong (onlyloveyd@gmaol.com)
 * @date 2018/6/22 07:45
 */
public class SecurityConfigure {

    /**
     * 键盘类型选中颜色
     */
    private int selectedColor = 0xff66aeff;

    /**
     * 键盘类型未选中颜色
     */
    private int unselectedColor = Color.BLACK;

    /**
     * 数字键盘使能
     */
    private boolean isNumberEnabled = true;

    /**
     * 字母键盘使能
     */
    private boolean isLetterEnabled = true;

    /**
     * 符号键盘使能
     */
    private boolean isSymbolEnabled = true;

    /**
     * 默认选中键盘
     */
    private KeyboardType defaultKeyboardType = KeyboardType.LETTER;

    public SecurityConfigure() {
    }

    public int getSelectedColor() {
        return selectedColor;
    }

    public SecurityConfigure setSelectedColor(int selectedColor) {
        this.selectedColor = selectedColor;
        return this;
    }

    public int getUnselectedColor() {
        return unselectedColor;
    }

    public SecurityConfigure setUnselectedColor(int unselectedColor) {
        this.unselectedColor = unselectedColor;
        return this;
    }

    public boolean isNumberEnabled() {
        return isNumberEnabled;
    }

    public SecurityConfigure setNumberEnabled(boolean numberEnabled) {
        this.isNumberEnabled = numberEnabled;
        return this;
    }

    public boolean isLetterEnabled() {
        return isLetterEnabled;
    }

    public SecurityConfigure setLetterEnabled(boolean letterEnabled) {
        this.isLetterEnabled = letterEnabled;
        return this;
    }

    public boolean isSymbolEnabled() {
        return isSymbolEnabled;
    }

    public SecurityConfigure setSymbolEnabled(boolean symbolEnabled) {
        this.isSymbolEnabled = symbolEnabled;
        return this;
    }

    public KeyboardType getDefaultKeyboardType() {
        return defaultKeyboardType;
    }

    public SecurityConfigure setDefaultKeyboardType(KeyboardType defaultKeyboardType) {
        this.defaultKeyboardType = defaultKeyboardType;
        return this;
    }
}
