package org.elastos.wallet.ela.ui.committee.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.io.Serializable;

public class PastCtBean extends BaseEntity {

    private String index;
    private String time;

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

    public static class DataBean {

    }


}
