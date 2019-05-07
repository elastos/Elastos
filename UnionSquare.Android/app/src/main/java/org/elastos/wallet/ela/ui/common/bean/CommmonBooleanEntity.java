package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonBooleanEntity extends BaseEntity {


    public CommmonBooleanEntity(String code, boolean data) {
        this.setCode(code);
        this.data = data;
    }

    public boolean getData() {
        return data;
    }

    public void setData(boolean data) {
        this.data = data;
    }

    private boolean data;
}
