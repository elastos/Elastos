package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonStringWithiMethNameEntity extends BaseEntity {

    private String data;
    private String methodName;

    public CommmonStringWithiMethNameEntity(String code, String data, String methodName) {
        this.setCode(code);
        this.data = data;
        this.methodName = methodName;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public String getMethodName() {
        return methodName;
    }

    public void setMethodName(String methodName) {
        this.methodName = methodName;
    }
}
