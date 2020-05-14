package org.elastos.wallet.ela.ui.committee.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class ExperienceBean extends BaseEntity {

    private String title;
    private String subTitle;
    private int type; //0 弃权， 1 反对， 2 赞成


    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getSubTitle() {
        return subTitle;
    }

    public void setSubTitle(String subTitle) {
        this.subTitle = subTitle;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }
}
