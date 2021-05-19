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

import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.widget.EditText;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.inject.Inject;

/**
 * 正则工具类
 * Created by wangdongfeng on 2018/4/24.
 */

public class MatcherUtil {

    @Inject
    public MatcherUtil() {
    }

    /*检查钱包地址的*/
    public boolean isMatcherAddr(String str) {
        if (TextUtils.isEmpty(str)) {
            return false;
        }
        String pattern = "^0x[\\w]{40}$";
        return Pattern.matches(pattern, str);
    }

    /*检查钱包私钥的*/
    public boolean isMatcherPrivateKey(String str) {
        String pattern = "^0x[\\w]{64}$";
        return Pattern.matches(pattern, str);
    }

    /*检测身份证*/
    public static final String REGEX_ID_CARD = "(^\\d{15}$)|(^\\d{17}([0-9]|X|x)$)";

    public boolean idcardChenck(String idCard) {
        return Pattern.matches(REGEX_ID_CARD, idCard);
    }

    public boolean idcardChenck(String idCard, String areaCode) {
        if (TextUtils.isEmpty(idCard)) {
            return false;
        } else if ("0086".equals(areaCode)) {
            return Pattern.matches(REGEX_ID_CARD, idCard);
        }
        return true;
    }

    public String replaceBlank(String src) {
        String dest = "";
        if (src != null) {
            Pattern pattern = Pattern.compile("\t|\r|\n|\\s*");
            Matcher matcher = pattern.matcher(src);
            dest = matcher.replaceAll("");
        }
        return dest;
    }

    public static String time(long sd) {

        Date dat = new Date(sd);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;
    }

    public static String time(String sd) {

        Date dat = new Date(Long.parseLong(sd));
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;
    }


    public static String time_nyr(String sd) {

        Date dat = new Date(Long.parseLong(sd));
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        String sb = format.format(gc.getTime());
        return sb;
    }

    public static String timeStamp2Date(String seconds, String format) {
        if (seconds == null || seconds.isEmpty() || seconds.equals("null")) {
            return "";
        }
        if (format == null || format.isEmpty()) {
            format = "yyyy-MM-dd HH:mm:ss";
        }
        SimpleDateFormat sdf = new SimpleDateFormat(format);
        return sdf.format(new Date(Long.valueOf(seconds)));
    }

    /**
     * 日期格式字符串转换成时间戳
     *
     * @param date_str 字符串日期
     * @param format   如：yyyy-MM-dd HH:mm:ss
     * @return
     */
    public static String date2TimeStamp(String date_str, String format) {
        try {
            SimpleDateFormat sdf = new SimpleDateFormat(format);
            return String.valueOf(sdf.parse(date_str).getTime());
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    public static String date2Date(String sd) {
        String date = date2TimeStamp(sd, "yyyy-MM-dd HH:mm:ss");
        return timeStamp2Date(date, "yyyy-MM-dd");
    }

    /**
     * 判断s长度是否小于count
     *
     * @param s
     * @return
     */
    public int getWordCountRegex(String s) {
        ///^[A-Za-z0-9\u4e00-\u9fa5]+$/
        s = s.replaceAll("[^\\x00-\\xff]", "*");
        return s.length();
    }

    public static String sub(String data) {
        return data.substring(0, 9) + "..." + data.substring(data.length() - 9, data.length());

    }

    public static boolean isRightName(String s) {
        ///^[A-Za-z0-9\u4e00-\u9fa5]+$/
        //"[\u4e00-\u9fa5_a-zA-Z0-9_]{2,6}"
        //String pattern = "[\u4e00-\u9fa5\\w]+";
        return (Pattern.matches("[\u4e00-\u9fa5a-zA-Z0-9]{1,16}", s));
    }

    public static boolean isEmoji(String string) {
        //过滤Emoji表情
        Pattern p = Pattern.compile("[^\\u0000-\\uFFFF]");
        //过滤Emoji表情和颜文字
        //Pattern p = Pattern.compile("[\\ud83c\\udc00-\\ud83c\\udfff]|[\\ud83d\\udc00-\\ud83d\\udfff]|[\\u2600-\\u27ff]|[\\ud83e\\udd00-\\ud83e\\uddff]|[\\u2300-\\u23ff]|[\\u2500-\\u25ff]|[\\u2100-\\u21ff]|[\\u0000-\\u00ff]|[\\u2b00-\\u2bff]|[\\u2d06]|[\\u3030]");
        Matcher m = p.matcher(string);
        return m.find();
    }

    public static InputFilter filter() {

        return new InputFilter() {

            Pattern pattern = Pattern.compile("[^a-zA-Z0-9\\u4E00-\\u9FA5_,.?!:;…~_\\-\"\"/@*+'()<>{}/[/]()<>{}\\[\\]=%&$|\\/♀♂#¥£¢€\"^` ，。？！：；……～“”、“（）”、（——）‘’＠‘·’＆＊＃《》￥《〈〉》〈＄〉［］￡［］｛｝｛｝￠【】【】％〖〗〖〗／〔〕〔〕＼『』『』＾「」「」｜﹁﹂｀．]");

            @Override
            public CharSequence filter(CharSequence charSequence, int i, int i1, Spanned spanned, int i2, int i3) {
                Matcher matcher = pattern.matcher(charSequence);
                if (!matcher.find()) {
                    return null;
                } else {
                    //   Toast.makeText(context, "非法字符！", Toast.LENGTH_SHORT).show();
                    return "";
                }

            }
        };


    }

    /*source: CharSequence, //将要输入的字符串,如果是删除操作则为空字符串
    start: Int, //将要输入的字符串起始下标，一般为0
    end: Int, //start + source字符的长度
    dest: Spanned, //输入之前文本框中的内容
    dstart: Int, //将会被替换的起始位置
    dend: Int //dstart+将会被替换的字符串长度
    返回值 ：CharSequence //方法返回的值将会替换掉dest字符串中dstartd位置到dend位置之间字符，返回source表示不做任何处理，返回空字符串""表示不输入任何字符
 **/
    public static InputFilter filter(int digits) {
// beizhu.setFilters(new InputFilter[]{MatcherUtil.filter(getContext())});
        return new InputFilter() {
            public CharSequence filter(CharSequence source, int start, int end,
                                       Spanned dest, int dstart, int dend) {
                // 删除等特殊字符，直接返回
                if ("".equals(source.toString())) {
                    return null;
                }
                String dValue = dest.toString();
                String[] splitArray = dValue.split("\\.");
                if (splitArray.length > 1) {
                    String dotValue = splitArray[1];
                    int diff = dotValue.length() + 1 - digits;
                    if (diff > 0) {
                        return source.subSequence(start, end - diff);
                    }
                }
                return null;
            }
        };
    }

    /**
     * 限制edittext字节数
     * @param editText
     * @param wei 字节数
     */
    public static void editTextFormat(EditText editText, int wei) {
        editText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (editText.getTag() != null && (boolean) editText.getTag()) {
                    editText.setTag(false);
                    return;
                }
                if (!TextUtils.isEmpty(s)) {
                    String original = s.toString().trim();
                    if (original.getBytes().length <= wei) {
                        return;
                    }
                    StringBuffer res = new StringBuffer();
                    for (int i = 0; i < original.length(); i++) {
                        char c = original.charAt(i);
                        if (String.valueOf(c).getBytes().length + res.toString().getBytes().length <= wei) {
                            res = res.append(c);
                        } else {
                            break;
                        }

                    }
                    editText.setTag(true);
                    editText.setText(res.toString());
                    editText.setSelection(res.length());


                }
            }
        });
    }



}
