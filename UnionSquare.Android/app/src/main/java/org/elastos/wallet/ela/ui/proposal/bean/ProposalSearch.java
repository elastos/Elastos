package org.elastos.wallet.ela.ui.proposal.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ProposalSearch extends BaseEntity {
    /**
     * code : 1
     * data : {"list":[{"id":2,"title":"user02的建议","status":"ACTIVE","createdAt":1589271912,"proposedBy":"user02","proposalHash":"5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy"}],"total":0}
     */


    private DataBean data;

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean {
        /**
         * list : [{"id":2,"title":"user02的建议","status":"ACTIVE","createdAt":1589271912,"proposedBy":"user02","proposalHash":"5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy"}]
         * total : 0
         */

        private int total;
        private List<ListBean> list;

        public int getTotal() {
            return total;
        }

        public void setTotal(int total) {
            this.total = total;
        }

        public List<ListBean> getList() {
            return list;
        }

        public void setList(List<ListBean> list) {
            this.list = list;
        }

        public static class ListBean {
            /**
             * id : 2
             * title : user02的建议
             * status : ACTIVE
             * createdAt : 1589271912
             * proposedBy : user02
             * proposalHash : 5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy
             */

            private int id;
            private String title;
            private String status;
            private int createdAt;
            private String proposedBy;
            private String proposalHash;

            public int getId() {
                return id;
            }

            public void setId(int id) {
                this.id = id;
            }

            public String getTitle() {
                return title;
            }

            public void setTitle(String title) {
                this.title = title;
            }

            public String getStatus() {
                return status;
            }

            public void setStatus(String status) {
                this.status = status;
            }

            public int getCreatedAt() {
                return createdAt;
            }

            public void setCreatedAt(int createdAt) {
                this.createdAt = createdAt;
            }

            public String getProposedBy() {
                return proposedBy;
            }

            public void setProposedBy(String proposedBy) {
                this.proposedBy = proposedBy;
            }

            public String getProposalHash() {
                return proposalHash;
            }

            public void setProposalHash(String proposalHash) {
                this.proposalHash = proposalHash;
            }
        }
    }
}
