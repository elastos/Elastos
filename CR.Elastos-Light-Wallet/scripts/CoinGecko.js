'use strict';

let priceData;

let App;

const init = (_App) => {
  App = _App;
};

const requestPriceData = async () => {
  const url = 'https://api.coingecko.com/api/v3/simple/price?ids=elastos&vs_currencies=usd';
  const xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    // console.log('requestPriceData', this.readyState, this.response);
    if (this.readyState == 4) {
      if (this.status == 200) {
        priceData = JSON.parse(this.response);
      } else {
        priceData = {'status': this.status, 'statusText': this.statusText, 'response': this.response};
      }
      setImmediate(() => {
        App.renderApp;
      });
    }
  };
  xhttp.responseType = 'text';
  xhttp.open('GET', url, true);
  xhttp.send();
};

const getPriceData = () => {
  return priceData;
};

exports.init = init;
exports.requestPriceData = requestPriceData;
exports.getPriceData = getPriceData;
