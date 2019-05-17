package org.elastos.wallet.ela.utils;

/**
 * Created by wangdongfeng on 2018/4/22.
 */

public enum RxEnum {
    //
    ONE,//创建钱包
    TWO,//刷新钱包余额
    DELETE,//删除钱包
    FOUR,
    UPDATAPROPERTY,//增加或者删除资产
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
    CHOSESIDECHAIN,//选择测链地址
    UPDATAPROGRESS,//首页同步
    VOTETRANSFERACTIVITY,//投票
    BALANCECHANGE,//balance发生变化
    CHANGELANGUAGE;//改变语言

    enum Type {
        CoinBase, // 创币交易
        RegisterAsset, // 注册资产交易
        TransferAsset, // 普通转账交易
        Record, // 记录交易
        Deploy, // 部署交易
        SideChainPow, // 侧链挖矿交易
        RechargeToSideChain, // 侧链充值交易
        WithdrawFromSideChain, // 侧链提现交易
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
    }


}
