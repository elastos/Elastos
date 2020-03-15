package org.elastos.wallet.ela.ui.did.entity;

public class PersonalInfoItemEntity implements Comparable<PersonalInfoItemEntity> {
    private int index;
    private String hintShow1;//初始化后不会再修改
    private String hintShow2;//初始化后不会再修改
    private String hintChose;//初始化后不会再修改
    private String text1;
    private String text2;

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public String getHintShow1() {
        return hintShow1;
    }

    public void setHintShow1(String hintShow1) {
        this.hintShow1 = hintShow1;
    }

    public String getHintShow2() {
        return hintShow2;
    }

    public void setHintShow2(String hintShow2) {
        this.hintShow2 = hintShow2;
    }

    public String getHintChose() {
        return hintChose;
    }

    public void setHintChose(String hintChose) {
        this.hintChose = hintChose;
    }

    public String getText1() {
        return text1;
    }

    public void setText1(String text1) {
        this.text1 = text1;
    }

    public String getText2() {
        return text2;
    }

    public void setText2(String text2) {
        this.text2 = text2;
    }

    @Override
    public int compareTo(PersonalInfoItemEntity o) {
        return this.index - o.index;
    }
}
