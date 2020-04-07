let app;

const init = (_app) => {
  app = _app;
};

const hide = (id) => {
  get(id).style = 'display:none;';
};

const show = (id) => {
  get(id).style = 'display:default;';
};

const hideEverything = () => {
  hide('home');
  hide('login-private-key');
  hide('landing');
};

const showHome = () => {
  hideEverything();
  app.clearSendData();
  show('home');
};

const showLoginPrivateKey = () => {
  hideEverything();
  app.clearSendData();
  show('login-private-key');
};

exports.init = init;
exports.showLoginPrivateKey = showLoginPrivateKey;
exports.showHome = showHome;
