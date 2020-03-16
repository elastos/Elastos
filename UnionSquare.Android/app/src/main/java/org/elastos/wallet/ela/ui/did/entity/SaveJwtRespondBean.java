package org.elastos.wallet.ela.ui.did.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class SaveJwtRespondBean extends BaseEntity {

    /**
     * message : jwt Info added successful ^_^
     * data : null
     * exceptionMsg : null
     */

    private String message;
    private Object data;
    private Object exceptionMsg;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Object getData() {
        return data;
    }

    public void setData(Object data) {
        this.data = data;
    }

    public Object getExceptionMsg() {
        return exceptionMsg;
    }

    public void setExceptionMsg(Object exceptionMsg) {
        this.exceptionMsg = exceptionMsg;
    }
}
