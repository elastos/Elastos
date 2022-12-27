'use strict';

var footerCtrl = function($scope, globalService) {
    var gasPriceKey = "gasPrice";
    var packageJson = require('../../../package.json');
    $scope.version = packageJson.version;
    $scope.footerModal = new Modal(document.getElementById('disclaimerModal'));
    $scope.ethBlockNumber = "loading";
    $scope.etcBlockNumber = "loading";
    $scope.showBlocks = window.location.protocol == "https:";
    $scope.setBlockNumbers = function() {
        if (!$scope.showBlocks) return;
        ajaxReq.getCurrentBlock(function(data) { $scope.currentBlockNumber = data.data; });
    }
    $scope.setBlockNumbers();
    $scope.globalService = globalService;

    $scope.curLang = globalFuncs.curLang;
    $scope.gasChanged = function() {
        globalFuncs.localStorage.setItem(gasPriceKey, $scope.gas.value);
        ethFuncs.gasAdjustment = $scope.gas.value;
    }
    var setGasValues = function() {
        $scope.gas = {
            curVal: 10,
            value: globalFuncs.localStorage.getItem(gasPriceKey, null) ? parseInt(globalFuncs.localStorage.getItem(gasPriceKey)) : 10,
            max: 99,
            min: 0.1,
            step: 0.1
        }
        ethFuncs.gasAdjustment = $scope.gas.value;
    }
    setGasValues();
    $scope.gasChanged();
};
module.exports = footerCtrl;
