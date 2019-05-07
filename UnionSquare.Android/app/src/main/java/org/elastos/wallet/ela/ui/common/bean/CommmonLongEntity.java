package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonLongEntity extends BaseEntity {


    public CommmonLongEntity(String code, long data) {
        this.setCode(code);
        this.data = data;
    }

    public long getData() {
        return data;
    }

    public void setData(long data) {
        this.data = data;
    }

    private long data;
}
