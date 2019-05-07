package org.elastos.wallet.ela.ui.Assets.bean;

import java.util.List;

public class Payload {

    private List<String> CrossChainAddress;
    private List<String> CrossChainAmount;
    private List<String> OutputIndex;

    public List<String> getCrossChainAddress() {
        return CrossChainAddress;
    }

    public void setCrossChainAddress(List<String> CrossChainAddress) {
        this.CrossChainAddress = CrossChainAddress;
    }

    public List<String> getCrossChainAmount() {
        return CrossChainAmount;
    }

    public void setCrossChainAmount(List<String> CrossChainAmount) {
        this.CrossChainAmount = CrossChainAmount;
    }

    public List<String> getOutputIndex() {
        return OutputIndex;
    }

    public void setOutputIndex(List<String> OutputIndex) {
        this.OutputIndex = OutputIndex;
    }
}
