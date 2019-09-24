package org.elastos.wallet.ela.utils;

import android.content.Context;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;

public class DateUtil {
    public static String getCurrentData() {
        Date day = new Date();
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return df.format(day);

    }

    public static String time(String sd) {
        try {
            return time(Long.parseLong(sd));
        } catch (NumberFormatException e) {
            return sd;
        }

    }

    public static String time(String sd, Context context) {
        try {
            return time(Long.parseLong(sd), context);
        } catch (NumberFormatException e) {
            return sd;
        }

    }

    public static String time(long sd, Context context) {

        Date dat = new Date(sd * 1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd HH:mm:ss yyyy", Locale.ENGLISH);

        }
        return format.format(gc.getTime());
    }


    public static String time(long sd) {

        Date dat = new Date(sd * 1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;

    }
}
