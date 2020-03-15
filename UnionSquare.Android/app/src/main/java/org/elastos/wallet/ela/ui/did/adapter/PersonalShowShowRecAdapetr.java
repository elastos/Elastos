package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.graphics.drawable.PictureDrawable;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.svg.SvgSoftwareLayerSetter;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class PersonalShowShowRecAdapetr extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private Context context;

    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener1 commonRvListener;
    private List<PersonalInfoItemEntity> list;

    public PersonalShowShowRecAdapetr(Context context, List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == 1)
            //展示照片
            return new ViewHolder1(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_tviv, parent, false));
        else
            return new ViewHolder0(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2textviewt_show, parent, false));

    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        //  holder.setIsRecyclable(false);
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        int index = personalInfoItemEntity.getIndex();
        if (holder instanceof ViewHolder1) {
            //手机号
            ((ViewHolder1) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            String logo = personalInfoItemEntity.getText1();
            if (logo.endsWith(".svg")) {
                GlideApp.with(context).as(PictureDrawable.class).listener(new SvgSoftwareLayerSetter()).load(logo).into(((ViewHolder1) holder).iv1);
            } else {
                GlideApp.with(context).load(logo).into(((ViewHolder1) holder).iv1);
            }

        } else {
            ((ViewHolder0) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            ((ViewHolder0) holder).tv2.setText(personalInfoItemEntity.getText1());
            if (index == 5) {
                //电话
                String result = personalInfoItemEntity.getText1() + personalInfoItemEntity.getText2();
                if (personalInfoItemEntity.getText1() == null) {
                    result = personalInfoItemEntity.getText2();
                } else if (personalInfoItemEntity.getText2() == null) {
                    result = personalInfoItemEntity.getText1();
                }
                ((ViewHolder0) holder).tv2.setText(result);
            }
            if (index == 7) {
                ((ViewHolder0) holder).itemView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (commonRvListener != null)
                            commonRvListener.onRvItemClick(((ViewHolder0) holder).tv2, position, personalInfoItemEntity);
                    }
                });
            }
        }


    }

    @Override
    public int getItemViewType(int position) {
        int index = list.get(position).getIndex();
        if (index == 3) {
            //头像
            return 1;
        }
        return 0;
    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder0 extends RecyclerView.ViewHolder {


        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.tv2)
        TextView tv2;

        ViewHolder0(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder1 extends RecyclerView.ViewHolder {

        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.iv1)
        ImageView iv1;

        ViewHolder1(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
