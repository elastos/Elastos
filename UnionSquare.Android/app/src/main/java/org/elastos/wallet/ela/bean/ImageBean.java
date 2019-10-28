package org.elastos.wallet.ela.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class ImageBean extends BaseEntity {

    /**
     * message : Query successful ^_^
     * data : images/df1f4de34ff312cef7e0da1317e3d082.svg
     * exceptionMsg : null
     */

    private String message;
    private String data;
    private Object exceptionMsg;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public Object getExceptionMsg() {
        return exceptionMsg;
    }

    public void setExceptionMsg(Object exceptionMsg) {
        this.exceptionMsg = exceptionMsg;
    }
}
