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


import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

/**
 * 给java文件批量添加License信息
 *
 * @author tanghc
 */
public class Copyright {

    private static String lineSeperator = System.getProperty("line.separator");
    private static String encode = "UTF-8";

    private String folder;
    private String copyright;

    /**
     * @param folder    java文件夹
     * @param copyright 版权内容
     */
    public Copyright(String folder, String copyright) {
        this.folder = folder;
        this.copyright = copyright;
    }

    public static void main(String[] args) throws IOException {
        // 从文件读取版权内容
        // 在D盘创建一个copyright.txt文件,
        String copyright = "/*\n" +
                " * Copyright (c) 2019 Elastos Foundation\n" +
                " *\n" +
                " * Permission is hereby granted, free of charge, to any person obtaining a copy\n" +
                " * of this software and associated documentation files (the \"Software\"), to deal\n" +
                " * in the Software without restriction, including without limitation the rights\n" +
                " * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n" +
                " * copies of the Software, and to permit persons to whom the Software is\n" +
                " * furnished to do so, subject to the following conditions:\n" +
                " *\n" +
                " * The above copyright notice and this permission notice shall be included in all\n" +
                " * copies or substantial portions of the Software.\n" +
                " *\n" +
                " * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n" +
                " * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n" +
                " * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n" +
                " * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n" +
                " * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n" +
                " * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n" +
                " * SOFTWARE.\n" +
                " */\n";
        // 存放java文件的文件夹,必须是文件夹
        String folder = "./app/src/main/java/org/elastos/wallet/ela";

        new Copyright(folder, copyright).process();

        System.out.println("end.");
    }

    public void process() {
        this.addCopyright(new File(folder));
    }

    private void addCopyright(File folder) {
        File[] files = folder.listFiles();

        if (files == null || files.length == 0) {
            return;
        }

        for (File f : files) {
            if (f.isFile()) {
                doAddCopyright(f);
            } else {
                addCopyright(f);
            }
        }
    }

    private void doAddCopyright(File file) {
        String fileName = file.getName();
        boolean isJavaFile = fileName.toLowerCase().endsWith(".java");
        if (isJavaFile) {
            try {
                this.doWrite(file);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void doWrite(File file) throws IOException {


        StringBuilder javaFileContent = new StringBuilder();
        String line = null;
        // 先添加copyright到文件头
        javaFileContent.append(copyright).append(lineSeperator);
        // 追加剩余内容
        BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(file), encode));
        while ((line = br.readLine()) != null) {
            if (line.contains("Copyright")) {
                br.close();
                return;
            }
            javaFileContent.append(line).append(lineSeperator);
        }

        OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(file), encode);
        writer.write(javaFileContent.toString());
        writer.close();
        br.close();
    }

    private static String readCopyrightFromFile(String copyFilePath) throws IOException {
        StringBuilder copyright = new StringBuilder();

        String line = null;

        BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(copyFilePath), encode));

        while ((line = br.readLine()) != null) {
            copyright.append(line).append(lineSeperator);
        }
        br.close();

        return copyright.toString();
    }

}


