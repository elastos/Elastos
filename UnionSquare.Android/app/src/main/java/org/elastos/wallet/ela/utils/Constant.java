package org.elastos.wallet.ela.utils;


import org.elastos.wallet.ela.MyApplication;

public class Constant {

    public static final String REQUEST_BASE_URL = MyApplication.chainID == 0 ? "https://unionsquare.elastos.org/" : "https://52.81.8.194:442/";


    public static final String CONTACTSHOW = "contact_show";
    public static final String CONTACTADD = "contact_add";
    public static final String CONTACTEDIT = "contact_edit";
    public static final String INNER = "inner";//油钱包列表打开
    public static final String SIDEWITHDRAW = "sideWithdraw";//侧链充值
    public static final String TRANFER = "transfer";//普通充值

}
