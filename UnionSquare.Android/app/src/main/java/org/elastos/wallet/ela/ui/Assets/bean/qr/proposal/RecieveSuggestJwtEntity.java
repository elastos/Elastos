package org.elastos.wallet.ela.ui.Assets.bean.qr.proposal;

import android.os.Parcel;

import java.util.ArrayList;
import java.util.List;

/**
 * 备用
 */
public class RecieveSuggestJwtEntity extends RecieveProposalFatherJwtEntity {

    /**
     * iat : 1566352213
     * exp : 1580607089
     * command : createsuggestion
     * data : {"proposaltype":"normal","categorydata":"","ownerpublickey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","drafthash":"6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464","budgets":[{"type":"imprest","stage":0,"amount":"100.1"},{"type":"normalpayment","stage":1,"amount":"200.2"},{"type":"finalpayment","stage":2,"amount":"300.2"}],"recipient":"EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k"}
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
         */

        private String proposaltype;
        private String categorydata;
        private String ownerpublickey;
        private String drafthash;
        private String recipient;
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

        public List<BudgetsBean> getBudgets() {
            return budgets;
        }

        public void setBudgets(List<BudgetsBean> budgets) {
            this.budgets = budgets;
        }

        public static class BudgetsBean implements android.os.Parcelable {
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

            @Override
            public int describeContents() {
                return 0;
            }

            @Override
            public void writeToParcel(Parcel dest, int flags) {
                dest.writeString(this.type);
                dest.writeInt(this.stage);
                dest.writeString(this.amount);
            }

            public BudgetsBean() {
            }

            protected BudgetsBean(Parcel in) {
                this.type = in.readString();
                this.stage = in.readInt();
                this.amount = in.readString();
            }

            public static final Creator<BudgetsBean> CREATOR = new Creator<BudgetsBean>() {
                @Override
                public BudgetsBean createFromParcel(Parcel source) {
                    return new BudgetsBean(source);
                }

                @Override
                public BudgetsBean[] newArray(int size) {
                    return new BudgetsBean[size];
                }
            };
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

    public RecieveSuggestJwtEntity() {
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

    protected RecieveSuggestJwtEntity(Parcel in) {
        super(in);
        this.data = in.readParcelable(DataBean.class.getClassLoader());
    }

    public static final Creator<RecieveSuggestJwtEntity> CREATOR = new Creator<RecieveSuggestJwtEntity>() {
        @Override
        public RecieveSuggestJwtEntity createFromParcel(Parcel source) {
            return new RecieveSuggestJwtEntity(source);
        }

        @Override
        public RecieveSuggestJwtEntity[] newArray(int size) {
            return new RecieveSuggestJwtEntity[size];
        }
    };
}
