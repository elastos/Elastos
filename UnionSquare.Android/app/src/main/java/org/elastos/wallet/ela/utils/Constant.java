package org.elastos.wallet.ela.utils;


import org.elastos.wallet.ela.MyApplication;

public class Constant {
    public static final String CONTACTSHOW = "contact_show";
    public static final String CONTACTADD = "contact_add";
    public static final String CONTACTEDIT = "contact_edit";
    public static final String INNER = "inner";//油钱包列表打开
    public static final String SIDEWITHDRAW = "sideWithdraw";//侧链充值
    public static final String TRANFER = "transfer";//普通充值
    public static final String SUPERNODESIGN = "supernodevote";//超级节点参选
    public static final String CRSIGNUP = "crsignup";//cr注册
    public static final String CRVOTE = "crvote";//cr投票
    public static final String UPDATENODEINFO= "updatenodeinfo";//超级节点更新参选信息
    public static final String CRUPDATE= "crupdate";//cr更新参选信息
    public static final String Email = "wallet@elastos.org";
    public static final String UpdateLog = "https://download.elastos.org/app/release-notes/ela-wallet/index.html";
    public static final String FRAGMENTTAG = "commonwebview";
    public static final int CREATEREADONLY = 0x1;//打开创建单签只读钱包页面
    public static final int CREATEMUL = 0x2;//打开创建多签钱包页面 已经打开了的情况直接填入
    public static final int SIGN = 0x3;//打开签名页面
    public static final int TRANSFER = 0x4;//打开转账页面
    public static final String SERVERLIST_BASE = "http://54.223.158.189:5739/";
    public static final String SERVERLIST = "api/dposNodeRPC/getProducerNodesList";

}
