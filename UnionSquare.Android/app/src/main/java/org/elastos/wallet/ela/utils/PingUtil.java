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

import java.io.IOException;
import java.util.List;

public class PingUtil {
    public static boolean ping(String addr) {

        if (addr.contains("//")) {
            addr = addr.split("//")[1];
        }
        Process proc = null;
        try {

            String str = "ping -c 1 " + addr;
            proc = Runtime.getRuntime().exec(str);

            int result = proc.waitFor();

            if (result == 0) {
                Log.i("PingUtil", "ping成功" + addr);
                return true;
            } else {
                Log.i("PingUtil", "ping失败" + addr);
                return false;
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            if (proc != null)
                proc.destroy();
        }

        return false;


    }

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
            if (proc != null)
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
