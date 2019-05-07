package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class CommonStringArrayEntity extends BaseEntity {

    private String[] data;


    public CommonStringArrayEntity(String code, String[] data) {
        this.setCode(code);
        this.data = data;
    }

    public String[] getData() {
        return data;
    }

    public void setData(String[] data) {
        this.data = data;
    }
}
