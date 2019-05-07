package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonStringEntity extends BaseEntity {


    public CommmonStringEntity(String code, String data) {
        this.setCode(code);
        this.data = data;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    private String data;
}
