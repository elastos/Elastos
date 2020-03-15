package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class PersonalShowRecAdapetr extends RecyclerView.Adapter<PersonalShowRecAdapetr.ViewHolderParent> {


    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener1 commonRvListener;


    private Context context;

    private List<PersonalInfoItemEntity> list;

    public PersonalShowRecAdapetr(Context context, List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == 3)
            //两个textview
            return new ViewHolder3(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2textviewt, parent, false));
        else if (viewType == 2)
            //两个输入框
            return new ViewHolder2(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2input, parent, false));
        else if (viewType == 1)
            return new ViewHolder1(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_text, parent, false));
        else
            //一个输入框
            return new ViewHolder0(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_input, parent, false));

    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        int index = personalInfoItemEntity.getIndex();
        int type = holder.getItemViewType();
        if (holder instanceof ViewHolder1) {
            ((ViewHolder1) holder).tv.setText(personalInfoItemEntity.getText1());
            ((ViewHolder1) holder).tv.setHint(personalInfoItemEntity.getHintShow1());
            ((ViewHolder1) holder).tv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (commonRvListener != null)
                        commonRvListener.onRvItemClick(v, position, personalInfoItemEntity);
                }
            });

        } else if (holder instanceof ViewHolder2) {
            //手机号
            ((ViewHolder2) holder).et1.setText(personalInfoItemEntity.getText1());
            ((ViewHolder2) holder).et2.setText(personalInfoItemEntity.getText2());
            ((ViewHolder2) holder).et1.setHint(personalInfoItemEntity.getHintShow1());
            ((ViewHolder2) holder).et2.setHint(personalInfoItemEntity.getHintShow2());
        } else if (holder instanceof ViewHolder3) {
            //个人简介
            ((ViewHolder3) holder).tv1.setHint(personalInfoItemEntity.getHintShow1());
            ((ViewHolder3) holder).tv2.setText(personalInfoItemEntity.getText2());
            ((ViewHolder3) holder).itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (commonRvListener != null)
                        commonRvListener.onRvItemClick( ((ViewHolder3) holder).tv2, position, personalInfoItemEntity);
                }
            });
        } else {
            ((ViewHolder0) holder).et.setText(personalInfoItemEntity.getText1());
            ((ViewHolder0) holder).et.setHint(personalInfoItemEntity.getHintShow1());
        }
        if (commonRvListener != null) {
            holder.iv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(v, position, personalInfoItemEntity);
                }
            });
        }

    }

    @Override
    public int getItemViewType(int position) {
        int index = list.get(position).getIndex();
        if (index == 1 || index == 2 || index == 6) {
            return 1;
        }
        if (index == 5) {
            //手机号
            return 2;
        }
        if (index == 7) {
            //个人简介
            return 3;
        }
        return 0;
    }

    @Override
    public int getItemCount() {
        return list.size();
    }

    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        @BindView(R.id.iv)
        ImageView iv;

        ViewHolderParent(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder0 extends ViewHolderParent {
        @BindView(R.id.et)
        EditText et;

        ViewHolder0(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder1 extends ViewHolderParent {


        @BindView(R.id.tv)
        TextView tv;

        ViewHolder1(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder2 extends ViewHolderParent {
        @BindView(R.id.et1)
        EditText et1;
        @BindView(R.id.et2)
        EditText et2;

        ViewHolder2(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder3 extends ViewHolderParent {
        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.tv2)
        TextView tv2;

        ViewHolder3(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
