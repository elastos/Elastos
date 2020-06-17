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

package org.elastos.wallet.ela.utils;


public enum RxEnum {
    //
    ONE,//创建钱包
    TWO,//刷新钱包余额
    DELETE,//删除钱包
    FOUR,
    UPDATAPROPERTY,//增加或者删除资产
    ADDPROPERTY,//增加资产
    DELETEPROPERTY,//删除资产
    UPDATA_WALLET_NAME,//钱包名称
    SIX,//性别
    IDENTITY_AUTHENTICATION,//认证身份
    PHONE_AUTHENTICATION,//认证电话
    CARD_AUTHENTICATION,//认证银行卡
    UPDATA_NICKNAME,//昵称
    TOKEN_REFRESH,//刷新token
    UPDATA_FILED1,
    UPDATA_PHONE,//手机号
    TRANSFER,//交易成功
    AVASTER,//头像
    UPDATE_UNIT,//法币单位
    SHARE,
    INVITE,//邀请好友
    UPTADE,//更新
    BACKUP,//备份助记词
    TIME,//开始时间
    STATUS,//状态栏
    TRADESUCCESSFULLY,//交易成功
    TXSUCCESSFULLY,//提现成功
    MQTT,
    WALLETUPDATE,//更新切换当前的默认钱包
    SELECTWALLET,//选择钱包
    LOCKTRANSFER,//lock交易成功
    SUPMTRADESUCCESSFULLY,//交易成功
    AREA,//
    CHOODECOIN,//财富选择币种
    SETPAYPWD,//设置财富密码
    UPDATAPAYPWD,//设置财富密码
    FORGETPAYPWD,//修改财富密码
    MINESUCESS,//挖矿成功
    MINEERRO,//挖矿数量不足
    BINDMONEY,//某一收付款绑定或解绑成功
    UPDATACONTACT,//
    CHOOSECONTACT,//选择联系人
    TRANSFERSUCESS,//发起转账成功
    JUSTSHOWFEE,//只展示手续费
    CHOSESIDECHAIN,//选择测链地址
    UPDATAPROGRESS,//首页同步
    VOTETRANSFERACTIVITY,//投票
    BALANCECHANGE,//balance发生变化
    CHANGELANGUAGE,//改变语言
    SIGNSUCCESS,//签名成功

    //opentype
    CREATEDEFAULT,//创建单签钱包时候的
    MANAGER,//钱包管理
    PRIVATEKEY,//创建多签钱包时候添加私钥使用
    //添加私钥
    CREATEPRIVATEKEY,//新建私钥成功
    IMPORTRIVATEKEY,//导入助记词成功
    SELECTRIVATEKEY,//导入助记词成功
    TOSIGN,//生成待签名交易

    //did
    KEEPDRAFT,//保存草稿
    RETURCER,//返回cerdential数据
    EDITPERSONALINFO,//编辑个人信息
    EDITPERSONALINTRO,//编辑个人简介
    EDITSOCIAL,//编辑社交账号
    IPVALID,//节点ip可用
    AGREE,//cr注册同意协议
    NOTICE,//消息通知
    READNOTICE,//看完消息通知
    GETDEPOSITVOTR,//列表其他获得非active数据
    REFRESHMESSAGE,//刷新消息中心的数据
    VERTIFYPAYPASS,//回传密码
    TRANSFERSUCESSPWD,//交易成功并回传密码
    SAVECREDENCIALTOWEB,//上传凭证到服务器
    SCANDATATOASSETPAGE,//把数据给首页处理
    TRANSACTIONSUCCESSMESSAGE,//交易成功的消息 //只为通知特定页面加的补丁

   /* enum Type {
        CoinBase, // 创币交易
        RegisterAsset, // 注册资产交易
        TransferAsset, // 普通转账交易
        Record, // 记录交易
        Deploy, // 部署交易
        SideChainPow, // 侧链挖矿交易
        RechargeToSideChain, // 侧链充值交易
        WithdrawFromSideChain, // 主链提现交易
        TransferCrossChainAsset, // 跨链交易

        RegisterProducer, // 注册参选交易
        CancelProducer,// 取消参选交易
        UpdateProducer, // 更新参选信息交易
        ReturnDepositCoin, // 取回参选质押资产交易

        IllegalProposalEvidence, // 不用管
        IllegalVoteEvidence, // 不用管
        IllegalBlockEvidence, // 不用管

        RegisterIdentification, // 注册ID交易 待定 will refactor later
        //  TypeMaxCount
    }*/


}
