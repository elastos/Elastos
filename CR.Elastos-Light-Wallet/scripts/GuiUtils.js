'use strict';

const get = (id) => {
  const elt = document.getElementById(id);
  if (elt == null) {
    app.trace();
    throw new Error('elt is null:' + id);
  }
  return elt;
};

const setValue = (id, value) => {
  get(id).value = value;
};

exports.get = get;
exports.setValue = setValue;
