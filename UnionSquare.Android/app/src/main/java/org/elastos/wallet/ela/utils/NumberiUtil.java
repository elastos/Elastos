package org.elastos.wallet.ela.utils;

import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.widget.EditText;

import java.math.BigDecimal;
import java.util.regex.Pattern;

public class NumberiUtil {
    /**
     * @param number 数
     * @param wei    保留小数点几位
     * @return
     */
    public static String numberFormat(String number, int wei) {
        BigDecimal b = new BigDecimal(number);
        return removeZero(b.setScale(wei, BigDecimal.ROUND_DOWN).toPlainString());
    }

    public static String numberFormat(Object number, int wei) {
        BigDecimal b;
        if (number instanceof BigDecimal) {
            b = (BigDecimal) number;
        } else {
            b = new BigDecimal(number.toString());
        }
        return removeZero(b.setScale(wei, BigDecimal.ROUND_DOWN).toPlainString());
    }

    private static String removeZero(String number) {
        while ((number.endsWith("0") || number.endsWith(".")) && number.contains(".")) {
            number = number.substring(0, number.length() - 1);
        }
        return number;
    }

    /* public static BigDecimal setScal(String f, int newScale) {
         BigDecimal b = new BigDecimal(f);
         String newScalStr = b.setScale(newScale, 4).toString();
         return new BigDecimal(newScalStr);
     }*/
    public static boolean isNumber(String s) {
        return (!TextUtils.isEmpty(s)) && Pattern.matches("^\\d+(\\.\\d+)?", s);
    }

    public static String setPoint2Null(EditText etAmount, Editable s) {
        String temp = s.toString().trim();
        if (".".equals(temp)) {
            etAmount.setText("");
            temp = null;
        }
        return temp;
    }

    public static String maxNumberFormat(String number, int wei) {
        number = number.trim();
        if (number.contains("E") || number.contains("e")) {
            number = new BigDecimal(number).toPlainString();
        }
        number = removeZero(number);
        if (number.split("\\.").length > 1) {
            String part1 = (number.split("\\."))[0];//整数部分
            String part2 = number.split("\\.")[1];//小数部分

            if (part1.length() >= wei) {
                return part1;
            }

            return part1 + "." + part2.substring(0, part2.length() > wei - part1.length() ? wei - part1.length() : part2.length());

        } else {
            return number;
        }

    }

    public static String maxNumberFormat(BigDecimal number1, int wei) {
        String number = number1.toPlainString();
        number = removeZero(number);
        if (number.contains(".")) {
            String part1 = (number.split("\\."))[0];//整数部分
            String part2 = number.split("\\.")[1];//小数部分

            if (part1.length() >= wei) {
                return part1;
            }

            return part1 + "." + part2.substring(0, part2.length() > wei - part1.length() ? wei - part1.length() : part2.length());

        } else {
            return number;
        }

    }

    public static void editTestFormat(EditText etBalance, int wei) {
        etBalance.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (etBalance.getTag() != null && (boolean) etBalance.getTag()) {
                    etBalance.setTag(false);
                    return;
                }
                if (!TextUtils.isEmpty(s)) {
                    String number = s.toString().trim();
                    if (number.startsWith(".")) {
                        etBalance.setTag(true);
                        etBalance.setText(null);
                        return;
                    }
                    if (number.split("\\.").length > 1 && number.split("\\.")[1].length() > wei) {
                        number = (number.split("\\."))[0] + "." + number.split("\\.")[1].substring(0, wei);
                        etBalance.setTag(true);
                        etBalance.setText(number);
                        etBalance.setSelection(number.length());
                    }
                }
            }
        });
    }
}
