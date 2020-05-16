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
import android.text.TextUtils;
import android.view.ViewGroup;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.widget.keyboard.KeyboardType;
import org.elastos.wallet.ela.widget.keyboard.SecurityConfigure;
import org.elastos.wallet.ela.widget.keyboard.SecurityKeyboard;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;
import java.util.Random;
import java.util.regex.Pattern;

public class AppUtlis {
    //检测密码
    public static boolean chenckString(String mima) {
        //8-16个字符须包含字母、数字、特殊字符中至少两种
        Pattern p = Pattern.compile("^(?![A-Z]*$)(?![a-z]*$)(?![0-9]*$)(?![^a-zA-Z0-9]*$)\\S{8,16}$");

        return Pattern.matches(p.pattern(), mima);
    }

    //打开自定义 键盘
    public static void securityKeyboard(ViewGroup viewGroup) {
        SecurityConfigure configure = new SecurityConfigure().setDefaultKeyboardType(KeyboardType.NUMBER);
        new SecurityKeyboard(viewGroup, configure);
    }

    //生成随机数字和字母 masterWalletID
    public static String getStringRandom(int length) {

        String val = "";
        Random random = new Random();

        //参数length，表示生成几位随机数
        for (int i = 0; i < length; i++) {

            String charOrNum = random.nextInt(2) % 2 == 0 ? "char" : "num";
            //输出字母还是数字
            if ("char".equalsIgnoreCase(charOrNum)) {
                //输出是大写字母还是小写字母
                int temp = random.nextInt(2) % 2 == 0 ? 65 : 97;
                val += (char) (random.nextInt(26) + temp);
            } else if ("num".equalsIgnoreCase(charOrNum)) {
                val += String.valueOf(random.nextInt(10));
            }
        }
        return val;
    }

    public static String getLocCode(String area) {
        if (TextUtils.isEmpty(area)) {
            return null;
        }

        List<Area> listArea = CacheUtil.getArea();
        String adder = null;
        if (listArea != null) {
            for (int i = 0; i < listArea.size(); i++) {
                if (area.equals(listArea.get(i).getEn()) || area.equals(listArea.get(i).getZh())) {
                    adder = listArea.get(i).getCode() + "";
                }
            }
        }

        return TextUtils.isEmpty(adder) ? null : adder;
    }

    public static String getLoc(Context context, String code) {
        if (TextUtils.isEmpty(code)) {
            return null;
        }
//        String text = "{'244': '安哥拉', '93': '阿富汗', '355': '阿尔巴尼亚', '213': '阿尔及利亚', '376': '安道尔共和国', '1264': '安圭拉岛', '1268': '安提瓜和巴布达', '54': '阿根廷', '374': '亚美尼亚', '247': '阿森松', '61': '澳大利亚', '43': '奥地利', '994': '阿塞拜疆', '1242': '巴哈马', '973': '巴林', '880': '孟加拉国', '1246': '巴巴多斯', '375': '白俄罗斯', '32': '比利时', '501': '伯利兹', '229': '贝宁', '1441': '百慕大群岛', '591': '玻利维亚', '267': '博茨瓦纳', '55': '巴西', '673': '文莱', '359': '保加利亚', '226': '布基纳法索', '95': '缅甸', '257': '布隆迪', '237': '喀麦隆', '1': '美国', '1345': '开曼群岛', '236': '中非共和国', '235': '乍得', '56': '智利', '86': '中国', '57': '哥伦比亚', '242': '刚果', '682': '库克群岛', '506': '哥斯达黎加', '53': '古巴', '357': '塞浦路斯', '420': '捷克', '45': '丹麦', '253': '吉布提', '1890': '多米尼加共和国', '593': '厄瓜多尔', '20': '埃及', '503': '萨尔瓦多', '372': '爱沙尼亚', '251': '埃塞俄比亚', '679': '斐济', '358': '芬兰', '33': '法国', '594': '法属圭亚那', '241': '加蓬', '220': '冈比亚', '995': '格鲁吉亚', '49': '德国', '233': '乌兹别克斯坦', '350': '直布罗陀', '30': '希腊', '1809': '特立尼达和多巴哥', '1671': '关岛', '502': '危地马拉', '224': '几内亚', '592': '圭亚那', '509': '海地', '504': '洪都拉斯', '852': '香港', '36': '匈牙利', '354': '冰岛', '91': '印度', '62': '印度尼西亚', '98': '伊朗', '964': '伊拉克', '353': '爱尔兰', '972': '以色列', '39': '意大利', '225': '科特迪瓦', '1876': '牙买加', '81': '日本', '962': '约旦', '855': '柬埔寨', '327': '哈萨克斯坦', '254': '肯尼亚', '82': '韩国', '965': '科威特', '331': '吉尔吉斯坦', '856': '老挝', '371': '拉脱维亚', '961': '黎巴嫩', '266': '莱索托', '231': '利比里亚', '218': '利比亚', '423': '列支敦士登', '370': '立陶宛', '352': '卢森堡', '853': '澳门', '261': '马达加斯加', '265': '马拉维', '60': '马来西亚', '960': '马尔代夫', '223': '马里', '356': '马耳他', '1670': '马里亚那群岛', '596': '马提尼克', '230': '毛里求斯', '52': '墨西哥', '373': '摩尔多瓦', '377': '摩纳哥', '976': '蒙古', '1664': '蒙特塞拉特岛', '212': '摩洛哥', '258': '莫桑比克', '264': '纳米比亚', '674': '瑙鲁', '977': '尼泊尔', '599': '荷属安的列斯', '31': '荷兰', '64': '新西兰', '505': '尼加拉瓜', '227': '尼日尔', '234': '尼日利亚', '850': '朝鲜', '47': '挪威', '968': '阿曼', '92': '巴基斯坦', '507': '巴拿马', '675': '巴布亚新几内亚', '595': '巴拉圭', '51': '秘鲁', '63': '菲律宾', '48': '波兰', '689': '法属玻利尼西亚', '351': '葡萄牙', '1787': '波多黎各', '974': '卡塔尔', '262': '留尼旺', '40': '罗马尼亚', '7': '俄罗斯', '1758': '圣卢西亚', '1784': '圣文森特', '684': '东萨摩亚(美)', '685': '西萨摩亚', '378': '圣马力诺', '239': '圣多美和普林西比', '966': '沙特阿拉伯', '221': '塞内加尔', '248': '塞舌尔', '232': '塞拉利昂', '65': '新加坡', '421': '斯洛伐克', '386': '斯洛文尼亚', '677': '所罗门群岛', '252': '索马里', '27': '南非', '34': '西班牙', '94': '斯里兰卡', '249': '苏丹', '597': '苏里南', '268': '斯威士兰', '46': '瑞典', '41': '瑞士', '963': '叙利亚', '886': '台湾省', '992': '塔吉克斯坦', '255': '坦桑尼亚', '66': '泰国', '228': '多哥', '676': '汤加', '216': '突尼斯', '90': '土耳其', '993': '土库曼斯坦', '256': '乌干达', '380': '乌克兰', '971': '阿拉伯联合酋长国', '44': '英国', '598': '乌拉圭', '58': '委内瑞拉', '84': '越南', '967': '也门', '381': '南斯拉夫', '263': '津巴布韦', '243': '扎伊尔', '260': '赞比亚'}";
//        Gson gson = new Gson();
//        Map<String, Object> map = new HashMap<String, Object>();
//        map = gson.fromJson(text, map.getClass());
//        String goodsid = (String) map.get(code);
//        return !TextUtils.isEmpty(goodsid) ? goodsid : "未知地区";
        if (code.startsWith("00")) {
            code = code.substring(2, code.length());
        }
        List<Area> listArea = CacheUtil.getArea();
        int Language = new SPUtil(context).getLanguage();

        String adder = null;
        if (listArea != null) {
            for (int i = 0; i < listArea.size(); i++) {
                if (code.equals(listArea.get(i).getCode() + "")) {
                    adder = Language == 0 ? listArea.get(i).getZh() : listArea.get(i).getEn();
                }
            }
        }

        return TextUtils.isEmpty(adder) ? context.getString(R.string.unknown) : adder;
    }

