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

package org.elastos.wallet.ela.ui.crvote.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.AppCompatCheckBox;
import android.support.v7.widget.RecyclerView;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

public class CRNodeCartAdapter extends RecyclerView.Adapter<CRNodeCartAdapter.MyViewHolder> {
    public List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> getList() {
        return list;
    }

    public void setList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {
        this.list = list;
    }

    private List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list;
    private Context context;


    public void setBalance(BigDecimal balance) {
        this.balance = balance;
    }

    private BigDecimal balance;

    // 构造器
    public CRNodeCartAdapter(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list, BaseFragment context) {
        this.context = context.getContext();
        this.list = list;
        // dataMap = new HashMap<Integer, Boolean>();
        // dataMapELA = new HashMap<Integer, BigDecimal>();
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
        CRListBean.DataBean.ResultBean.CrcandidatesinfoBean producersBean = list.get(position);

        holder.checkbox.setChecked(producersBean.isChecked());


        holder.tvName.setText(producersBean.getNickname());
        int id = producersBean.getIndex() + 1;
        holder.tvId.setText(context.getString(R.string.currentrank) + id);
        holder.tvZb.setText(context.getString(R.string.vote_of) + "：" + producersBean.getVoterate() + "%");
        holder.tvTicketnum.setText(context.getString(R.string.ticketnum) + new BigDecimal(producersBean.getVotes()).intValue());
        holder.tvAddress.setText(AppUtlis.getLoc(context, producersBean.getLocation() + ""));
        holder.etTicketnum.setTag(false);
        if (holder.etTicketnum.getTag(R.id.et_ticketnum) != null) {
            holder.etTicketnum.removeTextChangedListener((TextWatcher) holder.etTicketnum.getTag(R.id.et_ticketnum));
        }
        holder.etTicketnum.setText(null);
        if (producersBean.getCurentBalance() != null) {
            holder.etTicketnum.setText(NumberiUtil.numberFormat(producersBean.getCurentBalance(), 8));
        }
        addTextChangedListener(holder.etTicketnum, position);
        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                holder.checkbox.toggle();
                producersBean.setChecked(holder.checkbox.isChecked());
                if (onViewClickListener != null) {
                    onViewClickListener.onItemViewClick(CRNodeCartAdapter.this, v, position);
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
        void onItemViewClick(CRNodeCartAdapter adapter, View clickView, int position);
    }

    private OnTextChangedListener onTextChangedListener;

    public void setOnTextChangedListener(OnTextChangedListener onTextChangedListener) {
        this.onTextChangedListener = onTextChangedListener;
    }

    public interface OnTextChangedListener {
        void onTextChanged(CRNodeCartAdapter adapter, View clickView, int position);
    }

    public BigDecimal getCountEla() {
        BigDecimal sum = new BigDecimal(0);
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (list.get(i).isChecked() && list.get(i).getCurentBalance() != null && list.get(i).getCurentBalance().compareTo(new BigDecimal(0)) > 0) {
                    sum = sum.add(list.get(i).getCurentBalance());
                }
            }
        }
        return sum;
    }

    public int getCheckNum() {
        int sum = 0;
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (list.get(i).isChecked()) {
                    sum++;
                }
            }
        }
        return sum;
    }

    public int getCheckAndHasBalanceNum() {
        int sum = 0;
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (list.get(i).isChecked() && list.get(i).getCurentBalance() != null && list.get(i).getCurentBalance().compareTo(new BigDecimal(0)) > 0) {
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
                list.get(i).setChecked(status);
            }
        }
    }

    // 初始化dataMap的数据
    public void initAllCurentBalance() {
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                list.get(i).setCurentBalance(null);
            }
        }
    }

    public void setDateStaus(int size, boolean status) {
        if (list != null) {
            for (int i = 0; i < size; i++) {
                list.get(i).setChecked(status);
            }
        }
    }

    // 初始化前 number个dataMap的数据
    public void equalDataMapELA(int number) {
        BigDecimal curentbalance = Arith.div(balance, number, 8);
        if (list != null) {
            for (int i = 0; i < number; i++) {
                list.get(i).setCurentBalance(curentbalance);
            }
        }
    }

    // 初始化dataMap的数据
    public Map<String, String> getCheckedData() {
        Map<String, String> checkedData = new HashMap();
        if (list != null) {
            for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : list) {
                if (bean.isChecked() && bean.getCurentBalance() != null && bean.getCurentBalance().compareTo(new BigDecimal(0)) > 0
                        && !TextUtils.isEmpty(bean.getCurentBalance().toPlainString())) {
                    checkedData.put("\"" + bean.getDid() + "\"", "\"" + bean.getCurentBalance().multiply(new BigDecimal(MyWallet.RATE)).setScale(0, BigDecimal.ROUND_DOWN).toPlainString() + "\"");
                }

            }
        }
        return checkedData;
    }


    private void addTextChangedListener(EditText editText, int position) {

        TextWatcher textWatcher = new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (editText.getTag() != null && (boolean) editText.getTag()) {
                    editText.setTag(false);
                    return;
                }
                if (TextUtils.isEmpty(s)) {
                    list.get(position).setCurentBalance(null);
                    if (onTextChangedListener != null) {
                        onTextChangedListener.onTextChanged(CRNodeCartAdapter.this, editText, position);
                    }
                    return;
                }
                String number = s.toString().trim();
                if (number.startsWith(".")) {
                    editText.setTag(true);
                    editText.setText(null);
                    return;
                }
                if (new BigDecimal(number).compareTo(balance) > 0) {
                    number = balance.toPlainString();
                    editText.setTag(true);
                    editText.setText(number);
                    editText.setSelection(number.length());
                } else if (number.split("\\.").length > 1 && number.split("\\.")[1].length() > 8) {
                    number = (number.split("\\."))[0] + "." + number.split("\\.")[1].substring(0, 8);
                    editText.setTag(true);
                    editText.setText(number);
                    editText.setSelection(number.length());
                }

                list.get(position).setCurentBalance(new BigDecimal(number));
                if (onTextChangedListener != null) {
                    onTextChangedListener.onTextChanged(CRNodeCartAdapter.this, editText, position);
                }
            }
        };
        editText.addTextChangedListener(textWatcher);
        editText.setTag(R.id.et_ticketnum, textWatcher);
    }
}
