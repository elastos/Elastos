package org.elastos.wallet.ela.ui.Assets.bean.qr.proposal;

import org.elastos.wallet.ela.ui.Assets.bean.qr.RecieveJwtEntity;

/**
 * 所有proposal相关的网站接收的二维码父亲
 */
public class RecieveProposalFatherJwtEntity extends RecieveJwtEntity {

    /**
     * iat : 1566352213
     * exp : 1580607089
     * command : createproposal
     * data : {"proposaltype":"normal","categorydata":"","ownerpublickey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","drafthash":"6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464","budgets":[{"type":"imprest","stage":0,"amount":"100.1"},{"type":"normalpayment","stage":1,"amount":"200.2"},{"type":"finalpayment","stage":2,"amount":"300.2"}],"recipient":"EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k","signature":"0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148","did":"did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw"}
     */

    private String command;
    private String sid;

    public String getSid() {
        return sid;
    }

    public void setSid(String sid) {
        this.sid = sid;
    }

    public String getCommand() {
        return command;
    }

    public void setCommand(String command) {
        this.command = command;
    }


}
