package org.elastos.wallet.ela.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.InputStreamReader;

public class FileUtile {
    public static boolean delFile(File file) {
        if (!file.exists()) {
            return false;
        }

        if (file.isDirectory()) {
            File[] files = file.listFiles();
            for (File f : files) {
                delFile(f);
            }
        }
        return file.delete();
    }


    /*
     * 生成文件
     * @param file 文件路径+文件名称
     * @param conent 要生成的文件内容
     */
    public static boolean writeFile(File file, String conent) {
        try {
            if (!file.exists()) {
                file.createNewFile();
            }
            FileWriter writer = new FileWriter(file);
            writer.write(conent);
            writer.close();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public static String readFile(File file) {
        InputStreamReader read = null;// 考虑到编码格式
        StringBuffer result = new StringBuffer();
        try {
            read = new InputStreamReader(new FileInputStream(file), "utf-8");

            BufferedReader bufferedReader = new BufferedReader(read);
            String lineTxt = null;
            while ((lineTxt = bufferedReader.readLine()) != null) {
                result.append(lineTxt);
            }

            read.close();
            return result.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

}
