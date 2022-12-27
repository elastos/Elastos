'use strict';

const get = (id) => {
  const elt = document.getElementById(id);
  if (elt == null) {
    throw new Error('elt is null:' + id);
  }
  return elt;
};

const setValue = (id, value) => {
  get(id).value = value;
};

const getValue = (id) => {
  return get(id).value;
};


const hide = (id) => {
  get(id).style = 'display:none;';
};

const show = (id) => {
  get(id).style = 'display:default;';
};

exports.get = get;
exports.setValue = setValue;
exports.getValue = getValue;
exports.hide = hide;
exports.show = show;
