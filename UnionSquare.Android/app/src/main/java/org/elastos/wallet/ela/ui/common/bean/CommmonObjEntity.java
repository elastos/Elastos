package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonObjEntity extends BaseEntity {


    public CommmonObjEntity(String code, Object data) {
        this.setCode(code);
        this.data = data;
    }

    public Object getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    private Object data;
}
