package org.elastos.wallet.ela.ui.vote.bean;

import java.io.Serializable;

public class Area implements Comparable<Area> ,Serializable{

    /**
     * en : Angola
     * zh : 安哥拉
     * locale : AO
     * code : 244
     */

    private String en;
    private String zh;
    private String locale;
    private long code;

    public String getEn() {
        return en;
    }

    public void setEn(String en) {
        this.en = en;
    }

    public String getZh() {
        return zh;
    }

    public void setZh(String zh) {
        this.zh = zh;
    }

    public String getLocale() {
        return locale;
    }

    public void setLocale(String locale) {
        this.locale = locale;
    }

    public long getCode() {
        return code;
    }

    public void setCode(long code) {
        this.code = code;
    }

    @Override
    public int compareTo(Area o) {
        return this.en.compareTo(o.en);
    }
}