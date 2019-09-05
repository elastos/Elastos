package org.elastos.wallet.ela.ui.crvote.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.AppCompatCheckBox;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class NodeCartAdapter extends RecyclerView.Adapter<NodeCartAdapter.MyViewHolder> {
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> list;
    // 用来控制CheckBox的选中状况
    static HashMap<Integer, Boolean> dataMap;
    static HashMap<Integer, BigDecimal> dataMapELA;
    private Context context;


    public void setBalance(BigDecimal balance) {
        this.balance = balance;
    }

    private BigDecimal balance;

    // 构造器
    public NodeCartAdapter(List<VoteListBean.DataBean.ResultBean.ProducersBean> list, BaseFragment context) {
        this.context = context.getContext();
        this.list = list;
        dataMap = new HashMap<Integer, Boolean>();
        dataMapELA = new HashMap<Integer, BigDecimal>();
        // 初始化数据
    }


    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View view = LayoutInflater.from(context).inflate(R.layout.item_cr_nodcart, viewGroup, false);
        return new MyViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull MyViewHolder holder, int position) {
        if (dataMap.get(position) != null) {
            holder.checkbox.setChecked(dataMap.get(position));
        }
        if (dataMapELA.get(position) != null) {
            holder.etTicketnum.setText(dataMapELA.get(position).toEngineeringString());
        }
        VoteListBean.DataBean.ResultBean.ProducersBean producersBean = list.get(position);

        holder.tvName.setText(producersBean.getNickname());
        int id = producersBean.getIndex() + 1;
        holder.tvId.setText(context.getString(R.string.currentrank) + id);
        holder.tvZb.setText(context.getString(R.string.vote_of) + "：" + NumberiUtil.numberFormat(Double.parseDouble(producersBean.getVoterate()) * 100 + "", 5) + "%");
        holder.tvTicketnum.setText(context.getString(R.string.ticketnum) + producersBean.getVotes());
        holder.tvAddress.setText(AppUtlis.getLoc(context, producersBean.getLocation() + ""));
        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                holder.checkbox.toggle();
                dataMap.put(position, holder.checkbox.isChecked());
                if (onViewClickListener != null) {
                    onViewClickListener.onItemViewClick(NodeCartAdapter.this, v, position);
                }
            }
        });

    }


    @Override
    public int getItemCount() {
        return list.size();
    }


    class MyViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.checkbox)
        AppCompatCheckBox checkbox;
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_id)
        TextView tvId;
        @BindView(R.id.tv_zb)
        TextView tvZb;
        @BindView(R.id.tv_ticketnum)
        TextView tvTicketnum;
        @BindView(R.id.tv_address)
        TextView tvAddress;
        @BindView(R.id.et_ticketnum)
        EditText etTicketnum;


        MyViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);


        }
    }

    private OnViewClickListener onViewClickListener;

    public void setOnViewClickListener(OnViewClickListener onViewClickListener) {
        this.onViewClickListener = onViewClickListener;
    }

    public interface OnViewClickListener {
        void onItemViewClick(NodeCartAdapter adapter, View clickView, int position);
    }


    public HashMap<Integer, Boolean> getDataMap() {
        return dataMap;
    }

    public void setDataMap(HashMap<Integer, Boolean> dataMap) {
        NodeCartAdapter.dataMap = dataMap;
    }


    public BigDecimal getCountEla() {
        BigDecimal sum = new BigDecimal(0);
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (dataMapELA.get(i) != null && dataMap.get(i)) {
                    sum = sum.add(dataMapELA.get(i));
                }
            }
        }
        return sum;
    }

    public int getCheckNum() {
        int sum = 0;
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (dataMap.get(i) != null && dataMap.get(i)) {
                    sum++;
                }
            }
        }
        return sum;
    }

    // 初始化dataMap的数据
    public void initDateStaus(boolean status) {
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                dataMap.put(i, status);
            }
        }
    }

    // 初始化dataMap的数据
    public void equalDataMapELA() {
        BigDecimal curentbalance = Arith.div(balance, list.size(), 8);
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                dataMapELA.put(i, curentbalance);
            }
        }
    }
}
