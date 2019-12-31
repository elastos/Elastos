package org.elastos.wallet.ela.ui.Assets.bean;

public class SubWalletBasicInfo {


    /**
     * ChainID : ELA
     * Info : {"Account":{"HasPassPhrase":true,"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"},"CoinIndex":0}
     */

    private String ChainID;
    private InfoBean Info;

    public String getChainID() {
        return ChainID;
    }

    public void setChainID(String ChainID) {
        this.ChainID = ChainID;
    }

    public InfoBean getInfo() {
        return Info;
    }

    public void setInfo(InfoBean Info) {
        this.Info = Info;
    }

    public static class InfoBean {
        /**
         * Account : {"HasPassPhrase":true,"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"}
         * CoinIndex : 0
         */

        private AccountBean Account;
        private int CoinIndex;

        public AccountBean getAccount() {
            return Account;
        }

        public void setAccount(AccountBean Account) {
            this.Account = Account;
        }

        public int getCoinIndex() {
            return CoinIndex;
        }

        public void setCoinIndex(int CoinIndex) {
            this.CoinIndex = CoinIndex;
        }

        public static class AccountBean {
            /**
             * HasPassPhrase : true
             * M : 1
             * N : 1
             * Readonly : false
             * SingleAddress : false
             * Type : Standard
             */

            private boolean HasPassPhrase;
            private int M;
            private int N;
            private boolean Readonly;
            private boolean SingleAddress;
            private String Type;

            public boolean isHasPassPhrase() {
                return HasPassPhrase;
            }

            public void setHasPassPhrase(boolean HasPassPhrase) {
                this.HasPassPhrase = HasPassPhrase;
            }

            public int getM() {
                return M;
            }

            public void setM(int M) {
                this.M = M;
            }

            public int getN() {
                return N;
            }

            public void setN(int N) {
                this.N = N;
            }

            public boolean isReadonly() {
                return Readonly;
            }

            public void setReadonly(boolean Readonly) {
                this.Readonly = Readonly;
            }

            public boolean isSingleAddress() {
                return SingleAddress;
            }

            public void setSingleAddress(boolean SingleAddress) {
                this.SingleAddress = SingleAddress;
            }

            public String getType() {
                return Type;
            }

            public void setType(String Type) {
                this.Type = Type;
            }
        }
    }
}
