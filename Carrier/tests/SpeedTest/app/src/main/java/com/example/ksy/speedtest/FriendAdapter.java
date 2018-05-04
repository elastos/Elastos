package com.example.ksy.speedtest;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.ArrayList;


public class FriendAdapter extends RecyclerView.Adapter {
    private ArrayList<Friend> mData = null;

    FriendAdapter.OnItemClickListener mListener;

    FriendAdapter(ArrayList<Friend> data) {
        mData = data;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.friend_item, parent, false);
        ViewHolder viewHolder = new ViewHolder(v);
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        final ViewHolder friendHolder = (ViewHolder)holder;
        friendHolder.mId.setText(mData.get(position).mUid);
        friendHolder.mStatus.setText(mData.get(position).mConnected ? "online" : "offline");
        friendHolder.mSession.setText(mData.get(position).mSessionConnected ? "session" : "");

        friendHolder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View view) {
                if (mListener != null) {
                    int pos = friendHolder.getLayoutPosition();
                    mListener.onItemClick(view, mData.get(pos));
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        return mData == null ? 0 : mData.size();
    }

    public void addFriendList(ArrayList<Friend> list) {
        if (mData == null) {
            mData = list;
        } else if (list != null) {
            mData.addAll(list);
        }
        notifyDataSetChanged();
    }

    public void addFriend(Friend friend) {
        if (mData == null) {
            mData = new ArrayList<>();
        }
        mData.add(friend);
        notifyDataSetChanged();
    }

    public void deleteFriend(Friend friend) {
        if (mData == null) return;

        for (Friend item : mData) {
            if (item.mUid.equals(friend.mUid)) {
                mData.remove(friend);
                notifyDataSetChanged();//notifyItemRemoved();
                //notifyItemRemoved(1);
                break;
            }
        }
    }

    public void clearFriends() {
        if (mData == null) return;

        mData.clear();
    }

    public void friendStatusChanged(String uid, boolean online, boolean session) {
        if (mData == null) {
            mData = new ArrayList<>();
        }

        for (Friend item : mData) {
            if (item.mUid.equals(uid)) {
                item.mConnected = online;
                item.mSessionConnected = session;
                notifyDataSetChanged();
                return;
            }
        }

        Friend friend = new Friend(uid, online);
        friend.mSessionConnected = session;
        mData.add(friend);
        notifyDataSetChanged();
    }

    void setOnClickListener(FriendAdapter.OnItemClickListener listener) {
        mListener = listener;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        TextView mId;
        TextView mStatus;
        TextView mSession;

        public ViewHolder(View itemView) {
            super(itemView);
            mId = itemView.findViewById(R.id.text_friend_id);
            mStatus = itemView.findViewById(R.id.text_friend_status);
            mSession = itemView.findViewById(R.id.text_friend_session);
        }
    }


    public interface OnItemClickListener {
        void onItemClick(View view, Friend friend);
    }
}
