package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class CommmonStringListEntity extends BaseEntity {

    private List<String> data;


    public CommmonStringListEntity(String code, List<String> data) {
       this.setCode(code);
        this.data = data;
    }

    public List<String> getData() {
        return data;
    }

    public void setData(List<String> data) {
        this.data = data;
    }
}
