package org.elastos.wallet.ela.ui.did.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class WebBackEntity extends BaseEntity {
    /**
     * code : 200
     * success : true
     * message : Ok
     */


    private boolean success;


    public boolean isSuccess() {
        return success;
    }

    public void setSuccess(boolean success) {
        this.success = success;
    }


}
