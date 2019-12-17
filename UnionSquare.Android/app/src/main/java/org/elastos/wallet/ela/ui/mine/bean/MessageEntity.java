package org.elastos.wallet.ela.ui.mine.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.Serializable;
import java.util.Objects;

/**
 * 用两个list存储entity
 * read  一旦打开就清空unread并加进来
 * unread  新收到的消息放这里来
 * (后期优化可以在内存里收集  在onSaveInstanceState和ondestory时候保存上次保存的_内存里的  在打开消息记录时候清空 )
 */
public class MessageEntity implements Parcelable, Serializable {
    private long time;
    private String walletName;
    private String hash;
    private String reason;
    private String transferType;
    private String chainId;

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public String getWalletName() {
        return walletName;
    }

    public void setWalletName(String walletName) {
        this.walletName = walletName;
    }

    public String getHash() {
        return hash;
    }

    public void setHash(String hash) {
        this.hash = hash;
    }

    public String getReason() {
        return reason;
    }

    public void setReason(String reason) {
        this.reason = reason;
    }

    public String getTransferType() {
        return transferType;
    }

    public void setTransferType(String transferType) {
        this.transferType = transferType;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(this.time);
        dest.writeString(this.walletName);
        dest.writeString(this.hash);
        dest.writeString(this.reason);
        dest.writeString(this.transferType);
        dest.writeString(this.chainId);
    }

    public MessageEntity() {
    }

    protected MessageEntity(Parcel in) {
        this.time = in.readLong();
        this.walletName = in.readString();
        this.hash = in.readString();
        this.reason = in.readString();
        this.transferType = in.readString();
        this.chainId = in.readString();
    }

    public static final Parcelable.Creator<MessageEntity> CREATOR = new Parcelable.Creator<MessageEntity>() {
        @Override
        public MessageEntity createFromParcel(Parcel source) {
            return new MessageEntity(source);
        }

        @Override
        public MessageEntity[] newArray(int size) {
            return new MessageEntity[size];
        }
    };

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        MessageEntity that = (MessageEntity) o;
        return Objects.equals(hash, that.hash);
    }

    @Override
    public int hashCode() {
        return Objects.hash(hash);
    }
}
