package org.elastos.wallet.ela.ui.committee.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.io.Serializable;

public class PastCtBean extends BaseEntity {

    private String index;
    private String time;
    private int type; // 0,普通 1,选举管理 2，委员管理 3，选举管理


    public String getIndex() {
        return index;
    }

    public void setIndex(String index) {
        this.index = index;
    }

    public String getTime() {
        return time;
    }

    public void setTime(String time) {
        this.time = time;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public static class DataBean {

    }


}
