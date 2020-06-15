/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.proposal.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ProposalDetailEntity extends BaseEntity {


    /**
     * code : 1
     * data : {"id":1,"status":"VOTING","abs":"this is sample abstract","address":"http://localhost:3001/proposals/5ea53e2bb0461a06630c0227","duration":0,"rejectAmount":"33.333","rejectThroughAmount":"1000","rejectRatio":0.033333,"voteResult":[{"value":"support","reason":"this is opinion","avatar":"","votedBy":"Feng Zhang"}],"tracking":[{"stage":2,"didName":"cr11-did","avatar":"https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=3695181958,4180835461&fm=26&gp=0.jpg","content":"test21","createdAt":1589380234,"comment":{"content":"test123","opinion":"REJECTED","avatar":"http://test.com/test.jpg","createdAt":1589380243}}],"message":"ok"}
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
         * id : 1
         * status : VOTING
         * abs : this is sample abstract
         * address : http://localhost:3001/proposals/5ea53e2bb0461a06630c0227
         * duration : 0
         * rejectAmount : 33.333
         * rejectThroughAmount : 1000
         * rejectRatio : 0.033333
         * voteResult : [{"value":"support","reason":"this is opinion","avatar":"","votedBy":"Feng Zhang"}]
         * tracking : [{"stage":2,"didName":"cr11-did","avatar":"https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=3695181958,4180835461&fm=26&gp=0.jpg","content":"test21","createdAt":1589380234,"comment":{"content":"test123","opinion":"REJECTED","avatar":"http://test.com/test.jpg","createdAt":1589380243}}]
         * message : ok
         */

        private int id;
        private String status;
        private String abs;
        private String address;
        private long duration;
        private String rejectAmount;
        private String rejectThroughAmount;
        private float rejectRatio;
        private List<VoteResultBean> voteResult;
        private List<TrackingBean> tracking;

        public int getId() {
            return id;
        }

        public void setId(int id) {
            this.id = id;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public String getAbs() {
            return abs;
        }

        public void setAbs(String abs) {
            this.abs = abs;
        }

        public String getAddress() {
            return address;
        }

        public void setAddress(String address) {
            this.address = address;
        }

        public long getDuration() {
            return duration;
        }

        public void setDuration(long duration) {
            this.duration = duration;
        }

        public String getRejectAmount() {
            return rejectAmount;
        }

        public void setRejectAmount(String rejectAmount) {
            this.rejectAmount = rejectAmount;
        }

        public String getRejectThroughAmount() {
            return rejectThroughAmount;
        }

        public void setRejectThroughAmount(String rejectThroughAmount) {
            this.rejectThroughAmount = rejectThroughAmount;
        }

        public float getRejectRatio() {
            return rejectRatio;
        }

        public void setRejectRatio(float rejectRatio) {
            this.rejectRatio = rejectRatio;
        }


        public List<VoteResultBean> getVoteResult() {
            return voteResult;
        }

        public void setVoteResult(List<VoteResultBean> voteResult) {
            this.voteResult = voteResult;
        }

        public List<TrackingBean> getTracking() {
            return tracking;
        }

        public void setTracking(List<TrackingBean> tracking) {
            this.tracking = tracking;
        }

        public static class VoteResultBean {
            /**
             * value : support
             * reason : this is opinion
             * avatar :
             * votedBy : Feng Zhang
             */

            private String value;
            private String reason;
            private String avatar;
            private String votedBy;

            public String getValue() {
                return value;
            }

            public void setValue(String value) {
                this.value = value;
            }

            public String getReason() {
                return reason;
            }

            public void setReason(String reason) {
                this.reason = reason;
            }

            public String getAvatar() {
                return avatar;
            }

            public void setAvatar(String avatar) {
                this.avatar = avatar;
            }

            public String getVotedBy() {
                return votedBy;
            }

            public void setVotedBy(String votedBy) {
                this.votedBy = votedBy;
            }
        }

        public static class TrackingBean {
            /**
             * stage : 2
             * didName : cr11-did
             * avatar : https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=3695181958,4180835461&fm=26&gp=0.jpg
             * content : test21
             * createdAt : 1589380234
             * comment : {"content":"test123","opinion":"REJECTED","avatar":"http://test.com/test.jpg","createdAt":1589380243}
             */

            private int stage;
            private String didName;
            private String avatar;
            private String content;
            private int createdAt;
            private CommentBean comment;

            public int getStage() {
                return stage;
            }

            public void setStage(int stage) {
                this.stage = stage;
            }

            public String getDidName() {
                return didName;
            }

            public void setDidName(String didName) {
                this.didName = didName;
            }

            public String getAvatar() {
                return avatar;
            }

            public void setAvatar(String avatar) {
                this.avatar = avatar;
            }

            public String getContent() {
                return content;
            }

            public void setContent(String content) {
                this.content = content;
            }

            public int getCreatedAt() {
                return createdAt;
            }

            public void setCreatedAt(int createdAt) {
                this.createdAt = createdAt;
            }

            public CommentBean getComment() {
                return comment;
            }

            public void setComment(CommentBean comment) {
                this.comment = comment;
            }

            public static class CommentBean {
                /**
                 * content : test123
                 * opinion : REJECTED
                 * avatar : http://test.com/test.jpg
                 * createdAt : 1589380243
                 */
                private String createdBy;
                private String content;
                private String opinion;
                private String avatar;
                private int createdAt;

                public String getCreatedBy() {
                    return createdBy;
                }

                public void setCreatedBy(String createdBy) {
                    this.createdBy = createdBy;
                }

                public String getContent() {
                    return content;
                }

                public void setContent(String content) {
                    this.content = content;
                }

                public String getOpinion() {
                    return opinion;
                }

                public void setOpinion(String opinion) {
                    this.opinion = opinion;
                }

                public String getAvatar() {
                    return avatar;
                }

                public void setAvatar(String avatar) {
                    this.avatar = avatar;
                }

                public int getCreatedAt() {
                    return createdAt;
                }

                public void setCreatedAt(int createdAt) {
                    this.createdAt = createdAt;
                }
            }
        }
    }
}
