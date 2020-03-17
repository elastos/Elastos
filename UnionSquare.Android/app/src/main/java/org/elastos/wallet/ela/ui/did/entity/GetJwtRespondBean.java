package org.elastos.wallet.ela.ui.did.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class GetJwtRespondBean extends BaseEntity {

    /**
     * message : Query successful ^_^
     * data : {"did":"did:elastos:iWsK3X8aBpLqFVXAd2KUpMpFGJYEYyfjUi","jwt":"jwtsave"}
     * exceptionMsg : null
     */

    private String message;
    private DataBean data;
    private Object exceptionMsg;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public Object getExceptionMsg() {
        return exceptionMsg;
    }

    public void setExceptionMsg(Object exceptionMsg) {
        this.exceptionMsg = exceptionMsg;
    }

    public static class DataBean {
        /**
         * did : did:elastos:iWsK3X8aBpLqFVXAd2KUpMpFGJYEYyfjUi
         * jwt : jwtsave
         */

        private String did;
        private String jwt;

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public String getJwt() {
            return jwt;
        }

        public void setJwt(String jwt) {
            this.jwt = jwt;
        }
    }
}
