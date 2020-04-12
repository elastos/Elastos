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

import android.content.Context;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class SensitiveWordsUtils {

    /**
     * 敏感词匹配规则
     */
    public static final int MinMatchTYpe = 1;      //最小匹配规则，如：敏感词库["中国","中国人"]，语句："我是中国人"，匹配结果：我是[中国]人
    public static final int MaxMatchType = 2;      //最大匹配规则，如：敏感词库["中国","中国人"]，语句："我是中国人"，匹配结果：我是[中国人]

    /**
     * 敏感词集合
     */
    public static HashMap sensitiveWordMap;

    /**
     * 初始化敏感词库，构建DFA算法模型
     *
     * @param
     */
    public static synchronized void init(Context context) {
        try {
            initSensitiveWordMap();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //这里直接从asstes读敏感词文件，然后加入到set集合
    public static Set<String> readSensitiveWordFile(Context context) throws Exception {
        Set<String> set = null;

        InputStreamReader read = new InputStreamReader(context.getResources().getAssets().open("sensitiveWord"), "UTF-8");
        try {
            set = new HashSet<String>();
            BufferedReader bufferedReader = new BufferedReader(read);
            String txt = null;
            while ((txt = bufferedReader.readLine()) != null) {    //读取文件，将文件内容放入到set中
                set.add(txt);
            }
        } catch (Exception e) {
            throw e;
        } finally {
            read.close();     //关闭文件流
        }
        Log.e("ryan", "set==" + set.toString());
        return set;
    }



    /**
     * 初始化敏感词库，构建DFA算法模型
     */
    private static void initSensitiveWordMap() throws Exception {
        Set<String> sensitiveWordSet = new HashSet<>();
        sensitiveWordSet.add("中");
        sensitiveWordSet.add("中国家");
        //初始化敏感词容器，减少扩容操作
        if (null != sensitiveWordSet && sensitiveWordSet.size() > 0) {
            sensitiveWordMap = new HashMap(sensitiveWordSet.size());
            String key;
            Map nowMap;
            Map<String, String> newWorMap;
            //迭代sensitiveWordSet
            Iterator<String> iterator = sensitiveWordSet.iterator();
            while (iterator.hasNext()) {
                //关键字
                key = iterator.next();
                nowMap = sensitiveWordMap;
                for (int i = 0; i < key.length(); i++) {
                    //转换成char型
                    char keyChar = key.charAt(i);
                    //库中获取关键字
                    Object wordMap = nowMap.get(keyChar);
                    //如果存在该key，直接赋值，用于下一个循环获取
                    if (wordMap != null) {
                        nowMap = (Map) wordMap;
                    } else {
                        //不存在则，则构建一个map，同时将isEnd设置为0，因为他不是最后一个
                        newWorMap = new HashMap<>();
                        //不是最后一个
                        newWorMap.put("isEnd", "0");
                        nowMap.put(keyChar, newWorMap);
                        nowMap = newWorMap;
                    }

                    if (i == key.length() - 1) {
                        //最后一个
                        nowMap.put("isEnd", "1");
                    }
                }
            }
        }

    }

    /**
     * 判断文字是否包含敏感字符
     *
     * @param txt       文字
     * @param matchType 匹配规则 1：最小匹配规则，2：最大匹配规则
     * @return 若包含返回true，否则返回false
     */
    public static boolean contains(String txt, int matchType) {
        boolean flag = false;
        for (int i = 0; i < txt.length(); i++) {
            int matchFlag = checkSensitiveWord(txt, i, matchType); //判断是否包含敏感字符
            if (matchFlag > 0) {    //大于0存在，返回true
                flag = true;
            }
        }
        return flag;
    }

    /**
     * 判断文字是否包含敏感字符
     *
     * @param txt 文字
     * @return 若包含返回true，否则返回false
     */
    public static boolean contains(String txt) {
        return contains(txt, MaxMatchType);
    }

    /**
     * 获取文字中的敏感词
     *
     * @param txt       文字
     * @param matchType 匹配规则 1：最小匹配规则，2：最大匹配规则
     * @return
     */
    public static Set<String> getSensitiveWord(String txt, int matchType) {
        Set<String> sensitiveWordList = new HashSet<>();

        for (int i = 0; i < txt.length(); i++) {
            //判断是否包含敏感字符
            int length = checkSensitiveWord(txt, i, matchType);
            if (length > 0) {//存在,加入list中
                sensitiveWordList.add(txt.substring(i, i + length));
                i = i + length - 1;//减1的原因，是因为for会自增
            }
        }

        return sensitiveWordList;
    }

    /**
     * 获取文字中的敏感词
     *
     * @param txt 文字
     * @return
     */
    public static Set<String> getSensitiveWord(String txt) {
        return getSensitiveWord(txt, MaxMatchType);
    }

    /**
     * 替换敏感字字符
     *
     * @param txt         文本
     * @param replaceChar 替换的字符，匹配的敏感词以字符逐个替换，如 语句：我爱中国人 敏感词：中国人，替换字符：*， 替换结果：我爱***
     * @param matchType   敏感词匹配规则
     * @return
     */
    public static String replaceSensitiveWord(String txt, char replaceChar, int matchType) {
        String resultTxt = txt;
        //获取所有的敏感词
        Set<String> set = getSensitiveWord(txt, matchType);
        Iterator<String> iterator = set.iterator();
        String word;
        String replaceString;
        while (iterator.hasNext()) {
            word = iterator.next();
            replaceString = getReplaceChars(replaceChar, word.length());
            resultTxt = resultTxt.replaceAll(word, replaceString);
        }

        return resultTxt;
    }

    /**
     * 替换敏感字字符
     *
     * @param txt         文本
     * @param replaceChar 替换的字符，匹配的敏感词以字符逐个替换，如 语句：我爱中国人 敏感词：中国人，替换字符：*， 替换结果：我爱***
     * @return
     */
    public static String replaceSensitiveWord(String txt, char replaceChar) {
        return replaceSensitiveWord(txt, replaceChar, MaxMatchType);
    }

    /**
     * 替换敏感字字符
     *
     * @param txt        文本
     * @param replaceStr 替换的字符串，匹配的敏感词以字符逐个替换，如 语句：我爱中国人 敏感词：中国人，替换字符串：[屏蔽]，替换结果：我爱[屏蔽]
     * @param matchType  敏感词匹配规则
     * @return
     */
    public static String replaceSensitiveWord(String txt, String replaceStr, int matchType) {
        String resultTxt = txt;
        //获取所有的敏感词
        Set<String> set = getSensitiveWord(txt, matchType);
        Iterator<String> iterator = set.iterator();
        String word;
        while (iterator.hasNext()) {
            word = iterator.next();
            resultTxt = resultTxt.replaceAll(word, replaceStr);
        }

        return resultTxt;
    }

    /**
     * 替换敏感字字符
     *
     * @param txt        文本
     * @param replaceStr 替换的字符串，匹配的敏感词以字符逐个替换，如 语句：我爱中国人 敏感词：中国人，替换字符串：[屏蔽]，替换结果：我爱[屏蔽]
     * @return
     */
    public static String replaceSensitiveWord(String txt, String replaceStr) {
        return replaceSensitiveWord(txt, replaceStr, MaxMatchType);
    }

    /**
     * 获取替换字符串
     *
     * @param replaceChar
     * @param length
     * @return
     */
    private static String getReplaceChars(char replaceChar, int length) {
        String resultReplace = String.valueOf(replaceChar);
        for (int i = 1; i < length; i++) {
            resultReplace += replaceChar;
        }

        return resultReplace;
    }
    public static void main(String[] s) throws Exception {
        initSensitiveWordMap();
        System.out.println(sensitiveWordMap.toString());
        checkSensitiveWord("你好中国人",0,MinMatchTYpe);
    }
    /**
     * 检查文字中是否包含敏感字符，检查规则如下：<br>
     *
     * @param txt
     * @param beginIndex
     * @param matchType
     * @return 如果存在，则返回敏感词字符的长度，不存在返回0
     */
    private static int checkSensitiveWord(String txt, int beginIndex, int matchType) {
        //敏感词结束标识位：用于敏感词只有1位的情况
        boolean flag = false;
        //匹配标识数默认为0
        int matchFlag = 0;
        char word;
        if (null == sensitiveWordMap) {
            return 0;
        }
        Map nowMap = sensitiveWordMap;

        for (int i = beginIndex; i < txt.length(); i++) {
            word = txt.charAt(i);
            //获取指定key
            nowMap = (Map) nowMap.get(word);
            if (nowMap != null) {//存在，则判断是否为最后一个
                //找到相应key，匹配标识+1
                Log.e("ryan", "nowMap==" + nowMap.toString());
                matchFlag++;
                //如果为最后一个匹配规则,结束循环，返回匹配标识数
                if ("1".equals(nowMap.get("isEnd"))) {
                    //结束标志位为true
                    flag = true;
                    //最小规则，直接返回,最大规则还需继续查找
                    if (MinMatchTYpe == matchType) {
                        break;
                    }
                }
            } else {//不存在，直接返回
                break;
            }
        }
        if (matchFlag < 2 || !flag) {//长度必须大于等于1，为词
            matchFlag = 0;
        }
        return matchFlag;
    }
}
