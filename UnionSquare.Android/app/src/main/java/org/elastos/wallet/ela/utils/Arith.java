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

import java.math.BigDecimal;

/**
 * 由于Java的简单类型不能够精确的对浮点数进行运算，这个工具类提供精
 * 确的浮点数运算，包括加减乘除和四舍五入。
 */
public class Arith {
    //默认除法运算精度
    private static final int DEF_DIV_SCALE = 8;

    //这个类不能实例化
    private Arith() {
    }

    /**
     * 提供精确的加法运算。
     *
     * @param v1 被加数
     * @param v2 加数
     * @return 两个参数的和
     */
    public static BigDecimal add(String v1, String v2) {
        BigDecimal b1 = new BigDecimal(v1);
        BigDecimal b2 = new BigDecimal(v2);
        return b1.add(b2);
    }

    /**
     * 提供精确的减法运算。
     *
     * @param v1 被减数
     * @param v2 减数
     * @return 两个参数的差
     */
    public static BigDecimal sub(String v1, String v2) {
        BigDecimal b1 = new BigDecimal(v1);
        BigDecimal b2 = new BigDecimal(v2);
        return b1.subtract(b2);
    }

    public static BigDecimal sub(String v1, long v2) {
        BigDecimal b1 = new BigDecimal(v1);
        BigDecimal b2 = new BigDecimal(v2);
        return b1.subtract(b2);
    }

    public static BigDecimal sub(BigDecimal b1, String v2) {
        BigDecimal b2 = new BigDecimal(v2);
        return b1.subtract(b2);
    }

    public static BigDecimal sub(Object v1, Object v2) {
        BigDecimal b1;
        if (v1 instanceof Integer) {
            b1 = new BigDecimal((Integer) v1);
        } else if (v1 instanceof BigDecimal) {
            b1 = (BigDecimal) v1;
        } else {
            b1 = new BigDecimal(v1.toString());
        }
        BigDecimal b2;
        if (v2 instanceof Integer) {
            b2 = new BigDecimal((Integer) v2);
        } else if (v2 instanceof BigDecimal) {
            b2 = (BigDecimal) v2;
        } else {
            b2 = new BigDecimal(v2.toString());
        }
        return b1.subtract(b2);
    }

    /**
     * 提供精确的乘法运算。
     *
     * @param v1 被乘数
     * @param v2 乘数
     * @return 两个参数的积
     */
    public static BigDecimal mul(String v1, String v2) {
        BigDecimal b1 = new BigDecimal(v1);
        BigDecimal b2 = new BigDecimal(v2);
        return b1.multiply(b2);
    }

    /**
     * 提供精确的乘法运算。取整
     *
     * @param v1 被乘数
     * @param v2 乘数
     * @return 两个参数的积
     */
    public static BigDecimal mulRemoveZero(String v1, String v2) {
        return mul(v1, v2).setScale(0, BigDecimal.ROUND_DOWN);
    }

    public static BigDecimal mul(Object v1, Object v2) {
        BigDecimal b1;
        BigDecimal b2;
        if (v1 instanceof BigDecimal) {
            b1 = (BigDecimal) v1;
        } else if (v1 instanceof Integer) {
            b1 = new BigDecimal((Integer) v1);
        } else {
            b1 = new BigDecimal(v1.toString());
        }
        if (v2 instanceof BigDecimal) {
            b2 = (BigDecimal) v2;
        } else if (v2 instanceof Integer) {
            b2 = new BigDecimal((Integer) v2);
        } else {
            b2 = new BigDecimal(v2.toString());
        }
        return b1.multiply(b2);
    }

    /**
     * 提供（相对）精确的除法运算，当发生除不尽的情况时，精确到
     * 小数点以后10位，以后的数字四舍五入。
     *
     * @param v1 被除数
     * @param v2 除数
     * @return 两个参数的商
     */
    public static BigDecimal div(String v1, String v2) {
        return div(v1, v2, DEF_DIV_SCALE);
    }

    public static BigDecimal div(Object v1, Object v2, int wei) {
        BigDecimal b1;
        if (v1 instanceof Integer) {
            b1 = new BigDecimal((Integer) v1);
        } else if (v1 instanceof BigDecimal) {
            b1 = (BigDecimal) v1;
        } else {
            b1 = new BigDecimal(v1.toString());
        }
        BigDecimal b2;
        if (v2 instanceof Integer) {
            b2 = new BigDecimal((Integer) v2);
        } else if (v2 instanceof BigDecimal) {
            b2 = (BigDecimal) v2;
        } else {
            b2 = new BigDecimal(v2.toString());
        }
        return b1.divide(b2, wei, BigDecimal.ROUND_DOWN);
    }

    /**
     * 提供（相对）精确的除法运算。当发生除不尽的情况时，由scale参数指
     * 定精度，以后的数字四舍五入。
     *
     * @param v1    被除数
     * @param v2    除数
     * @param scale 表示表示需要精确到小数点以后几位。
     * @return 两个参数的商
     */
    public static BigDecimal div(String v1, String v2, int scale) {
        if (scale < 0) {
            return new BigDecimal("-1");
            /*throw new IllegalArgumentException(
                    "The scale must be a positive integer or zero");*/
        }
        if ("0".equals(v2.trim())) {
            return new BigDecimal("0");
        }
        BigDecimal b1 = new BigDecimal(v1);
        BigDecimal b2 = new BigDecimal(v2);

        return b1.divide(b2, scale, BigDecimal.ROUND_DOWN);
    }

    public static BigDecimal div(BigDecimal v1, int v2, int scale) {
        if (scale < 0) {
            return new BigDecimal("-1");
            /*throw new IllegalArgumentException(
                    "The scale must be a positive integer or zero");*/
        }
        if (0 == v2) {
            return new BigDecimal("0");
        }
        BigDecimal b2 = new BigDecimal(v2);

        return v1.divide(b2, scale, BigDecimal.ROUND_DOWN);
    }
    public static BigDecimal div(BigDecimal v1, String v2, int scale) {
        if (scale < 0) {
            return new BigDecimal("-1");
            /*throw new IllegalArgumentException(
                    "The scale must be a positive integer or zero");*/
        }

        BigDecimal b2 = new BigDecimal(v2);

        return v1.divide(b2, scale, BigDecimal.ROUND_DOWN);
    }
    /**
     * 提供精确的小数位四舍五入处理。
     *
     * @param v     需要四舍五入的数字
     * @param scale 小数点后保留几位
     * @return 四舍五入后的结果
     */
    public static double round(double v, int scale) {
        if (scale < 0) {
            throw new IllegalArgumentException(
                    "The scale must be a positive integer or zero");
        }
        BigDecimal b = new BigDecimal(Double.toString(v));
        BigDecimal one = new BigDecimal("1");
        return b.divide(one, scale, BigDecimal.ROUND_HALF_UP).doubleValue();
    }
};
