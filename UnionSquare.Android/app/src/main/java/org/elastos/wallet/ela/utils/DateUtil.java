package org.elastos.wallet.ela.utils;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;

public class DateUtil {
    public static String getCurrentData() {
        Date day = new Date();
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return df.format(day);

    }
    public static String time(String sd) {

        Date dat = new Date(Long.parseLong(sd)*1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
     SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;
    }
    public static String time(long sd) {

        Date dat = new Date(sd*1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
   SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;

    }
    public static String time1(long sd) {

        Date dat = new Date(sd*1000L);
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime(dat);
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String sb = format.format(gc.getTime());
        return sb;

    }}
