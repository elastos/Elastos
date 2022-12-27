<main class="tab-pane active"
      ng-if="globalService.currentTab==globalService.tabs.offlineTransaction.id"
      ng-controller='offlineTxCtrl'
      ng-cloak>

  <h1 translate="OFFLINE_Title">
    Generate &amp; Send Offline Transaction
  </h1>

  @@if (site === 'web' ) { @@include( './offlineTx-1.tpl',     { "site": "web" } ) }
  @@if (site === 'cx'  ) { @@include( './offlineTx-1.tpl',     { "site": "cx"  } ) }

  @@if (site === 'web' ) { @@include( './offlineTx-2.tpl',     { "site": "web" } ) }
  @@if (site === 'cx'  ) { @@include( './offlineTx-2.tpl',     { "site": "cx"  } ) }

  @@if (site === 'web' ) { @@include( './offlineTx-3.tpl',     { "site": "web" } ) }
  @@if (site === 'cx'  ) { @@include( './offlineTx-3.tpl',     { "site": "cx"  } ) }

  @@if (site === 'web' ) { @@include( './offlineTx-modal.tpl', { "site": "web" } ) }
  @@if (site === 'cx'  ) { @@include( './offlineTx-modal.tpl', { "site": "cx"  } ) }

</main>
