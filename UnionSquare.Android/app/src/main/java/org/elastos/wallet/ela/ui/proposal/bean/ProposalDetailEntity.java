package org.elastos.wallet.ela.ui.proposal.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ProposalDetailEntity extends BaseEntity {

    /**
     * code : 1
     * data : {"id":1,"abs":"this is sample abstract","address":"http://localhost:3001/proposals/5ea53e2bb0461a06630c0227","duration":0,"rejectAmount":"33.333","rejectThroughAmount":"1000","rejectRatio":0.033333,"voteResult":[{"value":"support","reason":"this is opinion","avatar":"http:/","votedBy":"Feng Zhang"}],"tracking":[{"comment":{"content":"tracking opinion","avatar":"http://test.com/assets/image.jpg","createdBy":"username"},"content":"tracking content","status":"PUBLISHED","createdAt":1589271912,"updatedAt":1589271912}],"summary":{"comment":{"content":"opinion content","avatar":"http://test.com/assets/image.jpg","createdBy":"username"},"content":"summary content","status":"PUBLISHED","createdAt":1589271912,"updatedAt":1589271912}}
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
         * abs : this is sample abstract
         * address : http://localhost:3001/proposals/5ea53e2bb0461a06630c0227
         * duration : 0
         * rejectAmount : 33.333
         * rejectThroughAmount : 1000
         * rejectRatio : 0.033333
         * voteResult : [{"value":"support","reason":"this is opinion","avatar":"http:/","votedBy":"Feng Zhang"}]
         * tracking : [{"comment":{"content":"tracking opinion","avatar":"http://test.com/assets/image.jpg","createdBy":"username"},"content":"tracking content","status":"PUBLISHED","createdAt":1589271912,"updatedAt":1589271912}]
         * summary : {"comment":{"content":"opinion content","avatar":"http://test.com/assets/image.jpg","createdBy":"username"},"content":"summary content","status":"PUBLISHED","createdAt":1589271912,"updatedAt":1589271912}
         */

        private int id;
        private String abs;
        private String address;
        private int duration;
        private String rejectAmount;
        private String rejectThroughAmount;
        private double rejectRatio;
        private SummaryBean summary;
        private List<VoteResultBean> voteResult;
        private List<TrackingBean> tracking;

        public int getId() {
            return id;
        }

        public void setId(int id) {
            this.id = id;
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

        public int getDuration() {
            return duration;
        }

        public void setDuration(int duration) {
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

        public double getRejectRatio() {
            return rejectRatio;
        }

        public void setRejectRatio(double rejectRatio) {
            this.rejectRatio = rejectRatio;
        }

        public SummaryBean getSummary() {
            return summary;
        }

        public void setSummary(SummaryBean summary) {
            this.summary = summary;
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

        public static class SummaryBean {
            /**
             * comment : {"content":"opinion content","avatar":"http://test.com/assets/image.jpg","createdBy":"username"}
             * content : summary content
             * status : PUBLISHED
             * createdAt : 1589271912
             * updatedAt : 1589271912
             */

            private CommentBean comment;
            private String content;
            private String status;
            private int createdAt;
            private int updatedAt;

            public CommentBean getComment() {
                return comment;
            }

            public void setComment(CommentBean comment) {
                this.comment = comment;
            }

            public String getContent() {
                return content;
            }

            public void setContent(String content) {
                this.content = content;
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

            public int getUpdatedAt() {
                return updatedAt;
            }

            public void setUpdatedAt(int updatedAt) {
                this.updatedAt = updatedAt;
            }

            public static class CommentBean {
                /**
                 * content : opinion content
                 * avatar : http://test.com/assets/image.jpg
                 * createdBy : username
                 */

                private String content;
                private String avatar;
                private String createdBy;

                public String getContent() {
                    return content;
                }

                public void setContent(String content) {
                    this.content = content;
                }

                public String getAvatar() {
                    return avatar;
                }

                public void setAvatar(String avatar) {
                    this.avatar = avatar;
                }

                public String getCreatedBy() {
                    return createdBy;
                }

                public void setCreatedBy(String createdBy) {
                    this.createdBy = createdBy;
                }
            }
        }

        public static class VoteResultBean {
            /**
             * value : support
             * reason : this is opinion
             * avatar : http:/
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
             * comment : {"content":"tracking opinion","avatar":"http://test.com/assets/image.jpg","createdBy":"username"}
             * content : tracking content
             * status : PUBLISHED
             * createdAt : 1589271912
             * updatedAt : 1589271912
             */

            private CommentBeanX comment;
            private String content;
            private String status;
            private int createdAt;
            private int updatedAt;

            public CommentBeanX getComment() {
                return comment;
            }

            public void setComment(CommentBeanX comment) {
                this.comment = comment;
            }

            public String getContent() {
                return content;
            }

            public void setContent(String content) {
                this.content = content;
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

            public int getUpdatedAt() {
                return updatedAt;
            }

            public void setUpdatedAt(int updatedAt) {
                this.updatedAt = updatedAt;
            }

            public static class CommentBeanX {
                /**
                 * content : tracking opinion
                 * avatar : http://test.com/assets/image.jpg
                 * createdBy : username
                 */

                private String content;
                private String avatar;
                private String createdBy;

                public String getContent() {
                    return content;
                }

                public void setContent(String content) {
                    this.content = content;
                }

                public String getAvatar() {
                    return avatar;
                }

                public void setAvatar(String avatar) {
                    this.avatar = avatar;
                }

                public String getCreatedBy() {
                    return createdBy;
                }

                public void setCreatedBy(String createdBy) {
                    this.createdBy = createdBy;
                }
            }
        }
    }
}
