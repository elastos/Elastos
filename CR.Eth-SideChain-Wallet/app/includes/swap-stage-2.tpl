<!-- Swap Start 2 -->
<article class="swap-start" ng-if="showStage2">


    <!-- Title -->
    <section class="row">
        <h5 class="col-xs-6 col-xs-offset-3" translate="SWAP_information">Your Information</h5>
        <div class="col-xs-3" ng-if="isBitySwap">
        </div>
        <div class="col-xs-3" ng-if="!isBitySwap">
            <a class="link bity-logo" href="https://shapeshift.io" target="_blank" rel="noopener">
                <img class="pull-right" src="images/shapeshift-dark.svg" width="100" height="38"/>
            </a>
        </div>
    </section>
    <!-- Title -->


    <!-- Info Row -->
    <section class="order-info-wrap row">
        <div class="col-sm-4 order-info">
            <h4> {{swapOrder.fromVal}} {{swapOrder.fromCoin}} </h4>
            <p translate="SWAP_send_amt"> Amount to send </p>
        </div>
        <div class="col-sm-4 order-info">
            <h4> {{swapOrder.toVal}} {{swapOrder.toCoin}} </h4>
            <p translate="SWAP_rec_amt"> Amount to receive </p>
        </div>
        <div class="col-sm-4 order-info">
            <h4> {{swapOrder.swapRate}} {{swapOrder.swapPair}} </h4>
            <p translate="SWAP_your_rate"> Your rate </p>
        </div>
    </section>
    <!-- / Info Row -->


    <!-- Your Address -->
    <section class='swap-address block'>
        <section class="row">
            <div class="col-sm-8 col-sm-offset-2 col-xs-12">
                <label><span translate="SWAP_rec_add">Your Receiving Address</span> <strong>({{swapOrder.toCoin}})</strong></label>
                <div class="form-group" ng-show="swapOrder.toCoin!='BTC'">
                    <address-field placeholder="0x4bbeEB066eD09B7AEd07bF39EEe0460DFa261520" var-name="swapOrder.toAddress"></address-field>
                </div>
                <input class="form-control"
                       ng-show="swapOrder.toCoin=='BTC'"
                       type="text"
                       placeholder="32oirLEzZRhi33RCXDF9WHJjEb8RsrSss3"
                       ng-model="swapOrder.toAddress"
                       ng-class="Validator.isValidBTCAddress(swapOrder.toAddress) ? 'is-valid' : 'is-invalid'"/>
            </div>
        </section>
        <!-- /Your Address -->
        <!-- CTA -->
        <section class="row text-center">
            <button ng-click="openOrder()" class="btn btn-primary btn-lg" ng-disabled="orderOpenLoading">
                <div ng-if="orderOpenLoading">
                    Loading...
                </div>
                <span ng-if="!orderOpenLoading" translate="SWAP_start_CTA"> Start Swap </span>
            </button>
        </section>
        <!-- / CTA -->
    </section>


</article>
<!-- / Swap Start 2 -->
