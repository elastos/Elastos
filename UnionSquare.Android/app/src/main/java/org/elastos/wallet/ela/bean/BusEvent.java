package org.elastos.wallet.ela.bean;

/**
 * Created by wangdongfeng on 2018/5/3.
 */

public class BusEvent {
    private Object obj;
    private int code;
    private String name;
    private int pos;

    public int getPos() {
        return pos;
    }

    public void setPos(int pos) {
        this.pos = pos;
    }

      private BusEvent() {
    }
    public BusEvent(int code) {
        this.code = code;
    }
    public BusEvent(int code, String name) {
        this.code = code;
        this.name = name;
    }

    public BusEvent(int code, int pos) {
        this.code = code;
        this.pos = pos;
    }

    public BusEvent(int code, String name, Object obj) {
        this.code = code;
        this.name = name;
        this.obj = obj;
    }

    public int getCode() {
        return code;
    }

    public void setCode(int code) {
        this.code = code;
    }

    public Object getObj() {
        return obj;
    }

    public void setObj(Object obj) {
        this.obj = obj;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return "UpDateWalletNameResult{" +
                "obj=" + obj +
                ", code=" + code +
                ", name='" + name + '\'' +
                '}';
    }
}