    public static boolean urlReg(String url) {

        String regex = "(((https|http)?://)?([a-z0-9]+[.])|(www.))"
                + "\\w+[.|\\/]([a-z0-9]{0,})?[[.]([a-z0-9]{0,})]+((/[\\S&&[^,;\u4E00-\u9FA5]]+)+)?([.][a-z0-9]{0,}+|/?)";//设置正则表达式

        Pattern pattern = Pattern.compile(regex);

        return pattern.matcher(url).matches();

    }

    public static void getArea(Context context, OnGetAreaListner onGetAreaListner) {
        List<Area> listArea = CacheUtil.getArea();
        if (listArea == null || listArea.size() == 0) {
            new Thread() {
                @Override
                public void run() {
                    try {
                        InputStreamReader inputReader = new InputStreamReader(context.getResources().getAssets().open("area"));
                        BufferedReader bufReader = new BufferedReader(inputReader);
                        String line = "";
                        String Result = "";
                        while ((line = bufReader.readLine()) != null)
                            Result += line;
                        Gson gson1 = new Gson();
                        List<Area> listArea = gson1.fromJson(Result, new TypeToken<List<Area>>() {
                        }.getType());
                        // Collections.sort(listArea);
                        CacheUtil.setArea(listArea);
                        if (onGetAreaListner != null) {
                            onGetAreaListner.onGetArea(listArea);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }.start();
        } else {
            if (onGetAreaListner != null) {
                onGetAreaListner.onGetArea(listArea);
            }
        }
    }

    public interface OnGetAreaListner {
        void onGetArea(List<Area> listArea);
    }

    public static boolean ipCheck(String str) {
        if (str != null && !str.isEmpty()) {
            // 定义正则表达式
            String regex = "^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\." +
                    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\." +
                    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\." +
                    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$";

            // 判断ip地址是否与正则表达式匹配
            if (str.matches(regex))
                return true;
            else
                return false;
        }
        return false;
    }

    public static boolean isURL(String str) {
        //转换为小写
        str = str.toLowerCase();
        String regex = "^((https|http|ftp|rtsp|mms)?://)"  //https、http、ftp、rtsp、mms
                + "?(([0-9a-z_!~*'().&=+$%-]+: )?[0-9a-z_!~*'().&=+$%-]+@)?" //ftp的user@
                + "(([0-9]{1,3}\\.){3}[0-9]{1,3}" // IP形式的URL- 例如：199.194.52.184
                + "|" // 允许IP和DOMAIN（域名）
                + "([0-9a-z_!~*'()-]+\\.)*" // 域名- www.
                + "([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\\." // 二级域名
                + "[a-z]{2,6})" // first level domain- .com or .museum
                + "(:[0-9]{1,5})?" // 端口号最大为65535,5位数
                + "((/?)|" // a slash isn't required if there is no file name
                + "(/[0-9a-z_!~*'().;?:@&=+$,%#-]+)+/?)$";
        return str.matches(regex);
    }

    public static boolean isNullOrEmpty(String str) {
        if(str==null || str.isEmpty()) return true;
        return false;
    }
}
