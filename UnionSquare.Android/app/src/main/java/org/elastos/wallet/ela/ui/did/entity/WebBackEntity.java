package org.elastos.wallet.ela.ui.did.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class WebBackEntity extends BaseEntity {


    /**
     * code : 200
     * success : true
     * message : Ok
     */


    private boolean success;
    private String message;


    public boolean isSuccess() {
        return success;
    }

    public void setSuccess(boolean success) {
        this.success = success;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    @Override
    public String toString() {
        return "WebBackEntity{" +
                "code=" + getCode() +
                ", message='" + message + '\'' +
                '}';
    }
}
