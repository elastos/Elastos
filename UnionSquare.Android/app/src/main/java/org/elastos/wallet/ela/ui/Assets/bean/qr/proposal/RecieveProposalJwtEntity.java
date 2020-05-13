package org.elastos.wallet.ela.ui.Assets.bean.qr.proposal;

import android.os.Parcel;

import java.util.ArrayList;
import java.util.List;

/**
 * 兼顾提建议和提案
 */
public class RecieveProposalJwtEntity extends RecieveProposalFatherJwtEntity {

    /**
     * iat : 1566352213
     * exp : 1580607089
     * command : createproposal
     * data : {"proposaltype":"normal","categorydata":"","ownerpublickey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","drafthash":"6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464","budgets":[{"type":"imprest","stage":0,"amount":"100.1"},{"type":"normalpayment","stage":1,"amount":"200.2"},{"type":"finalpayment","stage":2,"amount":"300.2"}],"recipient":"EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k","signature":"0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148","did":"did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw"}
     */


    private DataBean data;


    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean implements android.os.Parcelable {
        /**
         * proposaltype : normal
         * categorydata :
         * ownerpublickey : 023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a
         * drafthash : 6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464
         * budgets : [{"type":"imprest","stage":0,"amount":"100.1"},{"type":"normalpayment","stage":1,"amount":"200.2"},{"type":"finalpayment","stage":2,"amount":"300.2"}]
         * recipient : EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k
         * signature : 0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148
         * did : did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw
         */

        private String proposaltype;
        private String categorydata;
        private String ownerpublickey;
        private String drafthash;
        private String recipient;
        private String signature;
        private String did;
        private List<BudgetsBean> budgets;

        public String getProposaltype() {
            return proposaltype;
        }

        public void setProposaltype(String proposaltype) {
            this.proposaltype = proposaltype;
        }

        public String getCategorydata() {
            return categorydata;
        }

        public void setCategorydata(String categorydata) {
            this.categorydata = categorydata;
        }

        public String getOwnerpublickey() {
            return ownerpublickey;
        }

        public void setOwnerpublickey(String ownerpublickey) {
            this.ownerpublickey = ownerpublickey;
        }

        public String getDrafthash() {
            return drafthash;
        }

        public void setDrafthash(String drafthash) {
            this.drafthash = drafthash;
        }

        public String getRecipient() {
            return recipient;
        }

        public void setRecipient(String recipient) {
            this.recipient = recipient;
        }

        public String getSignature() {
            return signature;
        }

        public void setSignature(String signature) {
            this.signature = signature;
        }

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public List<BudgetsBean> getBudgets() {
            return budgets;
        }

        public void setBudgets(List<BudgetsBean> budgets) {
            this.budgets = budgets;
        }

        public static class BudgetsBean {
            /**
             * type : imprest
             * stage : 0
             * amount : 100.1
             */

            private String type;
            private int stage;
            private String amount;

            public String getType() {
                return type;
            }

            public void setType(String type) {
                this.type = type;
            }

            public int getStage() {
                return stage;
            }

            public void setStage(int stage) {
                this.stage = stage;
            }

            public String getAmount() {
                return amount;
            }

            public void setAmount(String amount) {
                this.amount = amount;
            }
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.proposaltype);
            dest.writeString(this.categorydata);
            dest.writeString(this.ownerpublickey);
            dest.writeString(this.drafthash);
            dest.writeString(this.recipient);
            dest.writeString(this.signature);
            dest.writeString(this.did);
            dest.writeList(this.budgets);
        }

        public DataBean() {
        }

        protected DataBean(Parcel in) {
            this.proposaltype = in.readString();
            this.categorydata = in.readString();
            this.ownerpublickey = in.readString();
            this.drafthash = in.readString();
            this.recipient = in.readString();
            this.signature = in.readString();
            this.did = in.readString();
            this.budgets = new ArrayList<BudgetsBean>();
            in.readList(this.budgets, BudgetsBean.class.getClassLoader());
        }

        public static final Creator<DataBean> CREATOR = new Creator<DataBean>() {
            @Override
            public DataBean createFromParcel(Parcel source) {
                return new DataBean(source);
            }

            @Override
            public DataBean[] newArray(int size) {
                return new DataBean[size];
            }
        };
    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeParcelable(this.data, flags);
    }

    public RecieveProposalJwtEntity() {
    }

    protected RecieveProposalJwtEntity(Parcel in) {
        super(in);
        this.data = in.readParcelable(DataBean.class.getClassLoader());
    }

    public static final Creator<RecieveProposalJwtEntity> CREATOR = new Creator<RecieveProposalJwtEntity>() {
        @Override
        public RecieveProposalJwtEntity createFromParcel(Parcel source) {
            return new RecieveProposalJwtEntity(source);
        }

        @Override
        public RecieveProposalJwtEntity[] newArray(int size) {
            return new RecieveProposalJwtEntity[size];
        }
    };
}
