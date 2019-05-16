package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class CommmonObjectWithMethNameEntity extends BaseEntity {

    private Object data;
    private String methodName;

    public CommmonObjectWithMethNameEntity(String code, Object data, String methodName) {
        this.setCode(code);
        this.data = data;
        this.methodName = methodName;
    }

    public Object getData() {
        return data;
    }

    public void setData(Object data) {
        this.data = data;
    }

    public String getMethodName() {
        return methodName;
    }

    public void setMethodName(String methodName) {
        this.methodName = methodName;
    }
}
