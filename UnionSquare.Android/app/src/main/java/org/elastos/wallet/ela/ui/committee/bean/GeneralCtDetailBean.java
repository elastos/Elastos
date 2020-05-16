package org.elastos.wallet.ela.ui.committee.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class GeneralCtDetailBean extends BaseEntity {

    private List<DataBean> data;

    public List<DataBean> getData() {
        return data;
    }

    public void setData(List<DataBean> data) {
        this.data = data;
    }

    public static class DataBean {
        private String did;
        private String didName;
        private String avatar;
        private String address;
        private String introduction;
        private long impeachmentHeight;
        private long impeachmentVotes;
        private int location;
        private List<Term> term;

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
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

        public String getAddress() {
            return address;
        }

        public void setAddress(String address) {
            this.address = address;
        }

        public String getIntroduction() {
            return introduction;
        }

        public void setIntroduction(String introduction) {
            this.introduction = introduction;
        }

        public long getImpeachmentHeight() {
            return impeachmentHeight;
        }

        public void setImpeachmentHeight(long impeachmentHeight) {
            this.impeachmentHeight = impeachmentHeight;
        }

        public long getImpeachmentVotes() {
            return impeachmentVotes;
        }

        public void setImpeachmentVotes(long impeachmentVotes) {
            this.impeachmentVotes = impeachmentVotes;
        }

        public int getLocation() {
            return location;
        }

        public void setLocation(int location) {
            this.location = location;
        }

        public List<Term> getTerm() {
            return term;
        }

        public void setTerm(List<Term> term) {
            this.term = term;
        }
    }

    public static class Term {
        private String id;
        private String title;
        private String didName;
        private String status;
        private String voteResult;
        private long createdAt;

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public String getTitle() {
            return title;
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public String getVoteResult() {
            return voteResult;
        }

        public void setVoteResult(String voteResult) {
            this.voteResult = voteResult;
        }

        public long getCreatedAt() {
            return createdAt;
        }

        public void setCreatedAt(long createdAt) {
            this.createdAt = createdAt;
        }
    }
}
