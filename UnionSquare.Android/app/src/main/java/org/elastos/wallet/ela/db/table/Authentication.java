package org.elastos.wallet.ela.db.table;

import java.io.Serializable;

import io.realm.RealmObject;
import io.realm.annotations.PrimaryKey;

/**用户的认证状态
 * Created by wangdongfeng on 2018/4/15.
 */

public class Authentication extends RealmObject implements Serializable {

    @PrimaryKey
    private String memberId;//用戶id
    private boolean identity;//身份
    private boolean bank;//银行卡
    private boolean phone;//电话
    private boolean enterprise;//企业
    private String integral;//积分
    private String filed1;//
    private String filed2;//
    private String filed3;//

    public String getMemberId() {
        return memberId;
    }

    public void setMemberId(String memberId) {
        this.memberId = memberId;
    }

    public boolean getIdentity() {
        return identity;
    }

    public void setIdentity(boolean identity) {
        this.identity = identity;
    }

    public boolean getBank() {
        return bank;
    }

    public void setBank(boolean bank) {
        this.bank = bank;
    }

    public boolean getPhone() {
        return phone;
    }

    public void setPhone(boolean phone) {
        this.phone = phone;
    }

    public boolean getEnterprise() {
        return enterprise;
    }

    public void setEnterprise(boolean enterprise) {
        this.enterprise = enterprise;
    }

    public String getFiled1() {
        return filed1;
    }

    public void setFiled1(String filed1) {
        this.filed1 = filed1;
    }

    public String getFiled2() {
        return filed2;
    }

    public void setFiled2(String filed2) {
        this.filed2 = filed2;
    }

    public String getFiled3() {
        return filed3;
    }

    public void setFiled3(String filed3) {
        this.filed3 = filed3;
    }


    public void setAuthenticationData(Authentication data){
        this.memberId = data.getMemberId();//用戶id
        this.identity = data.getIdentity();//身份
        this.bank = data.getBank();//银行卡
        this.phone = data.getPhone();//电话
        this.enterprise = data.getEnterprise();//企业
        this.filed1 = data.getFiled1();//
        this.filed2 = data.getFiled2();//
        this.filed3 = data.getFiled3();//
    }

}
