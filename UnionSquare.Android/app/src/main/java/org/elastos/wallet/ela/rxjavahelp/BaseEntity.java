package org.elastos.wallet.ela.rxjavahelp;

public abstract   class BaseEntity {
    private String code;
    private String msg;

    public String getCode() {
        return code;
    }

    public void setCode(String code) {
        this.code = code;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }

    @Override
    public String toString() {
        return "BaseEntity{" +
                "code=" + code +
                ", msg='" + msg + '\'' +
                '}';
    }
}
