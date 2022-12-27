<main class="tab-pane active"
      ng-if="globalService.currentTab==globalService.tabs.sendTransaction.id"
      ng-controller='sendTxCtrl'
      ng-cloak >

  <!-- Header : todo turn into warning notification-->
  <div class="alert alert-info" ng-show="hasQueryString">
    <p translate="WARN_Send_Link">
      You arrived via a link that has the address, amount, gas or data fields filled in for you. You can change any information before sending. Unlock your wallet to get started.
    </p>
  </div>


  <!-- Unlock Wallet -->
  <article class="collapse-container">
    <span ng-click="wd = !wd">
      <a class="collapse-button"><span ng-show="wd">+</span><span ng-show="!wd">-</span></a>
      <h1 translate="NAV_SendEther">
        Send Ether &amp; Tokens
      </h1>
    </span>
    <div ng-show="!wd">
        @@if (site === 'cx' )  {  <cx-wallet-decrypt-drtv></cx-wallet-decrypt-drtv>   }
        @@if (site === 'web' ) {  <wallet-decrypt-drtv></wallet-decrypt-drtv>         }
    </div>
  </article>


  <!-- Send Tx Content -->
  <article class="row" ng-show="wallet!=null">
    @@if (site === 'web' ) { @@include( './sendTx-content.tpl', { "site": "web" } ) }
    @@if (site === 'cx'  ) { @@include( './sendTx-content.tpl', { "site": "cx"  } ) }

    @@if (site === 'web' ) { @@include( './sendTx-modal.tpl',   { "site": "web" } ) }
    @@if (site === 'cx'  ) { @@include( './sendTx-modal.tpl',   { "site": "cx"  } ) }
  </article>


</main>
