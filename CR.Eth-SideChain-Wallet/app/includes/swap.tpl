<!-- Swap Page -->
<main class="tab-pane swap-tab active" ng-if="globalService.currentTab==globalService.tabs.swap.id"
      ng-controller='swapCtrl' ng-cloak>

    @@if (site === 'web' ) { @@include( '../includes/swap-stage-1.tpl', { "site": "web" } ) }
    @@if (site === 'cx' ) { @@include( '../includes/swap-stage-1.tpl', { "site": "cx" } ) }

    @@if (site === 'web' ) { @@include( '../includes/swap-stage-2.tpl', { "site": "web" } ) }
    @@if (site === 'cx' ) { @@include( '../includes/swap-stage-2.tpl', { "site": "cx" } ) }

    @@if (site === 'web' ) { @@include( '../includes/swap-stage-3-shapeshift.tpl', { "site": "web" } ) }

    @@if (site === 'web' ) { @@include( '../includes/swap-stage-3.tpl', { "site": "web" } ) }
    @@if (site === 'cx' ) { @@include( '../includes/swap-stage-3.tpl', { "site": "cx" } ) }


    <section class="bity-contact text-center">
        <p><a class="btn-warning btn-sm"
              href="mailto:support@wallet.elaeth.io?Subject={{orderResult.orderId}}%20Issue%20regarding%20my%20Swap%20via%20MyCrypto%20&Body=%0APlease%20include%20the%20below%20if%20this%20issue%20is%20regarding%20your%20order.%20%0A%0AREF%20ID%23%3A%20{{orderResult.orderId}}%0A%0AAmount%20to%20send%3A%20{{orderResult.depositAmount}}%20{{orderResult.inputCurrency}}%0A%0AAmount%20to%20receive%3A%20{{orderResult.withdrawalAmount}}%20{{orderResult.pair.split('_')[1].toUpperCase()}}%0A%0APayment%20Address%3A%20{{orderResult.deposit}}%0A%0ARate%3A%20{{swapOrder.swapRate}}%20{{swapOrder.swapPair}}%0A%0A"
              target="_blank" rel="noopener noreferrer"> Issue with your Swap? Contact support</a></p>
        <p ng-click="swapIssue = !swapIssue">
            <small>Click here if link doesn't work</small>
        </p>
        <textarea class="form-control input-sm" rows="9" ng-show="swapIssue" style="max-width: 35rem;margin: auto;">
To: support@wallet.elaeth.io
Subject: {{orderResult.orderId}} - Issue regarding my Swap via MyCrypto
Message:
REF ID#: {{orderResult.orderId}}
Amount to send: {{orderResult.depositAmount}} {{orderResult.inputCurrency}}
Amount to receive: {{orderResult.withdrawalAmount}} {{orderResult.pair.split('_')[1].toUpperCase()}}
Payment Address: {{orderResult.deposit}}
Receiving Address: {{orderResult.withdrawal}}
Rate: {{swapOrder.swapRate}} {{swapOrder.swapPair}}</textarea>
    </section>


</main>
<!-- / Swap Page -->




