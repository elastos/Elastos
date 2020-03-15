package org.elastos.wallet.ela.utils;

import android.content.Context;
import android.text.TextUtils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;

public class DateUtil {
    public static final String FORMART1 = "yyyy-MM-dd HH:mm:ss";
    public static final String FORMART2 = "yyyy/MM/dd";
    public static final String FORMART3 = "yyyy-MM-dd";

    public static String getCurrentData() {
        Date day = new Date();
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return df.format(day);

    }

    public static String getCurrentData(String formant) {
        Date day = new Date();
        SimpleDateFormat df = new SimpleDateFormat(formant);
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

    private static Date parseDate(String sd) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        try {
            Date data = format.parse(sd);
            return data;
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static long parseToLong(String sd) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        try {
            Date data = format.parse(sd);
            return data.getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return 0;
    }

    public static Date parseToDate(String sd) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        try {
            return format.parse(sd);
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static String parseToLongWithLanguage(String sd, Context context) {
        if (TextUtils.isEmpty(sd)) {
            return null;
        }
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        int Language = new SPUtil(context).getLanguage();
        try {
            if (Language != 0) {
                format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);
            }
            return format.parse(sd).getTime() + "";
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static String timeNYR(long time, Context context, boolean isSecond) {
        if (time == 0) return null;
        if (isSecond) {
            time *= 1000L;
        }
        Date dat = new Date(time);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(dat);
    }

    public static String timeNYR(String time, Context context, boolean isSecond) {
        if (TextUtils.isEmpty(time)) {
            return null;
        }
        long milliseconds;
        if (isSecond) {
            milliseconds = Long.parseLong(time) * 1000L;

        } else {
            milliseconds = Long.parseLong(time);
        }
        Date dat = new Date(milliseconds);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(dat);
    }

    public static String timeNYR(long time, Context context) {
        if (time == 0) return null;
        Date dat = new Date(time * 1000L);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(dat);
    }

    public static String timeNYR(String time, Context context) {
        Date dat = new Date(Long.parseLong(time) * 1000L);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(dat);
    }

    public static String timeNYR(Date date, Context context) {
        if (date == null) {
            return "";
        }
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
        if (Language != 0) {
            format = new SimpleDateFormat("MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(date);
    }

    public static String time(long sd, Context context) {
        if (sd == 0) return null;
        Date dat = new Date(sd * 1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
        if (Language != 0) {
            format = new SimpleDateFormat("HH:mm:ss MMM dd yyyy", Locale.ENGLISH);

        }
        return format.format(gc.getTime());
    }

    public static String time1(long sd, Context context) {

        Date dat = new Date(sd * 1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        int Language = new SPUtil(context).getLanguage();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        if (Language != 0) {
            format = new SimpleDateFormat("HH:mm:ss MMM dd yyyy", Locale.ENGLISH);

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
