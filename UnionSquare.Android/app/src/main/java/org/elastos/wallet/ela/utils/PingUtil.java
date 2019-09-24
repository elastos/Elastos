package org.elastos.wallet.ela.utils;

import java.io.IOException;
import java.util.List;

public class PingUtil {

    /**
     * 测试ping成功的时间
     *
     * @param addr
     * @return
     */


    public static long pingSuccessTime(String addr) {


        Process proc = null;

        long nowtime = System.currentTimeMillis();

        try {

            String str = "ping -c 1 " + addr;

            System.out.println(str);

            proc = Runtime.getRuntime().exec(str);

            int result = proc.waitFor();
            long pingSuccesstime = System.currentTimeMillis() - nowtime;
            if (result == 0) {
                Log.i("PingUtil", "ping成功" + addr + "///" + pingSuccesstime);
                return pingSuccesstime;
            } else {
                Log.i("PingUtil", "ping失败" + addr + "///" + pingSuccesstime);
                return 55554;
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            proc.destroy();
        }

        return 55554;


    }

    public static String ping(List<String> serverAdd, String defaultAdd) {
        long timeTemp = 55555;
        String result = defaultAdd;

        for (int i = 0; i < serverAdd.size(); i++) {
            long time = pingSuccessTime(serverAdd.get(i).split("//")[1]);
            if (time < timeTemp) {
                result = serverAdd.get(i);
                timeTemp = time;
            }
        }
        return result;
    }
}
