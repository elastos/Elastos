package org.elastos.wallet.ela.ui.main.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ServerListEntity extends BaseEntity {
    /**
     * message : Getting Producer Nodes List Successed !
     * data : ["https://unionsquare.elastos.org","http://54.223.158.189"]
     * exceptionMsg : null
     */

    private String message;
    private Object exceptionMsg;
    private List<String> data;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Object getExceptionMsg() {
        return exceptionMsg;
    }

    public void setExceptionMsg(Object exceptionMsg) {
        this.exceptionMsg = exceptionMsg;
    }

    public List<String> getData() {
        return data;
    }

    public void setData(List<String> data) {
        this.data = data;
    }

    @Override
    public String toString() {
        return "ServerListEntity{" +
                "message='" + message + '\'' +
                ", exceptionMsg=" + exceptionMsg +
                ", data=" + data +
                '}';
    }
}
