<!--  <span class="hidden-xs">3.12.0</span> -->

<header class="header {{curNode.name}} {{curNode.service}} {{curNode.service}} nav-index-{{gService.currentTab}}"
        aria-label="header"
        ng-controller='tabsCtrl'>


    <section class="header__wrap">
      <div class="container">

        @@if (site === 'web' ) {
          <a class="header__brand" href="/" aria-label="Go to homepage">
            <img src="images/logo.svg" height="53" width="190" alt="MyCrypto Logo" />
          </a>
        }
        @@if (site === 'cx'  ) {
          <a class="header__brand" href="/cx-wallet.html" aria-label="Go to homepage">
            <img src="images/logo.svg" height="48" width="166" alt="MyCrypto Logo" />
          </a>
        }

        <div class="header__dropdowns">

          <span class="dropdown dropdown-gas" ng-cloak>
            <a tabindex="0"
               aria-haspopup="true"
               aria-label="adjust gas price"
               class="dropdown-toggle"
               ng-click="dropdownGasPrice = !dropdownGasPrice">
                <p translate="OFFLINE_Step2_Label_3">Gas Price</p>
                {{gas.value}} Gwei
            </a>
            <ul class="dropdown-menu" ng-show="dropdownGasPrice">
              <div class="header--gas">
                <span translate="OFFLINE_Step2_Label_3">
                  Gas Price</span>: {{gas.value}} Gwei
                  <input type="range" ng-model="gas.value" min="{{gas.min}}" max="{{gas.max}}" step="{{gas.step}}" ng-change="gasChanged()"/>
                  <p class="small col-xs-4 text-left">Cheaper</p>
                <p class="small col-xs-4 text-center">Balanced</p>
                <p class="small col-xs-4 text-right">Faster</p>
                <p class="small" style="white-space:normal;font-weight:300;margin: 1rem 0 0;" translate="GAS_PRICE_Desc"></p>
                <a class="small" translate="x_ReadMore" href="https://support.mycrypto.com/gas/what-is-gas-ethereum.html" target="_blank" rel="noopener noreferrer"></a>
              </div>
            </ul>
          </span>

          <span class="dropdown dropdown-node" ng-cloak>
            <a tabindex="0"
               aria-haspopup="true"
               aria-label="change node. current node {{curNode.name}} node by {{curNode.service}}"
               class="dropdown-toggle"
               ng-click="dropdownNode = !dropdownNode">
                 <p translate="X_Network">Network</p>
                 {{curNode.name}} <small>({{curNode.service}})</small>
            </a>
            <ul class="dropdown-menu" ng-show="dropdownNode">
              <li ng-repeat="(key, value) in nodeList">
                <a ng-class="{true:'active'}[curNode == key]" ng-click="changeNode(key)">
                  {{value.name}}
                  <small> ({{value.service}}) </small>
                  <img ng-show="value.service=='Custom'" src="images/icon-remove.svg" class="node-remove" title="Remove Custom Node" ng-click="removeNodeFromLocal(value.name)"/>
                </a>
              </li>
              <li>
                <a ng-click="customNodeModal.open(); dropdownNode = !dropdownNode;" translate="X_Network_Custom">
                  Add Custom Network / Node
                </a>
              </li>
            </ul>
          </span>

          <span class="dropdown dropdown-lang" ng-cloak>
            <a tabindex="0"
               aria-haspopup="true"
               aria-expanded="false"
               aria-label="change language. current language {{curLang}}"
               class="dropdown-toggle"
               ng-click="dropdown = !dropdown">
                <p> Language </p>
                {{curLang}}
            </a>
            <ul class="dropdown-menu" ng-show="dropdown">
              <li><a ng-class="{true:'active'}[curLang=='Català']"          ng-click="changeLanguage('ca','Català'         )"> Català          </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Deutsch']"         ng-click="changeLanguage('de','Deutsch'        )"> Deutsch         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Ελληνικά']"        ng-click="changeLanguage('el','Ελληνικά'       )"> Ελληνικά        </a></li>
              <li><a ng-class="{true:'active'}[curLang=='English']"         ng-click="changeLanguage('en','English'        )"> English         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Español']"         ng-click="changeLanguage('es','Español'        )"> Español         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Farsi']"           ng-click="changeLanguage('fa','Farsi'          )"> Farsi           </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Suomi']"           ng-click="changeLanguage('fi','Suomi'          )"> Suomi           </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Français']"        ng-click="changeLanguage('fr','Français'       )"> Français        </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Magyar']"          ng-click="changeLanguage('hu','Magyar'         )"> Magyar          </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Haitian Creole']"  ng-click="changeLanguage('ht','Haitian Creole' )"> Haitian Creole  </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Indonesian']"      ng-click="changeLanguage('id','Indonesian'     )"> Bahasa Indonesia</a></li>
              <li><a ng-class="{true:'active'}[curLang=='Italiano']"        ng-click="changeLanguage('it','Italiano'       )"> Italiano        </a></li>
              <li><a ng-class="{true:'active'}[curLang=='日本語']"           ng-click="changeLanguage('ja','日本語'          )"> 日本語          </a></li>
              <li><a ng-class="{true:'active'}[curLang=='한국어']"            ng-click="changeLanguage('ko','한국어'          )"> 한국어           </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Nederlands']"      ng-click="changeLanguage('nl','Nederlands'     )"> Nederlands      </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Norsk Bokmål']"    ng-click="changeLanguage('no','Norsk Bokmål'   )"> Norsk Bokmål    </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Polski']"          ng-click="changeLanguage('pl','Polski'         )"> Polski          </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Português']"       ng-click="changeLanguage('pt','Português'      )"> Português       </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Русский']"         ng-click="changeLanguage('ru','Русский'        )"> Русский         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='ภาษาไทย']"         ng-click="changeLanguage('th','ภาษาไทย'        )"> ภาษาไทย         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Türkçe']"          ng-click="changeLanguage('tr','Türkçe'         )"> Türkçe          </a></li>
              <li><a ng-class="{true:'active'}[curLang=='Tiếng Việt']"      ng-click="changeLanguage('vi','Tiếng Việt'     )"> Tiếng Việt      </a></li>
              <li><a ng-class="{true:'active'}[curLang=='简体中文']"         ng-click="changeLanguage('zhcn','简体中文'      )"> 简体中文         </a></li>
              <li><a ng-class="{true:'active'}[curLang=='繁體中文']"         ng-click="changeLanguage('zhtw','繁體中文'      )"> 繁體中文         </a></li>
              <li role="separator" class="divider"></li>
              <li><a data-toggle="modal" data-target="#disclaimerModal" translate="FOOTER_4"> Disclaimer </a></li>
            </ul>
          </span>

        </div>

      </div>
  </section>


  <nav role="navigation" aria-label="main navigation" class="nav-container overflowing">
    <div class="container">
      <a aria-hidden="true"
         ng-show="showLeftArrow"
         class="nav-arrow-left"
         ng-click="scrollLeft(100);"
         ng-mouseover="scrollHoverIn(true,2);" ng-mouseleave="scrollHoverOut()">&#171;</a>
      <div class="nav-scroll">
        <ul class="nav-inner">
          @@if (site === 'web' ) {
            <li ng-repeat="tab in tabNames track by $index" \
                class="nav-item {{tab.name}}" \
                ng-class="{active: $index==gService.currentTab}"
                ng-show="tab.mew"
                ng-click="tabClick($index)">
                  <a tabindex="0" aria-label="nav item: {{tab.name | translate}}" translate="{{tab.name}}"></a>
            </li>
          }
          @@if (site === 'cx' ) {
            <li ng-repeat="tab in tabNames track by $index" \
                class="nav-item {{tab.name}}" \
                ng-class="{active: $index==gService.currentTab}"
                ng-show="tab.cx"
                ng-click="tabClick($index)">
                  <a tabindex="0" aria-label="nav item: {{tab.name | translate}}" translate="{{tab.name}}"></a>
            </li>
          }
          <li class="nav-item help">
            <a href="https://support.mycrypto.com/" target="_blank" rel="noopener noreferrer">
              <span translate="NAV_Help">
                Help
              </span>
            </a>
          </li>
        </ul>
      </div>

      <a aria-hidden="true"
         ng-show="showRightArrow"
         class="nav-arrow-right"
         ng-click="scrollRight(100);"
         ng-mouseover="scrollHoverIn(false,2);"
         ng-mouseleave="scrollHoverOut()">&#187;
      </a>
    </div>
  </nav>

  @@if (site === 'web' ) { @@include( './header-node-modal.tpl', { "site": "web" } ) }
  @@if (site === 'cx'  ) { @@include( './header-node-modal.tpl', { "site": "cx"  } ) }

</header>
