package org.elastos.wallet.ela.ui.Assets.bean;

import java.math.BigDecimal;

public class VoteStatus {
    private int index;//唯一表示
    private int iconID;
    private String name;
    private int status;//0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
    private BigDecimal count;

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public int getIconID() {
        return iconID;
    }

    public void setIconID(int iconID) {
        this.iconID = iconID;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public BigDecimal getCount() {
        return count;
    }

    public void setCount(BigDecimal count) {
        this.count = count;
    }
}