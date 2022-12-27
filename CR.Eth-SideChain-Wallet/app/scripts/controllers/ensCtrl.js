'use strict';
var ensCtrl = function($scope, $sce, walletService) {
    $scope.ajaxReq = ajaxReq;
    $scope.hideEnsInfoPanel = false;
    walletService.wallet = null;
    $scope.ensConfirmModalModal = new Modal(document.getElementById('ensConfirmModal'));
    $scope.ensFinalizeModal = new Modal(document.getElementById('ensFinalizeConfirm'));
    $scope.Validator = Validator;
    $scope.wd = false;
    $scope.haveNotAlreadyCheckedLength = true;
    var ENS = new ens();
    var DomainSale = new domainsale();
    $scope.ensModes = ens.modes;
    $scope.minNameLength = 7;
    $scope.objDomainSale = {};
    $scope.objENS = {
        bidValue: 0.01,
        dValue: 0.01,
        name: '',
        namehash: '',
        nameSHA3: '',
        nameReadOnly: false,
        resolvedAddress: null,
        revealObject: null,
        secret: hd.bip39.generateMnemonic().split(" ").splice(0, 3).join(" "),
        status: -1,
        timer: null,
        timeRemaining: null,
        timeRemainingReveal: null,
        txSent: false
    };
    $scope.gasLimitDefaults = {
        startAuction: '200000',
        newBid: '500000',
        reveal: '200000',
        finalize: '200000'
    }
    $scope.tx = {
        gasLimit: '500000',
        data: '',
        to: '',
        unit: "ether",
        value: 0,
        gasPrice: null
    };
    $scope.showENS = function() {
        return nodes.ensNodeTypes.indexOf(ajaxReq.type) > -1;
    }
    $scope.$watch(function() {
        if (walletService.wallet == null) return null;
        return walletService.wallet.getAddressString();
    }, function() {
        if (walletService.wallet == null) return;
        $scope.wallet = walletService.wallet;
        $scope.wd = true;
        $scope.objENS.nameReadOnly = true;
        $scope.wallet.setBalance();
        $scope.wallet.setTokens();
    });
    $scope.getCurrentTime = function() {
        return new Date().toString();
    }
    var updateScope = function() {
        if (!$scope.$$phase) $scope.$apply();
    }
    var timeRem = function(timeUntil) {
        var rem = timeUntil - new Date();
        if (rem < 0) {
            clearInterval($scope.objENS.timer);
            $scope.objENS.timeRemaining = "EXPIRED";
            return
        }
        var _second = 1000;
        var _minute = _second * 60;
        var _hour = _minute * 60;
        var _day = _hour * 24;
        var days = Math.floor(rem / _day);
        var hours = Math.floor((rem % _day) / _hour);
        var minutes = Math.floor((rem % _hour) / _minute);
        var seconds = Math.floor((rem % _minute) / _second);
        days = days < 10 ? '0' + days : days;
        hours = hours < 10 ? '0' + hours : hours;
        minutes = minutes < 10 ? '0' + minutes : minutes;
        seconds = seconds < 10 ? '0' + seconds : seconds;
        $scope.objENS.timeRemaining = days + ' days ' + hours + ' hours ' + minutes + ' minutes ' + seconds + ' seconds ';
        $scope.objENS.timeRemainingReveal = (days - 2) + ' days ' + hours + ' hours ' + minutes + ' minutes ' + seconds + ' seconds ';
        updateScope();
    }
    $scope.nameOnChange = function() {
        $scope.objENS.status = -1;
        $scope.objENS.timeRemaining = null;
        clearInterval($scope.objENS.timer);
    }
    $scope.checkName = function() {
        // checks if it's the same length as a PK and if so, warns them.
        // If they confirm they can set haveNotAlreadyCheckedLength to true and carry on
        if ( $scope.haveNotAlreadyCheckedLength && ($scope.objENS.name.length == 128 || $scope.objENS.name.length == 132 || $scope.objENS.name.length == 64 || $scope.objENS.name.length == 66) ) {
          $scope.notifier.danger( "That looks an awful lot like a private key. Are you sure you would like to check if this name is available on the ENS network? If so, click `Check`. If it is your private key, click refresh & try again." );
          $scope.haveNotAlreadyCheckedLength = false;
        } else if ($scope.Validator.isValidENSName($scope.objENS.name) && $scope.objENS.name.indexOf('.') == -1) {
            $scope.objENS.name = ens.normalise($scope.objENS.name);
            $scope.objENS.namehash = ens.getNameHash($scope.objENS.name + '.eth');
            $scope.objENS.nameSHA3 = ENS.getSHA3($scope.objENS.name);
            $scope.hideEnsInfoPanel = true;
            ENS.getAuctionEntries($scope.objENS.name, function(data) {
                if (data.error) $scope.notifier.danger(data.msg);
                else {
                    var entries = data.data;
                    for (var key in entries) $scope.objENS[key] = entries[key];
                    switch ($scope.objENS.status) {
                        case $scope.ensModes.owned:
                            ENS.getOwner($scope.objENS.name + '.eth', function(data) {
                                $scope.objENS.owner = data.data;
                            })
                            ENS.getDeedOwner($scope.objENS.deed, function(data) {
                                $scope.objENS.deedOwner = data.data;
                            })
                            ENS.getAddress($scope.objENS.name + '.eth', function(data) {
                                $scope.objENS.resolvedAddress = data.data;
                            })
                            DomainSale.getSale($scope.objENS.name, function(data) {
                                $scope.objDomainSale.sale = data.data;
                            })
                            break;
                        case $scope.ensModes.notAvailable:
                            ENS.getAllowedTime($scope.objENS.name, function(data) {
                                $scope.objENS.allowedTime = data.data;
                                clearInterval($scope.objENS.timer);
                                $scope.objENS.timer = setInterval(() => timeRem($scope.objENS.allowedTime), 1000);
                            })
                            break;
                        case $scope.ensModes.auction:
                            clearInterval($scope.objENS.timer);
                            $scope.objENS.timer = setInterval(() => timeRem($scope.objENS.registrationDate), 1000);
                            break;
                        case $scope.ensModes.reveal:
                            $scope.objENS.bidValue = 0;
                            $scope.objENS.secret = '';
                            $scope.objENS.highestBid = etherUnits.toEther($scope.objENS.highestBid.toString(), 'wei');
                            clearInterval($scope.objENS.timer);
                            $scope.objENS.timer = setInterval(() => timeRem($scope.objENS.registrationDate), 1000);
                            break;
                    }
                    updateScope();
                }
            })
        } else $scope.notifier.danger(globalFuncs.errorMsgs[30]);
    }

    $scope.onLongStringChanged = function() {
        try {
            $scope.objENS.revealObject = null;
            var tObj = JSON.parse($scope.longJsonString.replace(/\\/g, ''));
            $scope.objENS.revealObject = tObj;
            if (tObj.value) $scope.objENS.bidValue = Number(etherUnits.toEther(tObj.value, "wei"));
            if (tObj.secret) $scope.objENS.secret = tObj.secret;
            if (tObj.name && ens.normalise(tObj.name) != $scope.objENS.name) { // check if correct name
                $scope.notifier.danger(globalFuncs.errorMsgs[34]);
            } else if (tObj.owner && tObj.owner != $scope.wallet.getAddressString()) { // check owner = bidder
                $scope.notifier.danger(globalFuncs.errorMsgs[33]);
            } else { //estimate gas to see if it would not work
                //$scope.estimateGasLimit();
            }
            updateScope();
        } catch (e) {
            $scope.notifier.danger(e.message);
        }
    }
    var getShaBid = function(_bidObject, callback) {
        ENS.shaBid(_bidObject.nameSHA3, _bidObject.owner, _bidObject.value, _bidObject.secretSHA3, function(data) {
            if (data.error) callback(true, data.msg);
            else callback(false, data.data);
        });
    }
    var getBidObject = function() {
        var _objENS = $scope.objENS;
        var bidObject = {
            name: _objENS.name,
            nameSHA3: ENS.getSHA3(_objENS.name),
            owner: $scope.wallet.getAddressString(),
            value: etherUnits.toWei(_objENS.bidValue, 'ether'),
            secret: _objENS.secret.trim(),
            secretSHA3: ENS.getSHA3(_objENS.secret.trim())
        }
        return bidObject;
    }
    $scope.openAndBidAuction = function() {
        $scope.tx.gasLimit = $scope.gasLimitDefaults.newBid;
        var _objENS = $scope.objENS;
        $scope.bidObject = getBidObject();
        _objENS.registrationDate = new Date();
        _objENS.registrationDate.setDate(_objENS.registrationDate.getDate() + 5);
        getShaBid($scope.bidObject, function(isError, data) {
            if (isError) $scope.notifier.danger(data);
            else {
                var bidHash = data;
                $scope.tx.data = ENS.getStartAndBidAuctionData($scope.objENS.name, bidHash);
                $scope.tx.to = ENS.getAuctionAddress();
                $scope.tx.value = _objENS.dValue;
                var txData = uiFuncs.getTxData($scope);
                txData.nonce = txData.gasPrice = null;
                uiFuncs.generateTx(txData, function(rawTx) {
                    if (!rawTx.isError) {
                        $scope.generatedTxs.push(rawTx.signedTx);
                        $scope.bidObject = JSON.stringify($scope.bidObject)
                        $scope.ensConfirmModalModal.open();
                    } else {
                        $scope.notifier.danger(rawTx.error);
                    }
                    if (!$scope.$$phase) $scope.$apply();
                });
            }
        });
    }
    $scope.revealBid = function() {
        $scope.tx.gasLimit = $scope.gasLimitDefaults.reveal;
        var _objENS = $scope.objENS;
        ajaxReq.getTransactionData($scope.wallet.getAddressString(), function(data) {
            if (data.error) $scope.notifier.danger(data.msg);
            data = data.data;
            $scope.tx.to = ENS.getAuctionAddress();
            $scope.tx.data = ENS.getRevealBidData(_objENS.name, etherUnits.toWei(_objENS.bidValue, 'ether'), _objENS.secret);
            $scope.tx.value = 0;
            var txData = uiFuncs.getTxData($scope);
            txData.gasPrice = data.gasprice;
            txData.nonce = data.nonce;
            uiFuncs.generateTx(txData, function(rawTx) {
                if (!rawTx.isError) {
                    $scope.generatedTxs.push(rawTx.signedTx);
                    $scope.ensConfirmModalModal.open();
                } else {
                    $scope.notifier.danger(rawTx.error);
                }
            });
        });
    }
    $scope.finalizeDomain = function() {
        $scope.tx.gasLimit = $scope.gasLimitDefaults.finalize;
        if ($scope.wallet.getAddressString() != $scope.objENS.deedOwner) {
            $scope.notifier.danger(globalFuncs.errorMsgs[33]);
            return;
        }
        var _objENS = $scope.objENS;
        ajaxReq.getTransactionData($scope.wallet.getAddressString(), function(data) {
            if (data.error) $scope.notifier.danger(data.msg);
            data = data.data;
            $scope.tx.to = ENS.getAuctionAddress();
            $scope.tx.data = ENS.getFinalizeAuctionData(_objENS.name);
            $scope.tx.value = 0;
            var txData = uiFuncs.getTxData($scope);
            txData.gasPrice = data.gasprice;
            txData.nonce = data.nonce;
            uiFuncs.generateTx(txData, function(rawTx) {
                if (!rawTx.isError) {
                    $scope.generatedTxs = [];
                    $scope.generatedTxs.push(rawTx.signedTx);
                    $scope.ensFinalizeModal.open();
                } else {
                    $scope.notifier.danger(rawTx.error);
                }
            });
        });
    }
    $scope.getRevealTime = function() {
        if ($scope.objENS && $scope.objENS.registrationDate) return new Date($scope.objENS.registrationDate - (48 * 60 * 60 * 1000));
        return new Date().toString();
    }
    $scope.bidAuction = function(nonce, gasPrice) {
        $scope.tx.gasLimit = $scope.gasLimitDefaults.newBid;
        var _objENS = $scope.objENS;
        $scope.bidObject = getBidObject();
        getShaBid($scope.bidObject, function(isError, data) {
            if (isError) $scope.notifier.danger(data);
            else {
                var bidHash = data;
                $scope.tx.data = ENS.getNewBidData(bidHash);
                $scope.tx.to = ENS.getAuctionAddress();
                $scope.tx.value = _objENS.dValue;
                var txData = uiFuncs.getTxData($scope);
                if (nonce && gasPrice) {
                    txData.nonce = nonce;
                    txData.gasPrice = gasPrice;
                } else txData.nonce = txData.gasPrice = null;
                uiFuncs.generateTx(txData, function(rawTx) {
                    if (!rawTx.isError) {
                        $scope.generatedTxs.push(rawTx.signedTx);
                        $scope.bidObject = JSON.stringify($scope.bidObject)
                        $scope.ensConfirmModalModal.open();
                    } else {
                        $scope.notifier.danger(rawTx.error);
                    }
                    if (!$scope.$$phase) $scope.$apply();
                });
            }
        });
    }
    $scope.sendTxStatus = "";
    $scope.sendTx = function() {
        $scope.ensConfirmModalModal.close();
        $scope.ensFinalizeModal.close();
        var signedTx = $scope.generatedTxs.shift();
        uiFuncs.sendTx(signedTx, function(resp) {
            if (!resp.isError) {
                var emailLink = '<a class="strong" href="mailto:support@wallet.elaeth.io?Subject=Issue%20regarding%20my%20ENS%20&Body=Hi%20Taylor%2C%20%0A%0AI%20have%20a%20question%20concerning%20my%20ENS%20transaction.%20%0A%0AI%20was%20attempting%20to%3A%0A-%20Start%20an%20ENS%20auction%0A-%20Bid%20on%20an%20ENS%20name%0A-%20Reveal%20my%20ENS%20bid%0A-%20Finalize%20my%20ENS%20name%0A%0AUnfortunately%20it%3A%0A-%20Never%20showed%20on%20the%20blockchain%0A-%20Failed%20due%20to%20out%20of%20gas%0A-%20Failed%20for%20another%20reason%0A-%20Never%20showed%20up%20in%20the%20account%20I%20was%20sending%20to%0A%0APlease%20see%20the%20below%20details%20for%20additional%20information.%0A%0AThank%20you.%20%0A%0A_%0A%0A%20name%3A%20' + $scope.objENS.name + '%0A%20timeRemaining%3A%20' + $scope.getRevealTime().toString() + '%0A%20revealDate%3A%20' + $scope.objENS.registrationDate.toString() + "%0A%20timer%3A%20" + $scope.objENS.timer + "%0A%20txSent%3A%20" + $scope.objENS.txSent + "%0A%20to%3A%20" + $scope.tx.to + "%0A%20from%20address%3A%20" + $scope.wallet.getAddressString() + "%0A%20data%3A%20" + $scope.tx.data + "%0A%20value%3A%20" + $scope.tx.value + '" target="_blank" rel="noopener noreferrer">Confused? Email Us.</a>';
                var bExStr = $scope.ajaxReq.type != nodes.nodeTypes.Custom ? "<a class='strong' href='" + $scope.ajaxReq.blockExplorerTX.replace("[[txHash]]", resp.data) + "' target='_blank' rel='noopener'> View your transaction </a>" : '';
                $scope.sendTxStatus += globalFuncs.successMsgs[2] + "<p>" + resp.data + "</p><p>" + bExStr + "</p><p>" + emailLink + "</p>";
                $scope.notifier.success($scope.sendTxStatus);
                if ($scope.generatedTxs.length) $scope.sendTx();
                else $scope.sendTxStatus = ''
            } else {
                $scope.notifier.danger(resp.error);
            }
        });
        $scope.objENS.txSent = true;
        $scope.objENS.hideEnsInfoPanel = false;
    }
    $scope.generateTx = function() {
        try {
            var _objENS = $scope.objENS;
            $scope.sentTxs = [];
            $scope.generatedTxs = [];
            if (!$scope.Validator.isValidENSName(_objENS.name)) throw globalFuncs.errorMsgs[30];
            else if (!$scope.Validator.isPositiveNumber(_objENS.bidValue) || _objENS.bidValue < 0.01) throw globalFuncs.errorMsgs[0];
            else if (_objENS.status != $scope.ensModes.reveal && (!$scope.Validator.isPositiveNumber(_objENS.dValue) || _objENS.dValue < _objENS.bidValue || $scope.wallet.balance <= _objENS.dValue)) throw globalFuncs.errorMsgs[0];
            else if (!$scope.Validator.isPasswordLenValid(_objENS.secret, 0)) throw globalFuncs.errorMsgs[31];
            else if (_objENS.revealObject && _objENS.revealObject.name && ens.normalise(_objENS.revealObject.name) != _objENS.name) throw globalFuncs.errorMsgs[34];
            else {
                if ($scope.objENS.status == $scope.ensModes.open) $scope.openAndBidAuction();
                else if ($scope.objENS.status == $scope.ensModes.auction) $scope.bidAuction();
                else if ($scope.objENS.status == $scope.ensModes.reveal) $scope.revealBid();
            }
        } catch (e) {
            $scope.notifier.danger(e);
        }
    }
}
module.exports = ensCtrl;
