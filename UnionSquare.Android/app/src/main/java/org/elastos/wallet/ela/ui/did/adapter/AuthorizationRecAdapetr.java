package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class AuthorizationRecAdapetr extends RecyclerView.Adapter<AuthorizationRecAdapetr.ViewHolderParent> {


    private Context context;

    private List<PersonalInfoItemEntity> list;

    public AuthorizationRecAdapetr(Context context, List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {
        return new ViewHolderParent(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_authorization, parent, false));

    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        holder.tv.setText(personalInfoItemEntity.getHintShow1());
        holder.cb.setChecked(personalInfoItemEntity.isCheck());
        holder.cb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                personalInfoItemEntity.setCheck(isChecked);
            }
        });


    }


    @Override
    public int getItemCount() {
        return list.size();
    }

    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        @BindView(R.id.tv)
        TextView tv;
        @BindView(R.id.cb)
        CheckBox cb;

        ViewHolderParent(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
