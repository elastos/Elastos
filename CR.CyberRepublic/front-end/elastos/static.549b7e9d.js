(function webpackUniversalModuleDefinition(root, factory) {
	if(typeof exports === 'object' && typeof module === 'object')
		module.exports = factory();
	else if(typeof define === 'function' && define.amd)
		define([], factory);
	else {
		var a = factory();
		for(var i in a) (typeof exports === 'object' ? exports : root)[i] = a[i];
	}
})(typeof self !== 'undefined' ? self : this, function() {
return /******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, {
/******/ 				configurable: false,
/******/ 				enumerable: true,
/******/ 				get: getter
/******/ 			});
/******/ 		}
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "/";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 42);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports) {

module.exports = require("react");

/***/ }),
/* 1 */
/***/ (function(module, exports) {

module.exports = require("styled-components");

/***/ }),
/* 2 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});
var breakPoint = exports.breakPoint = {
    mobile: '768px'
};

/***/ }),
/* 3 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});
var bg = exports.bg = {
    dark: '#0F2D3B',
    lightGray: '#F1F5F8',
    selected: '#45427e'
};
var text = exports.text = {
    white: '#fcfcfc',
    blue: '#4f789c',
    darkBlue: '#324256',
    navy: '#0f2d3a'
};
var border = exports.border = {
    light: '#cfdae4'
};

/***/ }),
/* 4 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/col");

/***/ }),
/* 5 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/row");

/***/ }),
/* 6 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/col/style");

/***/ }),
/* 7 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/row/style");

/***/ }),
/* 8 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 9 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  text-align: left;\n'], ['\n  text-align: left;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  font-weight: ', ';\n  font-size: ', ';\n  color: ', ';\n'], ['\n  font-weight: ', ';\n  font-size: ', ';\n  color: ', ';\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  width: 100px;\n  height: 1px;\n  padding-left: 10px;\n'], ['\n  width: 100px;\n  height: 1px;\n  padding-left: 10px;\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  width: 50px;\n'], ['\n  width: 50px;\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  padding-top: 24px;\n  font-size: 36px;\n'], ['\n  padding-top: 24px;\n  font-size: 36px;\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _ecosystem_title_line = __webpack_require__(18);

var _ecosystem_title_line2 = _interopRequireDefault(_ecosystem_title_line);

var _cr_logo = __webpack_require__(31);

var _cr_logo2 = _interopRequireDefault(_cr_logo);

var _color = __webpack_require__(3);

__webpack_require__(12);

__webpack_require__(66);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var _props = this.props,
          label = _props.label,
          title = _props.title,
          titleSize = _props.titleSize,
          titleColor = _props.titleColor,
          titleWeight = _props.titleWeight,
          img = _props.img,
          line = _props.line,
          style = _props.style;

      return _react2.default.createElement(
        Container,
        { style: style },
        _react2.default.createElement(
          Title,
          { titleColor: titleColor || 'blue', titleSize: titleSize || '10px', titleWeight: titleWeight || 400 },
          label || 'ELASTOS',
          _react2.default.createElement(Line, { src: line || _ecosystem_title_line2.default })
        ),
        img && _react2.default.createElement(StyledImg, { src: _cr_logo2.default }),
        title && _react2.default.createElement(
          SubTitle,
          null,
          title
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var Title = _styledComponents2.default.div(_templateObject2, function (props) {
  return props.titleWeight;
}, function (props) {
  return props.titleSize;
}, function (props) {
  return _color.text[props.titleColor];
});
var Line = _styledComponents2.default.img(_templateObject3);
var StyledImg = _styledComponents2.default.img(_templateObject4);
var SubTitle = _styledComponents2.default.span(_templateObject5);

/***/ }),
/* 10 */
/***/ (function(module, exports) {

module.exports = require("react-static");

/***/ }),
/* 11 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _modal = __webpack_require__(15);

var _modal2 = _interopRequireDefault(_modal);

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n  background: url(', ') no-repeat;\n  background-size: cover;\n  padding: 30px;\n  color: white;\n'], ['\n  position: relative;\n  background: url(', ') no-repeat;\n  background-size: cover;\n  padding: 30px;\n  color: white;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  font-size: 30px;\n  font-weight: lighter;\n  color: white;\n'], ['\n  font-size: 30px;\n  font-weight: lighter;\n  color: white;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  font-size: 14px;\n  font-weight: lighter;\n  margin-left: 20px;\n'], ['\n  font-size: 14px;\n  font-weight: lighter;\n  margin-left: 20px;\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  margin: 30px 5px;\n  width: 630px;\n'], ['\n  margin: 30px 5px;\n  width: 630px;\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  max-width: 700px;\n  margin: auto;\n'], ['\n  max-width: 700px;\n  margin: auto;\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  position: relative;\n  color: white;\n  text-align: center;\n  max-width: 210px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: flex-start;\n  background-color: ', ';\n  padding: 15px;\n'], ['\n  position: relative;\n  color: white;\n  text-align: center;\n  max-width: 210px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: flex-start;\n  background-color: ', ';\n  padding: 15px;\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  position: absolute;\n  top: 20px;\n  right: 20px;\n  width: 35px;\n  cursor: pointer;\n'], ['\n  position: absolute;\n  top: 20px;\n  right: 20px;\n  width: 35px;\n  cursor: pointer;\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  font-size: 10px;\n  margin-bottom: 0.8em;\n'], ['\n  font-size: 10px;\n  margin-bottom: 0.8em;\n']);

__webpack_require__(6);

__webpack_require__(16);

__webpack_require__(7);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _btn_close_purp = __webpack_require__(8);

var _btn_close_purp2 = _interopRequireDefault(_btn_close_purp);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var modalStyle = {};
var bodyStyle = {
  padding: 0
};

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var _props = this.props,
          title = _props.title,
          subtitle = _props.subtitle,
          onClose = _props.onClose,
          bgImg = _props.bgImg,
          bodyImg = _props.bodyImg,
          textArr = _props.textArr,
          closeImg = _props.closeImg;

      var span = textArr.length > 0 ? 24 / Math.floor(textArr.length) : 24;
      var textNode = textArr.map(function (obj, index) {
        var innerTextNode = _react2.default.createElement(
          ItemText,
          null,
          obj.text
        );
        if (Array.isArray(obj.text)) {
          innerTextNode = obj.text.map(function (v, i) {
            return _react2.default.createElement(
              ItemText,
              { key: i },
              v
            );
          });
        }
        return _react2.default.createElement(
          Item,
          { span: span, key: index, bgcolor: obj.bgcolor },
          _react2.default.createElement(
            ItemTitle,
            null,
            obj.title
          ),
          innerTextNode
        );
      });
      var body = bodyImg ? _react2.default.createElement(Body, { src: bodyImg }) : _react2.default.createElement('div', { style: { height: 250 } });
      return _react2.default.createElement(
        _modal2.default,
        { visible: this.props.visible, onCancel: this.props.onClose, footer: null, width: 700, closable: false, style: modalStyle, bodyStyle: bodyStyle },
        _react2.default.createElement(
          Container,
          { bgImg: bgImg },
          _react2.default.createElement(
            Title,
            null,
            title,
            _react2.default.createElement(
              SubTitle,
              null,
              subtitle
            )
          ),
          _react2.default.createElement(Icon, { src: closeImg || _btn_close_purp2.default, onClick: onClose }),
          body,
          _react2.default.createElement(
            ListContainer,
            null,
            _react2.default.createElement(
              _row2.default,
              { type: 'flex', justify: 'space-between', gutter: 8 },
              textNode
            )
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject, function (props) {
  return props.bgImg;
});
var Title = _styledComponents2.default.h2(_templateObject2);
var SubTitle = _styledComponents2.default.span(_templateObject3);
var Body = _styledComponents2.default.img(_templateObject4);
var ListContainer = _styledComponents2.default.div(_templateObject5);
var Item = (0, _styledComponents2.default)(_col2.default)(_templateObject6, function (props) {
  return props.bgcolor || 'none';
});
var Icon = _styledComponents2.default.img(_templateObject7);
var ItemTitle = _styledComponents2.default.div(_templateObject8);
var ItemText = _styledComponents2.default.div(_templateObject9);

/***/ }),
/* 12 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 13 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _modal = __webpack_require__(15);

var _modal2 = _interopRequireDefault(_modal);

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n  background: url(', ') no-repeat;\n  background-size: cover;\n  padding: 30px;\n  color: white;\n'], ['\n  position: relative;\n  background: url(', ') no-repeat;\n  background-size: cover;\n  padding: 30px;\n  color: white;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  font-size: 30px;\n  font-weight: lighter;\n  color: white;\n'], ['\n  font-size: 30px;\n  font-weight: lighter;\n  color: white;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  font-size: 14px;\n  font-weight: lighter;\n  margin-left: 20px;\n'], ['\n  font-size: 14px;\n  font-weight: lighter;\n  margin-left: 20px;\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  margin: 30px 5px;\n  width: 630px;\n'], ['\n  margin: 30px 5px;\n  width: 630px;\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  max-width: 700px;\n  margin: auto;\n'], ['\n  max-width: 700px;\n  margin: auto;\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  position: relative;\n  color: white;\n  text-align: center;\n  max-width: 210px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: flex-start;\n  background-color: ', ';\n  padding: 15px;\n'], ['\n  position: relative;\n  color: white;\n  text-align: center;\n  max-width: 210px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: flex-start;\n  background-color: ', ';\n  padding: 15px;\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  position: absolute;\n  top: 20px;\n  right: 20px;\n  width: 35px;\n  cursor: pointer;\n'], ['\n  position: absolute;\n  top: 20px;\n  right: 20px;\n  width: 35px;\n  cursor: pointer;\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  font-size: 10px;\n  margin-bottom: 0.8em;\n'], ['\n  font-size: 10px;\n  margin-bottom: 0.8em;\n']);

__webpack_require__(6);

__webpack_require__(16);

__webpack_require__(7);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _btn_close_purp = __webpack_require__(8);

var _btn_close_purp2 = _interopRequireDefault(_btn_close_purp);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var modalStyle = {};
var bodyStyle = {
  padding: 0
};

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var _props = this.props,
          title = _props.title,
          subtitle = _props.subtitle,
          onClose = _props.onClose,
          bgImg = _props.bgImg,
          bodyImg = _props.bodyImg,
          textArr = _props.textArr,
          closeImg = _props.closeImg;

      var span = textArr.length > 0 ? 24 / Math.floor(textArr.length) : 24;
      var textNode = textArr.map(function (obj, index) {
        var innerTextNode = _react2.default.createElement(
          ItemText,
          null,
          obj.text
        );
        if (Array.isArray(obj.text)) {
          innerTextNode = obj.text.map(function (v, i) {
            return _react2.default.createElement(
              ItemText,
              { key: i },
              v
            );
          });
        }
        return _react2.default.createElement(
          Item,
          { span: span, key: index, bgcolor: obj.bgcolor },
          _react2.default.createElement(
            ItemTitle,
            null,
            obj.title
          ),
          innerTextNode
        );
      });
      var body = bodyImg ? _react2.default.createElement(Body, { src: bodyImg }) : _react2.default.createElement('div', { style: { height: 250 } });
      return _react2.default.createElement(
        _modal2.default,
        { visible: this.props.visible, onCancel: this.props.onClose, footer: null, width: 700, closable: false, style: modalStyle, bodyStyle: bodyStyle },
        _react2.default.createElement(
          Container,
          { bgImg: bgImg },
          _react2.default.createElement(
            Title,
            null,
            title,
            _react2.default.createElement(
              SubTitle,
              null,
              subtitle
            )
          ),
          _react2.default.createElement(Icon, { src: closeImg || _btn_close_purp2.default, onClick: onClose }),
          body,
          _react2.default.createElement(
            ListContainer,
            null,
            _react2.default.createElement(
              _row2.default,
              { type: 'flex', justify: 'space-between', gutter: 8 },
              textNode
            )
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject, function (props) {
  return props.bgImg;
});
var Title = _styledComponents2.default.h2(_templateObject2);
var SubTitle = _styledComponents2.default.span(_templateObject3);
var Body = _styledComponents2.default.img(_templateObject4);
var ListContainer = _styledComponents2.default.div(_templateObject5);
var Item = (0, _styledComponents2.default)(_col2.default)(_templateObject6, function (props) {
  return props.bgcolor || 'none';
});
var Icon = _styledComponents2.default.img(_templateObject7);
var ItemTitle = _styledComponents2.default.div(_templateObject8);
var ItemText = _styledComponents2.default.div(_templateObject9);

/***/ }),
/* 14 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/team-bg@2x.81fe9aa0.png";

/***/ }),
/* 15 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/modal");

/***/ }),
/* 16 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/modal/style");

/***/ }),
/* 17 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 18 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAASgAAAACCAYAAADrR0H9AAAAJUlEQVRIiWM8e+vpf4ZRMApGwSgYhIBpNFJGwSgYBYMSMDAwAADhUAOP/Ql4fgAAAABJRU5ErkJggg=="

/***/ }),
/* 19 */
/***/ (function(module, exports) {

module.exports = require("react-responsive");

/***/ }),
/* 20 */
/***/ (function(module, exports) {

module.exports = function(module) {
	if(!module.webpackPolyfill) {
		module.deprecate = function() {};
		module.paths = [];
		// module.parent = undefined by default
		if(!module.children) module.children = [];
		Object.defineProperty(module, "loaded", {
			enumerable: true,
			get: function() {
				return module.l;
			}
		});
		Object.defineProperty(module, "id", {
			enumerable: true,
			get: function() {
				return module.i;
			}
		});
		module.webpackPolyfill = 1;
	}
	return module;
};


/***/ }),
/* 21 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/* WEBPACK VAR INJECTION */(function(module) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.cacheProm = exports.loadFromPromiseCache = exports.cacheExport = exports.loadFromCache = exports.callForString = exports.createElement = exports.findExport = exports.resolveExport = exports.requireById = exports.tryRequire = exports.DefaultError = exports.DefaultLoading = exports.babelInterop = exports.isWebpack = exports.isServer = exports.isTest = undefined;

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var isTest = exports.isTest = "production" === 'test';
var isServer = exports.isServer = !(typeof window !== 'undefined' && window.document && window.document.createElement);

var isWebpack = exports.isWebpack = function isWebpack() {
  return typeof __webpack_require__ !== 'undefined';
};
var babelInterop = exports.babelInterop = function babelInterop(mod) {
  return mod && (typeof mod === 'undefined' ? 'undefined' : _typeof(mod)) === 'object' && mod.__esModule ? mod.default : mod;
};

var DefaultLoading = exports.DefaultLoading = function DefaultLoading() {
  return _react2.default.createElement(
    'div',
    null,
    'Loading...'
  );
};
var DefaultError = exports.DefaultError = function DefaultError(_ref) {
  var error = _ref.error;
  return _react2.default.createElement(
    'div',
    null,
    'Error: ',
    error && error.message
  );
};

var tryRequire = exports.tryRequire = function tryRequire(id) {
  try {
    return requireById(id);
  } catch (err) {
    // warn if there was an error while requiring the chunk during development
    // this can sometimes lead the server to render the loading component.
    if (false) {
      console.warn('chunk not available for synchronous require yet: ' + id + ': ' + err.message, err.stack);
    }
  }

  return null;
};

var requireById = exports.requireById = function requireById(id) {
  if (!isWebpack() && typeof id === 'string') {
    return module.require(id);
  }

  return __webpack_require__(id);
};

var resolveExport = exports.resolveExport = function resolveExport(mod, key, onLoad, chunkName, props, context, modCache) {
  var isSync = arguments.length > 7 && arguments[7] !== undefined ? arguments[7] : false;

  var exp = findExport(mod, key);
  if (onLoad && mod) {
    var _isServer = typeof window === 'undefined';
    var info = { isServer: _isServer, isSync: isSync };
    onLoad(mod, info, props, context);
  }
  if (chunkName && exp) cacheExport(exp, chunkName, props, modCache);
  return exp;
};

var findExport = exports.findExport = function findExport(mod, key) {
  if (typeof key === 'function') {
    return key(mod);
  } else if (key === null) {
    return mod;
  }

  return mod && (typeof mod === 'undefined' ? 'undefined' : _typeof(mod)) === 'object' && key ? mod[key] : babelInterop(mod);
};

var createElement = exports.createElement = function createElement(Component, props) {
  return _react2.default.isValidElement(Component) ? _react2.default.cloneElement(Component, props) : _react2.default.createElement(Component, props);
};

var callForString = exports.callForString = function callForString(strFun, props) {
  return typeof strFun === 'function' ? strFun(props) : strFun;
};

var loadFromCache = exports.loadFromCache = function loadFromCache(chunkName, props, modCache) {
  return !isServer && modCache[callForString(chunkName, props)];
};

var cacheExport = exports.cacheExport = function cacheExport(exp, chunkName, props, modCache) {
  return modCache[callForString(chunkName, props)] = exp;
};

var loadFromPromiseCache = exports.loadFromPromiseCache = function loadFromPromiseCache(chunkName, props, promisecache) {
  return promisecache[callForString(chunkName, props)];
};

var cacheProm = exports.cacheProm = function cacheProm(pr, chunkName, props, promisecache) {
  return promisecache[callForString(chunkName, props)] = pr;
};
/* WEBPACK VAR INJECTION */}.call(exports, __webpack_require__(20)(module)))

/***/ }),
/* 22 */
/***/ (function(module, exports) {

module.exports = require("prop-types");

/***/ }),
/* 23 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _templateObject = _taggedTemplateLiteral(['\n  text-align: center;\n'], ['\n  text-align: center;\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _hero = __webpack_require__(54);

var _hero2 = _interopRequireDefault(_hero);

var _industry = __webpack_require__(81);

var _industry2 = _interopRequireDefault(_industry);

var _ecosystem = __webpack_require__(105);

var _ecosystem2 = _interopRequireDefault(_ecosystem);

var _cr = __webpack_require__(122);

var _cr2 = _interopRequireDefault(_cr);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }
// import Partner from '../components/partner'


exports.default = function () {
  return _react2.default.createElement(
    Container,
    null,
    _react2.default.createElement(_hero2.default, null),
    _react2.default.createElement(_industry2.default, null),
    _react2.default.createElement(_ecosystem2.default, null),
    _react2.default.createElement(_cr2.default, null)
  );
};

var Container = _styledComponents2.default.div(_templateObject);

/***/ }),
/* 24 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 25 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_blockchain_info.648a6910.png";

/***/ }),
/* 26 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_blockchain_bg.76aa06aa.png";

/***/ }),
/* 27 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 28 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_eth_bg.c8872dd0.png";

/***/ }),
/* 29 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_runtime_bg.523d1bfe.png";

/***/ }),
/* 30 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_carrier_bg.6f10bf56.png";

/***/ }),
/* 31 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/cr_logo.6b858c10.png";

/***/ }),
/* 32 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/menu");

/***/ }),
/* 33 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/dropdown");

/***/ }),
/* 34 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/menu/style");

/***/ }),
/* 35 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/dropdown/style");

/***/ }),
/* 36 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAPEAAAACCAYAAACJzTq+AAAAIUlEQVQ4jWPk17VyYBgFo2AUDFnANBp1o2AUDGHAwMAAAHDuALqtPgMiAAAAAElFTkSuQmCC"

/***/ }),
/* 37 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _templateObject = _taggedTemplateLiteral(['\n  margin: 0 auto;\n  \n  background: url(', ') no-repeat;\n  background-position: 125% top;\n  background-size: 50%;\n'], ['\n  margin: 0 auto;\n  \n  background: url(', ') no-repeat;\n  background-position: 125% top;\n  background-size: 50%;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  overflow: visible;\n  \n  max-width: 950px;\n  height: 100%;\n  margin: 0 auto;\n  \n  @media only screen and (max-width: ', ') {\n    padding: 24px 24px 72px 24px;\n  }\n'], ['\n  overflow: visible;\n  \n  max-width: 950px;\n  height: 100%;\n  margin: 0 auto;\n  \n  @media only screen and (max-width: ', ') {\n    padding: 24px 24px 72px 24px;\n  }\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  padding-top: 48px;\n  font-size: 64px;\n  color: ', ';\n'], ['\n  padding-top: 48px;\n  font-size: 64px;\n  color: ', ';\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  margin: 24px 0;\n  padding-left: 4px;\n  width: 60%;\n'], ['\n  margin: 24px 0;\n  padding-left: 4px;\n  width: 60%;\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  margin: 36px 0;\n'], ['\n  margin: 36px 0;\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  font-size: 24px;\n  font-weight: 300;\n  color: ', '\n'], ['\n  font-size: 24px;\n  font-weight: 300;\n  color: ', '\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  font-size: 14px;\n  font-weight: 200;\n  padding-left: 1px;\n'], ['\n  font-size: 14px;\n  font-weight: 200;\n  padding-left: 1px;\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 16px;\n  font-weight: 400;\n'], ['\n  font-size: 16px;\n  font-weight: 400;\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  clearfix: both; \n'], ['\n  clearfix: both; \n']),
    _templateObject10 = _taggedTemplateLiteral(['\n  float: left;\n  padding-top: 24px;\n  width: 15%;\n  margin-right: 24px;\n'], ['\n  float: left;\n  padding-top: 24px;\n  width: 15%;\n  margin-right: 24px;\n']),
    _templateObject11 = _taggedTemplateLiteral(['\n  display: block;\n  margin: 0 auto;\n  width: 100%;\n  padding: 12px 0;\n'], ['\n  display: block;\n  margin: 0 auto;\n  width: 100%;\n  padding: 12px 0;\n']),
    _templateObject12 = _taggedTemplateLiteral(['\n  padding: 12px 0;\n  text-align: left;\n  font-weight: 400;\n  font-size: 18px;\n  color: ', '\n'], ['\n  padding: 12px 0;\n  text-align: left;\n  font-weight: 400;\n  font-size: 18px;\n  color: ', '\n']),
    _templateObject13 = _taggedTemplateLiteral(['\n  clear: both;\n'], ['\n  clear: both;\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _variable = __webpack_require__(2);

var _color = __webpack_require__(3);

var _TitleWithLine = __webpack_require__(9);

var _TitleWithLine2 = _interopRequireDefault(_TitleWithLine);

var _teamBg2x = __webpack_require__(14);

var _teamBg2x2 = _interopRequireDefault(_teamBg2x);

var _cr_title_line = __webpack_require__(36);

var _cr_title_line2 = _interopRequireDefault(_cr_title_line);

var _teamMembers = __webpack_require__(127);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }
// import { Link } from '@reach/router'


function App() {
  return _react2.default.createElement(
    'div',
    null,
    _react2.default.createElement(
      'div',
      { className: 'content' },
      _react2.default.createElement(
        Container,
        { className: 'container' },
        _react2.default.createElement(
          ContainerInner,
          { className: 'container-inner' },
          _react2.default.createElement(
            Title,
            null,
            'Elastos Teams'
          ),
          _react2.default.createElement(
            Desc,
            null,
            'Elastos is structured into small teams, or \u201Ccompanies,\u201D who are divided under individual entities. Please note that some employees and contributors work for multiple teams, some are not currently on a team and more teams will be added in the future as they are formed and funded.'
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Foundation', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              'Directors of the Elastos Foundation'
            ),
            _react2.default.createElement(
              MemberContainer,
              null,
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.RongChenImg }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Rong Chen'
                )
              ),
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.FengHanImg }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Feng Han'
                )
              ),
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.BenLeeImg }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Ben Lee'
                )
              ),
              _react2.default.createElement(ClearFix, null)
            )
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Cyber Republic Council Preparatory Committee', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              'Members of the CR Preparatory Committee'
            ),
            _react2.default.createElement(
              MemberContainer,
              null,
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.YipengSu }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Yipeng Su'
                )
              ),
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.KevinZhang }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Kevin Zhang'
                )
              ),
              _react2.default.createElement(
                Member,
                null,
                _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.PlaceholderImg, placeholder: 'true' }),
                _react2.default.createElement(
                  MemberTitle,
                  null,
                  'Feng Zhang'
                )
              ),
              _react2.default.createElement(ClearFix, null)
            )
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Blockchain Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Shunan Yu with 28 engineers, responsible for'
              ),
              '- Elastos mainchain, sidechains, cross chain asset transfer',
              _react2.default.createElement('br', null),
              '- Blockchain explorer, Wallet, on-chain services like DPoS election etc.',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.ShunanYuImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Shunan Yu'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Carrier Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Jingyu Niu and Zhilong Tang with 9 engineers, responsible for:'
              ),
              '- DHT, P2P communication protocol',
              _react2.default.createElement('br', null),
              '- Relay service, Bootstrap node implementation',
              _react2.default.createElement('br', null),
              '- Carrier SDK for multiple platforms',
              _react2.default.createElement('br', null),
              '- Reference server and client apps',
              _react2.default.createElement('br', null),
              '- Authorized user connection via DID',
              _react2.default.createElement('br', null),
              '- File transfer, group messaging and off-line messaging',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.JingyuNiuImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Jingyu Niu'
              )
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.PlaceholderImg, placeholder: 'true' }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Zhilong Tang'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Runtime Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Jingyu Niu and Zhiming Rao with 10 engineers, responsible for:'
              ),
              '\u2013 Elastos DApp browser framework',
              _react2.default.createElement('br', null),
              '\u2013 File management, local device accessibility, permission management',
              _react2.default.createElement('br', null),
              '\u2013 Elastos DApp full cycle toolchain support',
              _react2.default.createElement('br', null),
              '\u2013 Embedded DID, Carrier, Wallet plugins',
              _react2.default.createElement('br', null)
            )
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Consulting Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Shijun Song with 18 engineers, responsible for:'
              ),
              '\u2013 Elastos Eco-System project technical support',
              _react2.default.createElement('br', null),
              '\u2013 Elastos Developers Website (Chinese Version)',
              _react2.default.createElement('br', null),
              '\u2013 Business development',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.SongSjunImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Shijun Song'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos DevStudio Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Kiran Pachhai and Clarence Liu with 8 employees, 1 external design agency, and 1 external growth agency, responsible for:'
              ),
              '\u2013 Elastos Development toolchain kit and testbed',
              _react2.default.createElement('br', null),
              '\u2013 Documentation',
              _react2.default.createElement('br', null),
              '\u2013 Global developer community support',
              _react2.default.createElement('br', null),
              '\u2013 Design',
              _react2.default.createElement('br', null),
              '\u2013 Technical marketing',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.KiranPachhaiImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Kiran Pachhai'
              )
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.ClarenceLiuImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Clarence Liu'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos BizFramework (DMA) Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Brian Xin with 8 engineers, responsible for:'
              ),
              '\u2013 Elastos Decentralized e-Commerce platform',
              _react2.default.createElement('br', null),
              '\u2013 Digital asset management',
              _react2.default.createElement('br', null),
              '\u2013 Personal data management',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.BrianXinImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Brian Xin'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Storage (Hive) Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Bo Leng with 7 engineers, responsible for:'
              ),
              '\u2013 Elastos Decentralized File System',
              _react2.default.createElement('br', null),
              '\u2013 IPFS compatible APIs',
              _react2.default.createElement('br', null),
              '\u2013 Key Value Store',
              _react2.default.createElement('br', null),
              '\u2013 Client Native APIs for Android and iOS platforms',
              _react2.default.createElement('br', null)
            )
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Operations Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Led by Ben Lee and Rebecca Zhu with 11 people, responsible for:'
              ),
              '\u2013 SmartWeb functional support',
              _react2.default.createElement('br', null),
              '\u2013 Program management, outsourcing and inter-team coordination',
              _react2.default.createElement('br', null),
              '\u2013 Legal and accounting services',
              _react2.default.createElement('br', null),
              '\u2013 Human resources',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.BenLeeImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Ben Lee'
              )
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.RebeccaZhuImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Rebecca Zhu'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          ),
          _react2.default.createElement(
            Team,
            null,
            _react2.default.createElement(_TitleWithLine2.default, { label: 'Elastos Communications Team', titleSize: '24px', titleColor: 'navy', titleWeight: '200', line: _cr_title_line2.default }),
            _react2.default.createElement(
              TeamSubtitle,
              null,
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                _react2.default.createElement(
                  'b',
                  null,
                  'Chinese:'
                )
              ),
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                '2 writers and translators, 1 media manager, 1 community manager, 1 designer, 39 community moderators responsible for 36 community groups and 100,000 community members'
              ),
              _react2.default.createElement('br', null),
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                _react2.default.createElement(
                  'b',
                  null,
                  'English:'
                )
              ),
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Writers: Zach Warsavage + 1, technical advisor: Kiran Pachhai,  community management: Nicola Zimmerman + 7 community moderators, 1 translator, 1 external PR agency.'
              ),
              _react2.default.createElement('br', null),
              _react2.default.createElement(
                TeamLeadInfo,
                null,
                'Responsible for:'
              ),
              '\u2013 All public facing news, blog posts, end of year reports, weekly reports, written documents',
              _react2.default.createElement('br', null),
              '\u2013 Support DevStudio',
              _react2.default.createElement('br', null),
              '\u2013 Community platform management and communication',
              _react2.default.createElement('br', null),
              '\u2013 Coordination with media, interview coordination, press releases, strategic planning based on EF decisions',
              _react2.default.createElement('br', null),
              '\u2013 Internal and External translations',
              _react2.default.createElement('br', null)
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.ZachWarsavageImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Zach Warsavage'
              )
            ),
            _react2.default.createElement(
              Member,
              null,
              _react2.default.createElement(MemberPhoto, { src: _teamMembers.profilePhotos.KiranPachhaiImg }),
              _react2.default.createElement(
                MemberTitle,
                null,
                'Kiran Pachhai'
              )
            ),
            _react2.default.createElement(ClearFix, null)
          )
        )
      )
    )
  );
}
exports.default = App;

var Container = _styledComponents2.default.div(_templateObject, _teamBg2x2.default);
var ContainerInner = _styledComponents2.default.div(_templateObject2, _variable.breakPoint.mobile);
var Title = _styledComponents2.default.div(_templateObject3, _color.text.darkBlue);
var Desc = _styledComponents2.default.div(_templateObject4);
var Team = _styledComponents2.default.div(_templateObject5);
var TeamTitle = _styledComponents2.default.div(_templateObject6, _color.text.darkBlue);
var TeamSubtitle = _styledComponents2.default.span(_templateObject7);
var TeamLeadInfo = _styledComponents2.default.div(_templateObject8);
var MemberContainer = _styledComponents2.default.div(_templateObject9);
var Member = _styledComponents2.default.div(_templateObject10);
var MemberPhoto = _styledComponents2.default.img(_templateObject11);
var MemberTitle = _styledComponents2.default.p(_templateObject12, _color.text.darkBlue);
var ClearFix = _styledComponents2.default.div(_templateObject13);

/***/ }),
/* 38 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n'], ['\n  position: relative;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n'], ['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 0;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n  }\n'], ['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 0;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n\n  }\n'], ['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n\n  }\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _teamBg2x = __webpack_require__(14);

var _teamBg2x2 = _interopRequireDefault(_teamBg2x);

var _color = __webpack_require__(3);

var _variable = __webpack_require__(2);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(BgRight, { src: _teamBg2x2.default }),
        _react2.default.createElement(
          Inner,
          null,
          _react2.default.createElement(
            Title,
            null,
            'Elastos Roadmap'
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          '2019 Roadmap Coming Soon',
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          'For a recap of our 2018 accomplishments see our ',
          _react2.default.createElement(
            'a',
            { href: 'https://news.elastos.org/wp-content/uploads/2019/01/Elastos-EOY-Report-2018.pdf', target: '_blank' },
            '2018 End of Year Report'
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var BgRight = _styledComponents2.default.img(_templateObject2);
var Inner = _styledComponents2.default.div(_templateObject3, _variable.breakPoint.mobile);
var Title = _styledComponents2.default.div(_templateObject4, _color.text.navy, _variable.breakPoint.mobile);

/***/ }),
/* 39 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n'], ['\n  position: relative;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n'], ['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 0;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n  }\n'], ['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 0;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  @media only screen and (max-width: ', ') {\n\n  }\n'], ['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  @media only screen and (max-width: ', ') {\n\n  }\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  width: calc(100% - 300px);\n  max-width: 850px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n'], ['\n  width: calc(100% - 300px);\n  max-width: 850px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 200px;\n  width: 200px;\n  margin: 30px;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n  }\n'], ['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 200px;\n  width: 200px;\n  margin: 30px;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n  }\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n'], ['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n'], ['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n']);

__webpack_require__(6);

__webpack_require__(7);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _teamBg2x = __webpack_require__(14);

var _teamBg2x2 = _interopRequireDefault(_teamBg2x);

var _variable = __webpack_require__(2);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(BgRight, { src: _teamBg2x2.default }),
        _react2.default.createElement(
          Inner,
          null,
          _react2.default.createElement(
            Title,
            null,
            'Elastos Wallets'
          ),
          _react2.default.createElement(
            ListContainer,
            null,
            _react2.default.createElement(
              Item,
              { xs: 24, md: 8 },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS WEB WALLET'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Our official web wallet, and the easiest to setup and use',
                  _react2.default.createElement('br', null),
                  _react2.default.createElement('br', null),
                  _react2.default.createElement(
                    'a',
                    { href: 'https://wallet.elastos.org' },
                    'https://wallet.elastos.org'
                  )
                )
              )
            ),
            _react2.default.createElement(
              Item,
              { xs: 24, md: 8 },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS ELEPHANT WALLET'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Our ecosystem wallet with all our partners and more',
                  _react2.default.createElement('br', null),
                  _react2.default.createElement('br', null),
                  _react2.default.createElement(
                    'a',
                    { href: 'https://download.elastos.org/app/elephantwallet/elephant_wallet.apk' },
                    'Download'
                  )
                )
              )
            ),
            _react2.default.createElement(
              Item,
              { xs: 24, md: 8 },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS SPV WALLET'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Powerful wallet for advanced users.',
                  _react2.default.createElement('br', null),
                  _react2.default.createElement('br', null),
                  _react2.default.createElement(
                    'a',
                    { href: 'https://github.com/elastos/Elastos.App.Wallet.Android/releases' },
                    'https://github.com/elastos/Elastos.App.Wallet.Android/releases'
                  )
                )
              )
            )
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var BgRight = _styledComponents2.default.img(_templateObject2);
var Inner = _styledComponents2.default.div(_templateObject3, _variable.breakPoint.mobile);
var Title = _styledComponents2.default.div(_templateObject4, _variable.breakPoint.mobile);
var ListContainer = (0, _styledComponents2.default)(_row2.default)(_templateObject5, _variable.breakPoint.mobile);
var Item = (0, _styledComponents2.default)(_col2.default)(_templateObject6, _variable.breakPoint.mobile);
var ItemContent = _styledComponents2.default.div(_templateObject7);
var ItemTitle = _styledComponents2.default.div(_templateObject8, _variable.breakPoint.mobile);
var ItemText = _styledComponents2.default.div(_templateObject9, _variable.breakPoint.mobile);

/***/ }),
/* 40 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n'], ['\n  position: relative;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n  \n  @media only screen and (max-width: ', ') {\n    right: -300px;\n  }\n'], ['\n  position: absolute;\n  right: 0;\n  top: -25px;\n  width: 450px;\n  z-index: -1;\n  \n  @media only screen and (max-width: ', ') {\n    right: -300px;\n  }\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 12px;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n    padding-left: 24px;\n    padding-right: 24px;\n  }\n'], ['\n  max-width: 950px;\n  margin: 0 auto;\n  padding: 100px 12px;\n  @media only screen and (max-width: ', ') {\n    padding-top: 140px;\n    padding-left: 24px;\n    padding-right: 24px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n\n  }\n'], ['\n  font-size: 64px;\n  letter-spacing: -0.02em;\n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n\n  }\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  list-style-type: disc;\n  \n  li {\n    margin: 12px 0;\n  }\n'], ['\n  list-style-type: disc;\n  \n  li {\n    margin: 12px 0;\n  }\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _reactStatic = __webpack_require__(10);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _teamBg2x = __webpack_require__(14);

var _teamBg2x2 = _interopRequireDefault(_teamBg2x);

var _color = __webpack_require__(3);

var _TitleWithLine = __webpack_require__(9);

var _TitleWithLine2 = _interopRequireDefault(_TitleWithLine);

var _variable = __webpack_require__(2);

var _bucket_blockchain_info = __webpack_require__(25);

var _bucket_blockchain_info2 = _interopRequireDefault(_bucket_blockchain_info);

__webpack_require__(142);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(
          _reactStatic.Head,
          null,
          _react2.default.createElement('script', { type: 'text/javascript', src: 'js/elastos-v2.js' })
        ),
        _react2.default.createElement(BgRight, { src: _teamBg2x2.default }),
        _react2.default.createElement(
          Inner,
          null,
          _react2.default.createElement(
            Title,
            null,
            'Developers'
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          _react2.default.createElement(
            'p',
            null,
            'Elastos is building The Modern Internet, where end-to-end encryption comes standard, everything is open-source, the blockchain is easy to use and you can develop using a straight-forward set of APIs.'
          ),
          _react2.default.createElement(
            'p',
            null,
            'You can find our active repo here: ',
            _react2.default.createElement(
              'a',
              { href: 'https://github.com/elastos/Elastos', target: '_blank' },
              'https://github.com/elastos/Elastos'
            )
          ),
          _react2.default.createElement(
            'div',
            { style: { backgroundColor: '#45427e', padding: '30px' } },
            _react2.default.createElement('img', { src: _bucket_blockchain_info2.default })
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement(_TitleWithLine2.default, { label: 'Trustable Sidechains', titleSize: '24px', titleColor: 'blue', titleWeight: '300' }),
          _react2.default.createElement(
            'p',
            null,
            'Elastos gets its hashpower from Bitcoin and some of the largest mining pools in the world including Bitmain, this secures the ELA token which is also used for DPoS node voting. By distributing load across multiple sidechains and replacing expensive PoW consensus in major blockchains like Ethereum with DPoS, Elastos is able to achieve far greater performance while maintaining a secure trustless environment.'
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          _react2.default.createElement(_TitleWithLine2.default, { label: 'Key Features Include', titleSize: '24px', titleColor: 'blue', titleWeight: '300' }),
          _react2.default.createElement(
            DevFeatures,
            null,
            _react2.default.createElement(
              'li',
              null,
              'Intuitive APIs, a 500-1500+ TPS Ethereum sidechain and a wide variety of useful services.'
            ),
            _react2.default.createElement(
              'li',
              null,
              'Cyber Republic, one of the first decentralized community governance models to trial on-chain governance, control over allocation of funds and council elections open to everybody.'
            ),
            _react2.default.createElement(
              'li',
              null,
              'Developers have a say in how the platform evolves, so even if there is some lock-in to Elastos there is a pathway to adapt it to best suit the community'
            ),
            _react2.default.createElement(
              'li',
              null,
              'Funding for startups, projects and frameworks through the Cyber Republic Consensus (CRC).',
              _react2.default.createElement('br', null),
              _react2.default.createElement('br', null),
              'Find out more by visiting us at ',
              _react2.default.createElement(
                'a',
                { href: 'https://www.cyberrepublic.org', target: '_blank' },
                'https://www.cyberrepublic.org'
              )
            )
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          _react2.default.createElement(_TitleWithLine2.default, { label: 'BETA Launching - Mid-April 2019', titleSize: '24px', titleColor: 'blue', titleWeight: '300' }),
          _react2.default.createElement('br', null),
          _react2.default.createElement(
            'p',
            null,
            'For now you can try our ALPHA at ',
            _react2.default.createElement(
              'a',
              { href: 'https://github.com/cyber-republic/elastos-privnet', target: '_blank' },
              'https://github.com/cyber-republic/elastos-privnet'
            )
          ),
          _react2.default.createElement(
            'p',
            null,
            'With our private net you can run Elastos, IPFS and the DID sidechain locally, however it is missing the new ETH Sidechain which will be included in the BETA release. At that point you will have the minimal set of services to develop your dApps.'
          ),
          _react2.default.createElement('br', null),
          _react2.default.createElement('br', null),
          _react2.default.createElement(_TitleWithLine2.default, { label: 'SIGN UP for BETA Early Access', titleSize: '24px', titleColor: 'blue', titleWeight: '300' }),
          _react2.default.createElement('br', null),
          _react2.default.createElement(
            'form',
            { id: 'join-form', className: 'global-form', name: 'mailing-list', action: 'https://elastos.us17.list-manage.com/subscribe/post-json?u=8d74b221b8912cf1478a69f37&id=456ecdde13', method: 'get' },
            _react2.default.createElement(
              'div',
              { className: 'radio-btns', style: { paddingLeft: '10px' } },
              _react2.default.createElement(
                'fieldset',
                null,
                _react2.default.createElement('input', { type: 'checkbox', name: 'NEWSLETTER', id: 'mcf-NEWSLETTER', value: 'yes' }),
                _react2.default.createElement(
                  'label',
                  { htmlFor: 'mcf-NEWSLETTER' },
                  _react2.default.createElement('span', null),
                  'Opt In to exclusive newsletter'
                )
              ),
              _react2.default.createElement(
                'fieldset',
                null,
                _react2.default.createElement('input', { type: 'checkbox', name: 'DEVELOPER', id: 'mcf-DEVELOPER', value: 'yes' }),
                _react2.default.createElement(
                  'label',
                  { htmlFor: 'mcf-DEVELOPER' },
                  _react2.default.createElement('span', null),
                  'I am a developer'
                )
              )
            ),
            _react2.default.createElement(
              'fieldset',
              { className: 'email-wrap' },
              _react2.default.createElement('input', { type: 'email', name: 'EMAIL', 'data-type': 'req', placeholder: 'EMAIL' }),
              _react2.default.createElement(
                'button',
                { type: 'submit', className: 'arrow-submit' },
                _react2.default.createElement('span', { className: 'icon icon-chevron-right' })
              )
            ),
            _react2.default.createElement(
              'div',
              { className: 'form-msg' },
              _react2.default.createElement(
                'div',
                { className: 'thanks' },
                'Thank you! We will be in contact shortly.'
              )
            )
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var BgRight = _styledComponents2.default.img(_templateObject2, _variable.breakPoint.mobile);
var Inner = _styledComponents2.default.div(_templateObject3, _variable.breakPoint.mobile);
var Title = _styledComponents2.default.div(_templateObject4, _color.text.navy, _variable.breakPoint.mobile);
var DevFeatures = _styledComponents2.default.ul(_templateObject5);

/***/ }),
/* 41 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

exports.default = function () {
    return _react2.default.createElement(
        'div',
        null,
        _react2.default.createElement(
            'h1',
            null,
            '404 - Oh no\'s! We couldn\'t find that page :('
        )
    );
};

/***/ }),
/* 42 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _reactDom = __webpack_require__(43);

var _reactDom2 = _interopRequireDefault(_reactDom);

var _App = __webpack_require__(44);

var _App2 = _interopRequireDefault(_App);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

// Export your top level component as JSX (for static rendering)
exports.default = _App2.default;
// Render your app

// Your top level component

if (typeof document !== 'undefined') {
    var renderMethod =  false ? _reactDom2.default.render : _reactDom2.default.hydrate || _reactDom2.default.render;
    var render = function render(Comp) {
        renderMethod(_react2.default.createElement(Comp, null), document.getElementById('root'));
    };
    // Render!
    render(_App2.default);
}

/***/ }),
/* 43 */
/***/ (function(module, exports) {

module.exports = require("react-dom");

/***/ }),
/* 44 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _reactStatic = __webpack_require__(10);

var _reactStaticRoutes = __webpack_require__(45);

var _reactStaticRoutes2 = _interopRequireDefault(_reactStaticRoutes);

var _header = __webpack_require__(143);

var _header2 = _interopRequireDefault(_header);

var _footer = __webpack_require__(152);

var _footer2 = _interopRequireDefault(_footer);

var _routerUtils = __webpack_require__(157);

var _routerUtils2 = _interopRequireDefault(_routerUtils);

__webpack_require__(159);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

// for local
// import '../fonts/nfh2hmc.css'

// import { Link } from '@reach/router'
function App() {
  return _react2.default.createElement(
    'div',
    null,
    _react2.default.createElement(_header2.default, null),
    _react2.default.createElement(
      _reactStatic.Router,
      { history: _routerUtils2.default },
      _react2.default.createElement(_reactStaticRoutes2.default, null)
    ),
    _react2.default.createElement(_footer2.default, null)
  );
}
exports.default = App;

/***/ }),
/* 45 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/* WEBPACK VAR INJECTION */(function(__dirname) {

Object.defineProperty(exports, "__esModule", {
    value: true
});

var _path2 = __webpack_require__(46);

var _path3 = _interopRequireDefault(_path2);

var _importCss2 = __webpack_require__(47);

var _importCss3 = _interopRequireDefault(_importCss2);

var _universalImport2 = __webpack_require__(48);

var _universalImport3 = _interopRequireDefault(_universalImport2);

var _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; };

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _reactRouterDom = __webpack_require__(49);

var _reactUniversalComponent = __webpack_require__(50);

var _reactUniversalComponent2 = _interopRequireDefault(_reactUniversalComponent);

var _reactStatic = __webpack_require__(10);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

(0, _reactUniversalComponent.setHasBabelPlugin)();
var universalOptions = {
    loading: function loading() {
        return null;
    },
    error: function error(props) {
        console.error(props.error);
        return _react2.default.createElement(
            'div',
            null,
            'An error occurred loading this page\'s template. More information is available in the console.'
        );
    }
};
var t_0 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/home',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 23)), (0, _importCss3.default)('src/pages/home', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/home');
    },
    resolve: function resolve() {
        return /*require.resolve*/(23);
    },
    chunkName: function chunkName() {
        return 'src/pages/home';
    }
}), universalOptions);
var t_1 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/team',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 37)), (0, _importCss3.default)('src/pages/team', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/team');
    },
    resolve: function resolve() {
        return /*require.resolve*/(37);
    },
    chunkName: function chunkName() {
        return 'src/pages/team';
    }
}), universalOptions);
var t_2 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/roadmap',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 38)), (0, _importCss3.default)('src/pages/roadmap', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/roadmap');
    },
    resolve: function resolve() {
        return /*require.resolve*/(38);
    },
    chunkName: function chunkName() {
        return 'src/pages/roadmap';
    }
}), universalOptions);
var t_3 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/wallet',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 39)), (0, _importCss3.default)('src/pages/wallet', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/wallet');
    },
    resolve: function resolve() {
        return /*require.resolve*/(39);
    },
    chunkName: function chunkName() {
        return 'src/pages/wallet';
    }
}), universalOptions);
var t_4 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/developer',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 40)), (0, _importCss3.default)('src/pages/developer', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/developer');
    },
    resolve: function resolve() {
        return /*require.resolve*/(40);
    },
    chunkName: function chunkName() {
        return 'src/pages/developer';
    }
}), universalOptions);
var t_5 = (0, _reactUniversalComponent2.default)((0, _universalImport3.default)({
    id: '../src/pages/404',
    file: '/Users/clarencel/workspace/Elastos.ORG.Homepage/elastos-new/dist/react-static-routes.js',
    load: function load() {
        return Promise.all([new Promise(function(resolve) { resolve(); }).then(__webpack_require__.bind(null, 41)), (0, _importCss3.default)('src/pages/404', {
            disableWarnings: true
        })]).then(function (proms) {
            return proms[0];
        });
    },
    path: function path() {
        return _path3.default.join(__dirname, '../src/pages/404');
    },
    resolve: function resolve() {
        return /*require.resolve*/(41);
    },
    chunkName: function chunkName() {
        return 'src/pages/404';
    }
}), universalOptions);
// Template Map
global.componentsByTemplateID = global.componentsByTemplateID || [t_0, t_1, t_2, t_3, t_4, t_5];
// Template Tree
global.templateIDsByPath = global.templateIDsByPath || {
    '404': 5
};
// Get template for given path
var getComponentForPath = function getComponentForPath(path) {
    path = (0, _reactStatic.cleanPath)(path);
    return global.componentsByTemplateID[global.templateIDsByPath[path]];
};
global.reactStaticGetComponentForPath = getComponentForPath;
global.reactStaticRegisterTemplateIDForPath = function (path, id) {
    global.templateIDsByPath[path] = id;
};

var Routes = function (_Component) {
    _inherits(Routes, _Component);

    function Routes() {
        _classCallCheck(this, Routes);

        return _possibleConstructorReturn(this, (Routes.__proto__ || Object.getPrototypeOf(Routes)).apply(this, arguments));
    }

    _createClass(Routes, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                Comp = _props.component,
                render = _props.render,
                children = _props.children;

            var getFullComponentForPath = function getFullComponentForPath(path) {
                var Comp = getComponentForPath(path);
                var is404 = path === '404';
                if (!Comp) {
                    is404 = true;
                    Comp = getComponentForPath('404');
                }
                return function (newProps) {
                    return Comp ? _react2.default.createElement(Comp, _extends({}, newProps, is404 ? { is404: true } : {})) : null;
                };
            };
            var renderProps = {
                componentsByTemplateID: global.componentsByTemplateID,
                templateIDsByPath: global.templateIDsByPath,
                getComponentForPath: getFullComponentForPath
            };
            if (Comp) {
                return _react2.default.createElement(Comp, renderProps);
            }
            if (render || children) {
                return (render || children)(renderProps);
            }
            // This is the default auto-routing renderer
            return _react2.default.createElement(_reactRouterDom.Route, { path: '*', render: function render(props) {
                    var Comp = getFullComponentForPath(props.location.pathname);
                    // If Comp is used as a component here, it triggers React to re-mount the entire
                    // component tree underneath during reconciliation, losing all internal state.
                    // By unwrapping it here we keep the real, static component exposed directly to React.
                    return Comp && Comp(_extends({}, props, { key: props.location.pathname }));
                } });
        }
    }]);

    return Routes;
}(_react.Component);

exports.default = Routes;
/* WEBPACK VAR INJECTION */}.call(exports, "/"))

/***/ }),
/* 46 */
/***/ (function(module, exports) {

module.exports = require("path");

/***/ }),
/* 47 */
/***/ (function(module, exports) {

module.exports = require("babel-plugin-universal-import/importCss");

/***/ }),
/* 48 */
/***/ (function(module, exports) {

module.exports = require("babel-plugin-universal-import/universalImport");

/***/ }),
/* 49 */
/***/ (function(module, exports) {

module.exports = require("react-router-dom");

/***/ }),
/* 50 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/* WEBPACK VAR INJECTION */(function(module) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = exports.setHasBabelPlugin = exports.ReportChunks = exports.MODULE_IDS = exports.CHUNK_NAMES = undefined;

var _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; };

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _requireUniversalModule = __webpack_require__(51);

Object.defineProperty(exports, 'CHUNK_NAMES', {
  enumerable: true,
  get: function get() {
    return _requireUniversalModule.CHUNK_NAMES;
  }
});
Object.defineProperty(exports, 'MODULE_IDS', {
  enumerable: true,
  get: function get() {
    return _requireUniversalModule.MODULE_IDS;
  }
});

var _reportChunks = __webpack_require__(52);

Object.defineProperty(exports, 'ReportChunks', {
  enumerable: true,
  get: function get() {
    return _interopRequireDefault(_reportChunks).default;
  }
});

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _propTypes = __webpack_require__(22);

var _propTypes2 = _interopRequireDefault(_propTypes);

var _hoistNonReactStatics = __webpack_require__(53);

var _hoistNonReactStatics2 = _interopRequireDefault(_hoistNonReactStatics);

var _requireUniversalModule2 = _interopRequireDefault(_requireUniversalModule);

var _utils = __webpack_require__(21);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

function _objectWithoutProperties(obj, keys) { var target = {}; for (var i in obj) { if (keys.indexOf(i) >= 0) continue; if (!Object.prototype.hasOwnProperty.call(obj, i)) continue; target[i] = obj[i]; } return target; }

var hasBabelPlugin = false;

var isHMR = function isHMR() {
  return (
    // $FlowIgnore
    module.hot && (module.hot.data || module.hot.status() === 'apply')
  );
};

var setHasBabelPlugin = exports.setHasBabelPlugin = function setHasBabelPlugin() {
  hasBabelPlugin = true;
};

function universal(component) {
  var _class, _temp;

  var opts = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

  var _opts$loading = opts.loading,
      Loading = _opts$loading === undefined ? _utils.DefaultLoading : _opts$loading,
      _opts$error = opts.error,
      Err = _opts$error === undefined ? _utils.DefaultError : _opts$error,
      _opts$minDelay = opts.minDelay,
      minDelay = _opts$minDelay === undefined ? 0 : _opts$minDelay,
      _opts$alwaysDelay = opts.alwaysDelay,
      alwaysDelay = _opts$alwaysDelay === undefined ? false : _opts$alwaysDelay,
      _opts$testBabelPlugin = opts.testBabelPlugin,
      testBabelPlugin = _opts$testBabelPlugin === undefined ? false : _opts$testBabelPlugin,
      _opts$loadingTransiti = opts.loadingTransition,
      loadingTransition = _opts$loadingTransiti === undefined ? true : _opts$loadingTransiti,
      options = _objectWithoutProperties(opts, ['loading', 'error', 'minDelay', 'alwaysDelay', 'testBabelPlugin', 'loadingTransition']);

  var isDynamic = hasBabelPlugin || testBabelPlugin;
  options.isDynamic = isDynamic;
  options.modCache = {};
  options.promCache = {};

  return _temp = _class = function (_React$Component) {
    _inherits(UniversalComponent, _React$Component);

    _createClass(UniversalComponent, null, [{
      key: 'preload',

      /* eslint-enable react/sort-comp */

      /* eslint-disable react/sort-comp */
      value: function preload(props) {
        var context = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

        props = props || {};

        var _req = (0, _requireUniversalModule2.default)(component, options, props),
            requireAsync = _req.requireAsync,
            requireSync = _req.requireSync;

        var Component = void 0;

        try {
          Component = requireSync(props, context);
        } catch (error) {
          return Promise.reject(error);
        }

        if (Component) return Promise.resolve(Component);

        return requireAsync(props, context);
      }
    }]);

    function UniversalComponent(props, context) {
      _classCallCheck(this, UniversalComponent);

      var _this = _possibleConstructorReturn(this, (UniversalComponent.__proto__ || Object.getPrototypeOf(UniversalComponent)).call(this, props, context));

      _this.update = function (state) {
        var isMount = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : false;
        var isSync = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : false;
        var isServer = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : false;

        if (!_this._mounted) return;
        if (!state.error) state.error = null;

        _this.handleAfter(state, isMount, isSync, isServer);
      };

      _this.state = { error: null };
      return _this;
    }

    _createClass(UniversalComponent, [{
      key: 'componentWillMount',
      value: function componentWillMount() {
        this._mounted = true;

        var _req2 = (0, _requireUniversalModule2.default)(component, options, this.props),
            addModule = _req2.addModule,
            requireSync = _req2.requireSync,
            requireAsync = _req2.requireAsync,
            asyncOnly = _req2.asyncOnly;

        var Component = void 0;

        try {
          Component = requireSync(this.props, this.context);
        } catch (error) {
          return this.update({ error: error });
        }

        this._asyncOnly = asyncOnly;
        var chunkName = addModule(this.props); // record the module for SSR flushing :)

        if (this.context.report) {
          this.context.report(chunkName);
        }

        if (Component || _utils.isServer) {
          this.handleBefore(true, true, _utils.isServer);
          this.update({ Component: Component }, true, true, _utils.isServer);
          return;
        }

        this.handleBefore(true, false);
        this.requireAsync(requireAsync, this.props, true);
      }
    }, {
      key: 'componentWillUnmount',
      value: function componentWillUnmount() {
        this._mounted = false;
      }
    }, {
      key: 'componentWillReceiveProps',
      value: function componentWillReceiveProps(nextProps) {
        var _this2 = this;

        if (isDynamic || this._asyncOnly) {
          var _req3 = (0, _requireUniversalModule2.default)(component, options, nextProps, this.props),
              requireSync = _req3.requireSync,
              requireAsync = _req3.requireAsync,
              shouldUpdate = _req3.shouldUpdate;

          if (shouldUpdate(nextProps, this.props)) {
            var Component = void 0;

            try {
              Component = requireSync(nextProps, this.context);
            } catch (error) {
              return this.update({ error: error });
            }

            this.handleBefore(false, !!Component);

            if (!Component) {
              return this.requireAsync(requireAsync, nextProps);
            }

            var state = { Component: Component };

            if (alwaysDelay) {
              if (loadingTransition) this.update({ Component: null }); // display `loading` during componentWillReceiveProps
              setTimeout(function () {
                return _this2.update(state, false, true);
              }, minDelay);
              return;
            }

            this.update(state, false, true);
          } else if (isHMR()) {
            var _Component = requireSync(nextProps, this.context);
            this.setState({ Component: function Component() {
                return null;
              } }); // HMR /w Redux and HOCs can be finicky, so we
            setTimeout(function () {
              return _this2.setState({ Component: _Component });
            }); // toggle components to insure updates occur
          }
        }
      }
    }, {
      key: 'requireAsync',
      value: function requireAsync(_requireAsync, props, isMount) {
        var _this3 = this;

        if (this.state.Component && loadingTransition) {
          this.update({ Component: null }); // display `loading` during componentWillReceiveProps
        }

        var time = new Date();

        _requireAsync(props, this.context).then(function (Component) {
          var state = { Component: Component };

          var timeLapsed = new Date() - time;
          if (timeLapsed < minDelay) {
            var extraDelay = minDelay - timeLapsed;
            return setTimeout(function () {
              return _this3.update(state, isMount);
            }, extraDelay);
          }

          _this3.update(state, isMount);
        }).catch(function (error) {
          return _this3.update({ error: error });
        });
      }
    }, {
      key: 'handleBefore',
      value: function handleBefore(isMount, isSync) {
        var isServer = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : false;

        if (this.props.onBefore) {
          var onBefore = this.props.onBefore;

          var info = { isMount: isMount, isSync: isSync, isServer: isServer };
          onBefore(info);
        }
      }
    }, {
      key: 'handleAfter',
      value: function handleAfter(state, isMount, isSync, isServer) {
        var Component = state.Component,
            error = state.error;


        if (Component && !error) {
          (0, _hoistNonReactStatics2.default)(UniversalComponent, Component, { preload: true });

          if (this.props.onAfter) {
            var onAfter = this.props.onAfter;

            var info = { isMount: isMount, isSync: isSync, isServer: isServer };
            onAfter(info, Component);
          }
        } else if (error && this.props.onError) {
          this.props.onError(error);
        }

        this.setState(state);
      }
    }, {
      key: 'render',
      value: function render() {
        var _state = this.state,
            error = _state.error,
            Component = _state.Component;

        var _props = this.props,
            isLoading = _props.isLoading,
            userError = _props.error,
            props = _objectWithoutProperties(_props, ['isLoading', 'error']);

        // user-provided props (e.g. for data-fetching loading):


        if (isLoading) {
          return (0, _utils.createElement)(Loading, props);
        } else if (userError) {
          return (0, _utils.createElement)(Err, _extends({}, props, { error: userError }));
        } else if (error) {
          return (0, _utils.createElement)(Err, _extends({}, props, { error: error }));
        } else if (Component) {
          // primary usage (for async import loading + errors):
          return (0, _utils.createElement)(Component, props);
        }

        return (0, _utils.createElement)(Loading, props);
      }
    }]);

    return UniversalComponent;
  }(_react2.default.Component), _class.contextTypes = {
    store: _propTypes2.default.object,
    report: _propTypes2.default.func
  }, _temp;
}
exports.default = universal;
/* WEBPACK VAR INJECTION */}.call(exports, __webpack_require__(20)(module)))

/***/ }),
/* 51 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.clearChunks = exports.flushModuleIds = exports.flushChunkNames = exports.MODULE_IDS = exports.CHUNK_NAMES = undefined;
exports.default = requireUniversalModule;

var _utils = __webpack_require__(21);

var CHUNK_NAMES = exports.CHUNK_NAMES = new Set();
var MODULE_IDS = exports.MODULE_IDS = new Set();

function requireUniversalModule(universalConfig, options, props, prevProps) {
  var key = options.key,
      _options$timeout = options.timeout,
      timeout = _options$timeout === undefined ? 15000 : _options$timeout,
      onLoad = options.onLoad,
      onError = options.onError,
      isDynamic = options.isDynamic,
      modCache = options.modCache,
      promCache = options.promCache;


  var config = getConfig(isDynamic, universalConfig, options, props);
  var chunkName = config.chunkName,
      path = config.path,
      resolve = config.resolve,
      load = config.load;

  var asyncOnly = !path && !resolve || typeof chunkName === 'function';

  var requireSync = function requireSync(props, context) {
    var exp = (0, _utils.loadFromCache)(chunkName, props, modCache);

    if (!exp) {
      var mod = void 0;

      if (!(0, _utils.isWebpack)() && path) {
        var modulePath = (0, _utils.callForString)(path, props) || '';
        mod = (0, _utils.tryRequire)(modulePath);
      } else if ((0, _utils.isWebpack)() && resolve) {
        var weakId = (0, _utils.callForString)(resolve, props);

        if (__webpack_require__.m[weakId]) {
          mod = (0, _utils.tryRequire)(weakId);
        }
      }

      if (mod) {
        exp = (0, _utils.resolveExport)(mod, key, onLoad, chunkName, props, context, modCache, true);
      }
    }

    return exp;
  };

  var requireAsync = function requireAsync(props, context) {
    var exp = (0, _utils.loadFromCache)(chunkName, props, modCache);
    if (exp) return Promise.resolve(exp);

    var cachedPromise = (0, _utils.loadFromPromiseCache)(chunkName, props, promCache);
    if (cachedPromise) return cachedPromise;

    var prom = new Promise(function (res, rej) {
      var reject = function reject(error) {
        error = error || new Error('timeout exceeded');
        clearTimeout(timer);
        if (onError) {
          var _isServer = typeof window === 'undefined';
          var info = { isServer: _isServer };
          onError(error, info);
        }
        rej(error);
      };

      // const timer = timeout && setTimeout(reject, timeout)
      var timer = timeout && setTimeout(reject, timeout);

      var resolve = function resolve(mod) {
        clearTimeout(timer);

        var exp = (0, _utils.resolveExport)(mod, key, onLoad, chunkName, props, context, modCache);
        if (exp) return res(exp);

        reject(new Error('export not found'));
      };

      var request = load(props, { resolve: resolve, reject: reject });

      // if load doesn't return a promise, it must call resolveImport
      // itself. Most common is the promise implementation below.
      if (!request || typeof request.then !== 'function') return;
      request.then(resolve).catch(reject);
    });

    (0, _utils.cacheProm)(prom, chunkName, props, promCache);
    return prom;
  };

  var addModule = function addModule(props) {
    if (_utils.isServer || _utils.isTest) {
      if (chunkName) {
        var name = (0, _utils.callForString)(chunkName, props);
        if (name) CHUNK_NAMES.add(name);
        if (!_utils.isTest) return name; // makes tests way smaller to run both kinds
      }

      if ((0, _utils.isWebpack)()) {
        var weakId = (0, _utils.callForString)(resolve, props);
        if (weakId) MODULE_IDS.add(weakId);
        return weakId;
      }

      if (!(0, _utils.isWebpack)()) {
        var modulePath = (0, _utils.callForString)(path, props);
        if (modulePath) MODULE_IDS.add(modulePath);
        return modulePath;
      }
    }
  };

  var shouldUpdate = function shouldUpdate(next, prev) {
    var cacheKey = (0, _utils.callForString)(chunkName, next);

    var config = getConfig(isDynamic, universalConfig, options, prev);
    var prevCacheKey = (0, _utils.callForString)(config.chunkName, prev);

    return cacheKey !== prevCacheKey;
  };

  return {
    requireSync: requireSync,
    requireAsync: requireAsync,
    addModule: addModule,
    shouldUpdate: shouldUpdate,
    asyncOnly: asyncOnly
  };
}

var flushChunkNames = exports.flushChunkNames = function flushChunkNames() {
  var chunks = Array.from(CHUNK_NAMES);
  CHUNK_NAMES.clear();
  return chunks;
};

var flushModuleIds = exports.flushModuleIds = function flushModuleIds() {
  var ids = Array.from(MODULE_IDS);
  MODULE_IDS.clear();
  return ids;
};

var clearChunks = exports.clearChunks = function clearChunks() {
  CHUNK_NAMES.clear();
  MODULE_IDS.clear();
};

var getConfig = function getConfig(isDynamic, universalConfig, options, props) {
  if (isDynamic) {
    return typeof universalConfig === 'function' ? universalConfig(props) : universalConfig;
  }

  var load = typeof universalConfig === 'function' ? universalConfig : // $FlowIssue
  function () {
    return universalConfig;
  };

  return {
    file: 'default',
    id: options.id || 'default',
    chunkName: options.chunkName || 'default',
    resolve: options.resolve || '',
    path: options.path || '',
    load: load
  };
};

/***/ }),
/* 52 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _propTypes = __webpack_require__(22);

var _propTypes2 = _interopRequireDefault(_propTypes);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var ReportChunks = function (_React$Component) {
  _inherits(ReportChunks, _React$Component);

  function ReportChunks() {
    _classCallCheck(this, ReportChunks);

    return _possibleConstructorReturn(this, (ReportChunks.__proto__ || Object.getPrototypeOf(ReportChunks)).apply(this, arguments));
  }

  _createClass(ReportChunks, [{
    key: 'getChildContext',
    value: function getChildContext() {
      return {
        report: this.props.report
      };
    }
  }, {
    key: 'render',
    value: function render() {
      return _react2.default.Children.only(this.props.children);
    }
  }]);

  return ReportChunks;
}(_react2.default.Component);

ReportChunks.propTypes = {
  report: _propTypes2.default.func.isRequired
};
ReportChunks.childContextTypes = {
  report: _propTypes2.default.func.isRequired
};
exports.default = ReportChunks;

/***/ }),
/* 53 */
/***/ (function(module, exports) {

module.exports = require("hoist-non-react-statics");

/***/ }),
/* 54 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n  margin: 80px 0 100px;\n  background-color: white;\n  @media only screen and (max-width: ', ') {\n    overflow: hidden;\n  }\n  '], ['\n  position: relative;\n  margin: 80px 0 100px;\n  background-color: white;\n  @media only screen and (max-width: ', ') {\n    overflow: hidden;\n  }\n  ']),
    _templateObject2 = _taggedTemplateLiteral(['\n  font-size: 50px;\n  font-weight: 300;\n  @media only screen and (max-width: ', ') {\n    font-size: 42px;\n  }\n'], ['\n  font-size: 50px;\n  font-weight: 300;\n  @media only screen and (max-width: ', ') {\n    font-size: 42px;\n  }\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 0;\n  top: 80px;\n  width: 70px;\n  height: 70px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: -38px;\n    margin-top: 70px;\n    width: 48px;\n    height: 48px;\n  }\n'], ['\n  position: absolute;\n  left: 0;\n  top: 80px;\n  width: 70px;\n  height: 70px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: -38px;\n    margin-top: 70px;\n    width: 48px;\n    height: 48px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 70px;\n  top: 150px;\n  width: 70px;\n  height: 70px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: -60px;\n    margin-top: 48px;\n    width: 48px;\n    height: 48px;\n  }\n'], ['\n  position: absolute;\n  left: 70px;\n  top: 150px;\n  width: 70px;\n  height: 70px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: -60px;\n    margin-top: 48px;\n    width: 48px;\n    height: 48px;\n  }\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 0;\n  top: 350px;\n  width: 60px;\n  height: 60px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: 0;\n    width: 30px;\n    height: 30px;\n    top: 280px;\n  }\n'], ['\n  position: absolute;\n  left: 0;\n  top: 350px;\n  width: 60px;\n  height: 60px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-left: 0;\n    width: 30px;\n    height: 30px;\n    top: 280px;\n  }\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  position: absolute;\n  right: 0;\n  top: 80px;\n  width: 124px;\n  height: 248px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-top: 70px;\n    width: 54px;\n  }\n'], ['\n  position: absolute;\n  right: 0;\n  top: 80px;\n  width: 124px;\n  height: 248px;\n  background: url(', ') no-repeat;\n  background-size: contain;\n  @media only screen and (max-width: ', ') {\n    margin-top: 70px;\n    width: 54px;\n  }\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  display: flex;\n  justify-content: space-around;\n  width: 600px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    flex-direction: column;\n    width: auto;\n  }\n'], ['\n  display: flex;\n  justify-content: space-around;\n  width: 600px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    flex-direction: column;\n    width: auto;\n  }\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  text-decoration: underline;\n  color: #0F2D3B;\n  font-weight: lighter;\n  @media only screen and (max-width: ', ') {\n    margin: 8px 0;\n    font-size: 16px;\n  }\n'], ['\n  font-size: 12px;\n  text-decoration: underline;\n  color: #0F2D3B;\n  font-weight: lighter;\n  @media only screen and (max-width: ', ') {\n    margin: 8px 0;\n    font-size: 16px;\n  }\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  height: 70px;\n  margin: 30px auto;\n  &:hover {\n    cursor: pointer;\n  }\n'], ['\n  height: 70px;\n  margin: 30px auto;\n  &:hover {\n    cursor: pointer;\n  }\n']),
    _templateObject10 = _taggedTemplateLiteral(['\n  width: calc(100% - 300px);\n  max-width: 850px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n'], ['\n  width: calc(100% - 300px);\n  max-width: 850px;\n  margin: auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n']),
    _templateObject11 = _taggedTemplateLiteral(['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 180px;\n  width: 180px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n    width: 325px;\n    height: 325px;\n    background-position: 50% 50%;\n    background-size: 100%;\n  }\n\n  background: url(\n    ', '\n  ) no-repeat;\n  background-size: contain;\n'], ['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 180px;\n  width: 180px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n    width: 325px;\n    height: 325px;\n    background-position: 50% 50%;\n    background-size: 100%;\n  }\n\n  background: url(\n    ', '\n  ) no-repeat;\n  background-size: contain;\n']),
    _templateObject12 = _taggedTemplateLiteral(['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n'], ['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n']),
    _templateObject13 = _taggedTemplateLiteral(['\n  position: absolute;\n  top: 15px;\n  right: 10px;\n  width: 35px;\n  :hover {\n    cursor: pointer;\n  }\n'], ['\n  position: absolute;\n  top: 15px;\n  right: 10px;\n  width: 35px;\n  :hover {\n    cursor: pointer;\n  }\n']),
    _templateObject14 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n']),
    _templateObject15 = _taggedTemplateLiteral(['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n'], ['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n']);

__webpack_require__(6);

__webpack_require__(7);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _btn_play_video = __webpack_require__(55);

var _btn_play_video2 = _interopRequireDefault(_btn_play_video);

var _ecosystem_expand_btn = __webpack_require__(24);

var _ecosystem_expand_btn2 = _interopRequireDefault(_ecosystem_expand_btn);

var _BucketBlockchain = __webpack_require__(56);

var _BucketBlockchain2 = _interopRequireDefault(_BucketBlockchain);

var _BucketEth = __webpack_require__(57);

var _BucketEth2 = _interopRequireDefault(_BucketEth);

var _BucketRuntime = __webpack_require__(59);

var _BucketRuntime2 = _interopRequireDefault(_BucketRuntime);

var _BucketCarrier = __webpack_require__(61);

var _BucketCarrier2 = _interopRequireDefault(_BucketCarrier);

var _Video = __webpack_require__(63);

var _Video2 = _interopRequireDefault(_Video);

var _bg_shapes_left_purple = __webpack_require__(73);

var _bg_shapes_left_purple2 = _interopRequireDefault(_bg_shapes_left_purple);

var _bg_shapes_left_tan = __webpack_require__(74);

var _bg_shapes_left_tan2 = _interopRequireDefault(_bg_shapes_left_tan);

var _bg_shapes_left_green = __webpack_require__(75);

var _bg_shapes_left_green2 = _interopRequireDefault(_bg_shapes_left_green);

var _bg_shapes_right = __webpack_require__(76);

var _bg_shapes_right2 = _interopRequireDefault(_bg_shapes_right);

var _box_purp = __webpack_require__(77);

var _box_purp2 = _interopRequireDefault(_box_purp);

var _box_tan = __webpack_require__(78);

var _box_tan2 = _interopRequireDefault(_box_tan);

var _box_green = __webpack_require__(79);

var _box_green2 = _interopRequireDefault(_box_green);

var _box_blue = __webpack_require__(80);

var _box_blue2 = _interopRequireDefault(_box_blue);

var _variable = __webpack_require__(2);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    var _this = _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));

    _this.state = {
      showBucketBlockchain: false,
      showBucketEth: false,
      showBucketRuntime: false,
      showBucketCarrier: false,
      showVideo: false
    };
    _this.toggleBucketBlockchain = function () {
      var showBucketBlockchain = _this.state.showBucketBlockchain;

      _this.setState({ showBucketBlockchain: !showBucketBlockchain });
    };
    _this.toggleBucketEth = function () {
      var showBucketEth = _this.state.showBucketEth;

      _this.setState({ showBucketEth: !showBucketEth });
    };
    _this.toggleBucketRuntime = function () {
      var showBucketRuntime = _this.state.showBucketRuntime;

      _this.setState({ showBucketRuntime: !showBucketRuntime });
    };
    _this.toggleBucketCarrier = function () {
      var showBucketCarrier = _this.state.showBucketCarrier;

      _this.setState({ showBucketCarrier: !showBucketCarrier });
    };
    _this.toggleVideo = function () {
      var showVideo = _this.state.showVideo;

      _this.setState({ showVideo: !showVideo });
    };
    return _this;
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var _state = this.state,
          showBucketBlockchain = _state.showBucketBlockchain,
          showBucketEth = _state.showBucketEth,
          showBucketRuntime = _state.showBucketRuntime,
          showBucketCarrier = _state.showBucketCarrier,
          showVideo = _state.showVideo;

      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(
          Title,
          null,
          'Build on the Modern Internet'
        ),
        _react2.default.createElement(
          LinkGroup,
          null,
          _react2.default.createElement(
            LinkItem,
            { target: '_blank', href: 'https://news.elastos.org/wp-content/uploads/2019/01/Elastos-EOY-Report-2018.pdf' },
            '2018 END OF YEAR REPORT'
          ),
          _react2.default.createElement(
            LinkItem,
            { href: '/developer' },
            'DEVELOP ON ELASTOS'
          ),
          _react2.default.createElement(
            LinkItem,
            { target: '_blank', href: 'https://www.cyberrepublic.org' },
            'GET FUNDED BY CYBER REPUBLIC'
          ),
          _react2.default.createElement(
            LinkItem,
            { target: '_blank', href: 'https://wallet.elastos.org' },
            'WALLET'
          )
        ),
        _react2.default.createElement(PlayButton, { src: _btn_play_video2.default, onClick: this.toggleVideo }),
        _react2.default.createElement(
          ListContainer,
          null,
          _react2.default.createElement(
            _row2.default,
            { type: 'flex', justify: 'space-around', gutter: { xs: 8, lg: 24 } },
            _react2.default.createElement(
              Item,
              { xs: 24, md: 6, type: 'blockchain', onClick: this.toggleBucketBlockchain },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS ',
                  _react2.default.createElement('br', null),
                  'BLOCKCHAIN'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Hybrid AuxPoW/PoS ',
                  _react2.default.createElement('br', null),
                  'Consenses'
                )
              )
            ),
            _react2.default.createElement(
              Item,
              { xs: 24, md: 6, type: 'eth', onClick: this.toggleBucketEth },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS ETH ',
                  _react2.default.createElement('br', null),
                  'SIDECHAINS'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Solidity Smart Contracts ',
                  _react2.default.createElement('br', null),
                  'at 500-1500+ TPS'
                )
              )
            ),
            _react2.default.createElement(
              Item,
              { xs: 24, md: 6, type: 'runtime', onClick: this.toggleBucketRuntime },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS ',
                  _react2.default.createElement('br', null),
                  'RUNTIME'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Secure Platform for the ',
                  _react2.default.createElement('br', null),
                  'Modern Internet'
                )
              )
            ),
            _react2.default.createElement(
              Item,
              { xs: 24, md: 6, type: 'carrier', onClick: this.toggleBucketCarrier },
              _react2.default.createElement(
                ItemContent,
                null,
                _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                _react2.default.createElement(
                  ItemTitle,
                  null,
                  'ELASTOS ',
                  _react2.default.createElement('br', null),
                  'CARRIER'
                ),
                _react2.default.createElement(
                  ItemText,
                  null,
                  'Blockchain Powered ',
                  _react2.default.createElement('br', null),
                  'Secure P2P Network'
                )
              )
            )
          )
        ),
        _react2.default.createElement(BGTopLeft, null),
        _react2.default.createElement(BGMiddleLeft, null),
        _react2.default.createElement(BGBottomLeft, null),
        _react2.default.createElement(BGRight, null),
        _react2.default.createElement(_BucketBlockchain2.default, { visible: showBucketBlockchain, onClose: this.toggleBucketBlockchain }),
        _react2.default.createElement(_BucketEth2.default, { visible: showBucketEth, onClose: this.toggleBucketEth }),
        _react2.default.createElement(_BucketRuntime2.default, { visible: showBucketRuntime, onClose: this.toggleBucketRuntime }),
        _react2.default.createElement(_BucketCarrier2.default, { visible: showBucketCarrier, onClose: this.toggleBucketCarrier }),
        _react2.default.createElement(_Video2.default, { visible: showVideo, onClose: this.toggleVideo })
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject, _variable.breakPoint.mobile);
var Title = _styledComponents2.default.h1(_templateObject2, _variable.breakPoint.mobile);
var BGTopLeft = _styledComponents2.default.div(_templateObject3, _bg_shapes_left_purple2.default, _variable.breakPoint.mobile);
var BGMiddleLeft = _styledComponents2.default.div(_templateObject4, _bg_shapes_left_tan2.default, _variable.breakPoint.mobile);
var BGBottomLeft = _styledComponents2.default.div(_templateObject5, _bg_shapes_left_green2.default, _variable.breakPoint.mobile);
var BGRight = _styledComponents2.default.div(_templateObject6, _bg_shapes_right2.default, _variable.breakPoint.mobile);
var LinkGroup = _styledComponents2.default.div(_templateObject7, _variable.breakPoint.mobile);
var LinkItem = _styledComponents2.default.a(_templateObject8, _variable.breakPoint.mobile);
var PlayButton = _styledComponents2.default.img(_templateObject9);
var ListContainer = _styledComponents2.default.div(_templateObject10, _variable.breakPoint.mobile);
var Item = (0, _styledComponents2.default)(_col2.default)(_templateObject11, _variable.breakPoint.mobile, function (props) {
  return props.type === 'blockchain' && _box_purp2.default || props.type === 'eth' && _box_tan2.default || props.type === 'runtime' && _box_green2.default || props.type === 'carrier' && _box_blue2.default;
});
var ItemContent = _styledComponents2.default.div(_templateObject12);
var Icon = _styledComponents2.default.img(_templateObject13);
var ItemTitle = _styledComponents2.default.div(_templateObject14, _variable.breakPoint.mobile);
var ItemText = _styledComponents2.default.div(_templateObject15, _variable.breakPoint.mobile);

/***/ }),
/* 55 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 56 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(11);

var _Modal2 = _interopRequireDefault(_Modal);

var _bucket_blockchain_info = __webpack_require__(25);

var _bucket_blockchain_info2 = _interopRequireDefault(_bucket_blockchain_info);

var _btn_close_purp = __webpack_require__(8);

var _btn_close_purp2 = _interopRequireDefault(_btn_close_purp);

var _bucket_blockchain_bg = __webpack_require__(26);

var _bucket_blockchain_bg2 = _interopRequireDefault(_bucket_blockchain_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Blockchain';
var subtitle = 'Hybrid AuxPoW/PoS Consensus';
var textArr = [{
    title: 'Secured and Proven by PoW',
    text: 'Elastos merge mines its blocks with Bitcoin using AuxPoW, giving it the combined hashpower and security of the many participating mining pools, with no additional cost to the miner.'
}, {
    title: 'Accelerated by DPoS',
    text: 'By replacing expensive PoW consensus in major blockchains like Ethereum with DPoS, Elastos is able to achieve far greater performance while maintaining a secure trustless environment.'
}, {
    title: 'Flexible Sidechains',
    text: 'With the Elastos mainchain acting as a root of trust, sidechains can develop their own consensus models and developers can even create dedicated sidechains. When the sidechains are fully launched we will initially support both ETH and NEO Smart Contracts.'
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_purp2.default,
                bodyImg: _bucket_blockchain_info2.default,
                bgImg: _bucket_blockchain_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 57 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(11);

var _Modal2 = _interopRequireDefault(_Modal);

var _bucket_eth_info = __webpack_require__(58);

var _bucket_eth_info2 = _interopRequireDefault(_bucket_eth_info);

var _btn_close_tan = __webpack_require__(27);

var _btn_close_tan2 = _interopRequireDefault(_btn_close_tan);

var _bucket_eth_bg = __webpack_require__(28);

var _bucket_eth_bg2 = _interopRequireDefault(_bucket_eth_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Sidechains';
var subtitle = 'Solidity Smart Contracts at 500-1500+ TPS';
var textArr = [{
    title: 'Ethereum but with dPoS',
    text: 'A redifined EVM leverages the Elastos DPoS super nodes to replace the slower PoW consensus with blazing fast DPoS at 500-1500+ TPS before any sharding.'
}, {
    title: 'Solidity Smart Contracts',
    text: 'The Elastos EVM will be continually update to be fully backwards compatible with Solidity Smart Contracts.'
}, {
    title: 'Dedicated Sidechains',
    text: 'Allocate dedicated TPS with your own sidechain. Then take that sidechain and invent your own multi-chain or scaling solution'
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_tan2.default,
                bodyImg: _bucket_eth_info2.default,
                bgImg: _bucket_eth_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 58 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_eth_info.872cf3f7.png";

/***/ }),
/* 59 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(11);

var _Modal2 = _interopRequireDefault(_Modal);

var _bucket_runtime_info = __webpack_require__(60);

var _bucket_runtime_info2 = _interopRequireDefault(_bucket_runtime_info);

var _btn_close_green = __webpack_require__(17);

var _btn_close_green2 = _interopRequireDefault(_btn_close_green);

var _bucket_runtime_bg = __webpack_require__(29);

var _bucket_runtime_bg2 = _interopRequireDefault(_bucket_runtime_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Runtime';
var subtitle = 'Secure Platform for the Modern Internet';
var textArr = [{
    title: 'Phoase 0 - Decentralized API - NOW',
    text: 'Simple, easy-to-use APIs to port existing apps to the Elastos Ecosystem. A wide variety of services are continually being added by partners and the Cyber Republic initiative'
}, {
    title: 'Phase 1 - Elastos Trinity - 2019',
    text: ['An official framework built on Ionic, which is bundled with SDKs, plugins and a built-in app launcher for quicl dApp development.', 'Access to decentralized services, such as Elastos Hive IPFS mean you can create real dApps with minimal blockchain experience.']
}, {
    title: 'Phase 2 - Elastos Runtime - 2020',
    text: 'With code from development for over 18 years, a revolutionary runtime provides a high performance secure sandbox for dApps. Network traffic is restricted through the Elastos Carrier network only, thus achieving the goal of a true decentralized network that is resistant to attacks.'
}, {
    title: 'Phase 3 - Elastos TEE - 2020+',
    text: ['The trusted execution environment(TEE) further secures Elastos dApps from malicious actors and opens the doors to a myriad of potential applicatons.', 'A hardware root of trust ensures the entire stack is verified by the blockchain.']
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_green2.default,
                bodyImg: _bucket_runtime_info2.default,
                bgImg: _bucket_runtime_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 60 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/bucket_runtime_info.2a60de70.png";

/***/ }),
/* 61 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(11);

var _Modal2 = _interopRequireDefault(_Modal);

var _btn_close_blue = __webpack_require__(62);

var _btn_close_blue2 = _interopRequireDefault(_btn_close_blue);

var _bucket_carrier_bg = __webpack_require__(30);

var _bucket_carrier_bg2 = _interopRequireDefault(_bucket_carrier_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }
// import bodyImg from '../../../images/bucket_carrier_bg.png'


var title = 'Elastos Carrier';
var subtitle = 'Blockchain Powered Secure P2P Network';
var textArr = [{
    title: 'The Future of IoT',
    text: 'With Elastos Carrier end-to-end encryption comes standard. There are few reasons anyone but you should have access to your IoT devices, but in today\'s world companies will any excuse to harvest your data. Take back control with safe, secure networks of the Modern Internet.',
    bgcolor: '#397AA2'
}, {
    title: 'Own Your Own Data',
    text: 'All comunications on the Elastos Carrier netowrk is encrypted and authorized by blockchain IDs, decide who to share your data with and how to use it. Monetize your data or keep it private, it\'s your choice to make.',
    bgcolor: '#3A789E'
}, {
    title: 'Robust Relay Netowrk',
    text: 'The carrier network is designed to allow anyone to connect and without risk of data leaks. Demonstrated tunneling and relay functions allow even bypassing of certain firewalls.',
    bgcolor: '#2F6787'
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_blue2.default,
                bgImg: _bucket_carrier_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 62 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 63 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _modal = __webpack_require__(15);

var _modal2 = _interopRequireDefault(_modal);

var _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; };

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  width: 100%;\n  height: 100%;\n  position: fixed;\n  padding: 25px;\n  -webkit-transform: translateZ(0);\n  overflow-x: hidden;\n  overflow-y: scroll;\n  -webkit-overflow-scrolling: touch;\n  z-index: 30;\n'], ['\n  width: 100%;\n  height: 100%;\n  position: fixed;\n  padding: 25px;\n  -webkit-transform: translateZ(0);\n  overflow-x: hidden;\n  overflow-y: scroll;\n  -webkit-overflow-scrolling: touch;\n  z-index: 30;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: relative;\n  width: 100%;\n  /* max-width: 1280px; */\n  height: 100%;\n  margin: 0 auto;\n  display: flex;\n  justify-content: center;\n'], ['\n  position: relative;\n  width: 100%;\n  /* max-width: 1280px; */\n  height: 100%;\n  margin: 0 auto;\n  display: flex;\n  justify-content: center;\n']);

__webpack_require__(16);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _VideoPlayer = __webpack_require__(64);

var _VideoPlayer2 = _interopRequireDefault(_VideoPlayer);

var _Elastos = __webpack_require__(70);

var _Elastos2 = _interopRequireDefault(_Elastos);

var _Elastos3 = __webpack_require__(71);

var _Elastos4 = _interopRequireDefault(_Elastos3);

var _Elastos5 = __webpack_require__(72);

var _Elastos6 = _interopRequireDefault(_Elastos5);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var videoJsOptions = {
    autoplay: true,
    controls: true,
    sources: [{
        src: _Elastos2.default,
        type: 'video/mp4'
    }, {
        src: _Elastos4.default,
        type: 'video/ogg'
    }, {
        src: _Elastos6.default,
        type: 'video/webm'
    }]
};
var modalStyle = {
    position: 'fixed',
    top: 0,
    left: 0,
    width: '100%',
    height: '100%',
    backgroundColor: '#172d39',
    opacity: 1,
    zIndex: 1
};
var bodyStyle = {
    padding: 0
};

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                visible = _props.visible,
                onClose = _props.onClose;

            return _react2.default.createElement(
                _modal2.default,
                { visible: visible, onCancel: onClose, footer: null, width: '100%', closable: false, style: modalStyle, bodyStyle: bodyStyle },
                _react2.default.createElement(
                    Container,
                    null,
                    _react2.default.createElement(
                        Inner,
                        null,
                        _react2.default.createElement(_VideoPlayer2.default, _extends({}, videoJsOptions, { onClose: onClose }))
                    )
                )
            );
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var Inner = _styledComponents2.default.div(_templateObject2);

/***/ }),
/* 64 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  position: relative;\n  width: 100%;\n  max-width: 1024px;\n  height: 100%;\n  display: flex;\n  flex-direction: column;\n  justify-content: center;\n'], ['\n  position: relative;\n  width: 100%;\n  max-width: 1024px;\n  height: 100%;\n  display: flex;\n  flex-direction: column;\n  justify-content: center;\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _video = __webpack_require__(65);

var _video2 = _interopRequireDefault(_video);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _TitleWithLine = __webpack_require__(9);

var _TitleWithLine2 = _interopRequireDefault(_TitleWithLine);

var _CloseBtn = __webpack_require__(67);

var _CloseBtn2 = _interopRequireDefault(_CloseBtn);

var _btn_close_purp = __webpack_require__(8);

var _btn_close_purp2 = _interopRequireDefault(_btn_close_purp);

__webpack_require__(68);

__webpack_require__(69);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var VideoPlayer = function (_React$Component) {
    _inherits(VideoPlayer, _React$Component);

    function VideoPlayer() {
        _classCallCheck(this, VideoPlayer);

        var _this = _possibleConstructorReturn(this, (VideoPlayer.__proto__ || Object.getPrototypeOf(VideoPlayer)).apply(this, arguments));

        _this.onClose = function () {
            _this.player.pause();
            _this.props.onClose();
        };
        return _this;
    }

    _createClass(VideoPlayer, [{
        key: 'componentDidMount',
        value: function componentDidMount() {
            // instantiate Video.js
            this.player = (0, _video2.default)(this.videoNode, this.props, function onPlayerReady() {
                console.log('onPlayerReady', this);
            });
        }
        // destroy player on unmount

    }, {
        key: 'componentWillUnmount',
        value: function componentWillUnmount() {
            if (this.player) {
                this.player.dispose();
            }
        }
        // wrap the player in a div with a `data-vjs-player` attribute
        // so videojs won't create additional wrapper in the DOM
        // see https://github.com/videojs/video.js/pull/3856

    }, {
        key: 'render',
        value: function render() {
            var _this2 = this;

            return _react2.default.createElement(
                Container,
                null,
                _react2.default.createElement(_TitleWithLine2.default, { label: 'WELCOME TO THE FUTURE', titleColor: 'white', style: { alignSelf: 'flex-start', marginBottom: 30 } }),
                _react2.default.createElement(_CloseBtn2.default, { src: _btn_close_purp2.default, onClick: this.onClose }),
                _react2.default.createElement(
                    'div',
                    { 'data-vjs-player': true, className: 'elastos-video-container', style: { position: 'relative', width: '100%', height: 'calc(100% * 1080/1920 + 6%)' } },
                    _react2.default.createElement('video', { ref: function ref(node) {
                            return _this2.videoNode = node;
                        }, className: 'video-js' })
                )
            );
        }
    }]);

    return VideoPlayer;
}(_react2.default.Component);

exports.default = VideoPlayer;

var Container = _styledComponents2.default.div(_templateObject);

/***/ }),
/* 65 */
/***/ (function(module, exports) {

module.exports = require("video.js");

/***/ }),
/* 66 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 67 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  width: 72px;\n  height: 72px;\n  transform: rotate(45deg);\n  position: absolute;\n  left: -36px;\n  top: 50%;\n  margin-top: -36px;\n  z-index: 3;\n  overflow: hidden;\n  background-color: #0e2d39;\n  box-shadow: 15px 14px 103px 0px rgba(47, 73, 100, 0.8);\n  cursor: pointer;\n  .close-btn-bg {\n    position: absolute;\n    left: -20%;\n    top: -20%;\n    transform: scaleX(-1) rotate(45deg);\n    width: 102px;\n    height: 102px;\n    > div {\n      width: 100%;\n      height: 100%;\n      position: absolute;\n      left: 0;\n      top: 0;\n      background-color: #2a3c57;\n      transform: translateY(-50%);\n      transition: transform .5s ease-out;\n    }\n  }\n  @media screen and (max-width: 1024px) {\n    transform: rotate(45deg) scale(.6);\n  }\n  @media screen and (max-width: 550px) {\n    transform: rotate(45deg) scale(0.4);\n  }\n  :hover > .close-btn-bg div {\n    transform: translateY(0%);\n  }\n  .close-line {\n    width: 23px;\n    height: 4px;\n    border-radius: 20px;\n    background-color: #fff;\n    position: absolute;\n    left: 24px;\n    top: 50%;\n    margin-top: -2px;\n    &.top {\n      transform: translate3d(0px, 0px, 0);\n    }\n    &.bot {\n      transform: rotate(90deg) translate3d(0px, 0px, 0);\n    }\n  }\n'], ['\n  width: 72px;\n  height: 72px;\n  transform: rotate(45deg);\n  position: absolute;\n  left: -36px;\n  top: 50%;\n  margin-top: -36px;\n  z-index: 3;\n  overflow: hidden;\n  background-color: #0e2d39;\n  box-shadow: 15px 14px 103px 0px rgba(47, 73, 100, 0.8);\n  cursor: pointer;\n  .close-btn-bg {\n    position: absolute;\n    left: -20%;\n    top: -20%;\n    transform: scaleX(-1) rotate(45deg);\n    width: 102px;\n    height: 102px;\n    > div {\n      width: 100%;\n      height: 100%;\n      position: absolute;\n      left: 0;\n      top: 0;\n      background-color: #2a3c57;\n      transform: translateY(-50%);\n      transition: transform .5s ease-out;\n    }\n  }\n  @media screen and (max-width: 1024px) {\n    transform: rotate(45deg) scale(.6);\n  }\n  @media screen and (max-width: 550px) {\n    transform: rotate(45deg) scale(0.4);\n  }\n  :hover > .close-btn-bg div {\n    transform: translateY(0%);\n  }\n  .close-line {\n    width: 23px;\n    height: 4px;\n    border-radius: 20px;\n    background-color: #fff;\n    position: absolute;\n    left: 24px;\n    top: 50%;\n    margin-top: -2px;\n    &.top {\n      transform: translate3d(0px, 0px, 0);\n    }\n    &.bot {\n      transform: rotate(90deg) translate3d(0px, 0px, 0);\n    }\n  }\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        { onClick: this.props.onClick },
        _react2.default.createElement(
          'div',
          { className: 'close-btn-bg' },
          _react2.default.createElement('div', null)
        ),
        _react2.default.createElement(
          'div',
          { className: 'close-icon' },
          _react2.default.createElement('div', { className: 'close-line top' }),
          _react2.default.createElement('div', { className: 'close-line bot' })
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);

/***/ }),
/* 68 */
/***/ (function(module, exports) {

module.exports = require("video.js/dist/video-js.css");

/***/ }),
/* 69 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 70 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/Elastos.8194c3bb.mp4";

/***/ }),
/* 71 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/Elastos.d96c2eda.ogv";

/***/ }),
/* 72 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/Elastos.ca1e6428.webm";

/***/ }),
/* 73 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAC8CAYAAADCScSrAAAEzUlEQVR4nO3aO44dVRCA4eIRErEGR5YI2AQxEdtiSwQmIAEkCxNgJIQsPGCLQUjIM0bG6Brb+DGPvvf2o6rO962gSucPuk/3O59+8vnTgAF8/8dX37zroBnBLvaTRz9/LHjaexH7bk/B09qrsYfg6ezN2EPwdHVR7CF4Oros9hA83VwVewieTq6LPQRPF1NiD8HTwdTYd9534lT23emXt387+2VS7CF4Krt9euvOw7P7H+2zgkcaSnoe+819Zxc85Rwaewieao6JPQRPJcfGHoKnijliD8FTwVyxh+DJbs7YQ/BkNnfsIXiyWiL2EDwZLRV7CJ5slow9BE8mS8cegieLNWIPwZPBWrGH4NnamrGH4NnS2rGH4NnKFrGH4NnCVrGH4FnblrGH4FnT1rGH4FlLhthD8KwhS+wheJaWKfYQPEvKFnsInqVkjD0EzxKyxh6CZ26ZYw/BM6fssYfgmUuF2EPwzKFK7CF4jlUp9hA8x6gWewieQ1WMPQTPIarGHoJnX5VjD8Gzj+qxh+CZqkPsIXim6BJ7CJ7rdIo9BM9VusUegucyHWOPzsGfP3mUYIqafvjz67sdY4/OwZ8+/nV3cAkmqeefp09udN2t9SPNvb9+FD2vaf8ML3peNcRLq+h5YZhbGtETo11Lip7h7uFFP7YhPzyJflzDfmkV/ZiG/rVA9OMZ/l8a0Y9l+OBD9EMR/HOiH4PgXyH6/gT/BtH3JvgLiL4vwV9C9D0J/gqi70fw1xB9L4KfQPR9CH4i0fcg+D2Ivj7B70n0tQn+AKKvS/AHEn1Ngj+C6OsR/JFEX4vgZyD6OgQ/E9HXIPgZiT4/wc9M9LkJfgGiz0vwCxF9ToJfkOjzEfzCRJ+L4Fcg+jwEvxLR5yD4FYl+e4Jfmei3JfgN7KL/9vcvhts7A8Fv5PT8geg3IPgNiX59gt+Y6Ncl+AREvx7BJyH6dQg+EdEvT/DJiH5Zgk9I9MsRfFKiX4bgExP9/ASfnOjnJfgCRD8fwRch+nkIvhDRH0/wxYj+OIIvSPSHE3xRoj+M4AsT/f4EX5zo9yP4BkQ/neCbEP00gm9E9NcTfDOiv5rgGxL95QTflOgvJvjGRP82wTcn+tcJfgCi/5/gByH6/wh+IKIX/HBGj17wAxo5esEPatToBT+wEaMX/OBGi17wDBW94HlmlOgFz0sjRN82+JNHP91NMEY5u+gfnp+03a9l8LdPb905PX9wI8EoJT1+ctZ2t3bB72J/eHb/ZoJRSKhV8GLnOm2CFztTtAhe7ExVPnixs4/SwYudfZUNXuwcomTwYudQ5YIXO8coFbzYOVaZ4MXOHEoEL3bmkj54sTOn1MGLnbmlDV7sLCFl8GJnKemCFztLShW82FlamuDFzhpSBC921rJ58GJnTZsGL3bWtlnwYmcLmwQvdrayevBiZ0urBi92trZa8GIng1WCFztZLB682Mlk0eDFTjaLBS92MlokeLGT1ezBi53MZg1e7GQ3W/Bip4JZghc7VRwdvNip5KjgxU41Bwcvdio6KHixU9XewYudyvYKXuxUNzl4sdPBpODFThfXBi92OrkyeLHTzaXBi52OLgxe7HT1VvBip7PXghc73b0MXuyM4FnwYmcU7/394QefiZ0hRMS/q1dggYUhphEAAAAASUVORK5CYII="

/***/ }),
/* 74 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 75 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 76 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 77 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/box_purp.beffcb0e.png";

/***/ }),
/* 78 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/box_tan.aeabeb81.png";

/***/ }),
/* 79 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/box_green.d5e9ef37.png";

/***/ }),
/* 80 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/box_blue.0f8403e1.png";

/***/ }),
/* 81 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _menu = __webpack_require__(32);

var _menu2 = _interopRequireDefault(_menu);

var _dropdown = __webpack_require__(33);

var _dropdown2 = _interopRequireDefault(_dropdown);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  margin: 0 auto;\n  background-size: cover;\n  height: 700px;\n \n  ', '\n  \n  -webkit-backface-visibility: hidden;\n  -moz-backface-visibility:    hidden;\n  -ms-backface-visibility:     hidden;\n  \n  @media only screen and (max-width: ', ') {\n    height: 100%;\n    \n    ', '\n  }\n'], ['\n  margin: 0 auto;\n  background-size: cover;\n  height: 700px;\n \n  ', '\n  \n  -webkit-backface-visibility: hidden;\n  -moz-backface-visibility:    hidden;\n  -ms-backface-visibility:     hidden;\n  \n  @media only screen and (max-width: ', ') {\n    height: 100%;\n    \n    ', '\n  }\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  overflow: visible;\n  \n  max-width: 950px;\n  height: 100%;\n  margin: 0 auto;\n  \n  @media only screen and (max-width: ', ') {\n    padding: 24px 24px 72px 24px;\n  }\n'], ['\n  overflow: visible;\n  \n  max-width: 950px;\n  height: 100%;\n  margin: 0 auto;\n  \n  @media only screen and (max-width: ', ') {\n    padding: 24px 24px 72px 24px;\n  }\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  background-color: white;\n  width: 225px;\n  height: 450px;\n  float: left;\n  text-align: left;\n  margin-top: 120px;\n  \n  @media only screen and (max-width: ', ') {\n    float: none;\n    width: 100%;\n    height: auto;\n    margin-top: 0;\n  }\n'], ['\n  background-color: white;\n  width: 225px;\n  height: 450px;\n  float: left;\n  text-align: left;\n  margin-top: 120px;\n  \n  @media only screen and (max-width: ', ') {\n    float: none;\n    width: 100%;\n    height: auto;\n    margin-top: 0;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  font-weight: 400;\n  font-size: 10px;\n\n  padding: 35px 0 0 30px;\n  \n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n    font-size: 14px;\n    font-weight: 300;\n  }\n'], ['\n  font-weight: 400;\n  font-size: 10px;\n\n  padding: 35px 0 0 30px;\n  \n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n    font-size: 14px;\n    font-weight: 300;\n  }\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  width: 100px;\n  height: 1px;\n  padding-left: 10px;\n  \n  @media only screen and (max-width: ', ') {\n    width: 200px;\n  }\n'], ['\n  width: 100px;\n  height: 1px;\n  padding-left: 10px;\n  \n  @media only screen and (max-width: ', ') {\n    width: 200px;\n  }\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  padding-top: 24px;\n  font-size: 36px;\n  padding-left: 30px;\n  \n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n    padding-top: 36px;\n    font-size: 48px;\n    font-weight: 300;\n  }\n'], ['\n  padding-top: 24px;\n  font-size: 36px;\n  padding-left: 30px;\n  \n  color: ', ';\n  \n  @media only screen and (max-width: ', ') {\n    padding-top: 36px;\n    font-size: 48px;\n    font-weight: 300;\n  }\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  overflow-y: visible;\n  overflow-x: hidden;\n  padding-top: 120px;\n  \n  @media only screen and (max-width: ', ') {\n    width: 100%;\n    padding-top: 24px;\n  }\n'], ['\n  overflow-y: visible;\n  overflow-x: hidden;\n  padding-top: 120px;\n  \n  @media only screen and (max-width: ', ') {\n    width: 100%;\n    padding-top: 24px;\n  }\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  width: 100%;\n  padding-left: 45px;\n  \n  @media only screen and (max-width: ', ') {\n    padding-left: 0;\n  }\n'], ['\n  width: 100%;\n  padding-left: 45px;\n  \n  @media only screen and (max-width: ', ') {\n    padding-left: 0;\n  }\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  margin-top: 30px;\n  padding-left: 0;\n  border-bottom: 1px solid #cfdae4;\n  \n  @media only screen and (max-width: ', ') {\n    margin-bottom: 0;\n    border-bottom: 0;\n  }\n'], ['\n  margin-top: 30px;\n  padding-left: 0;\n  border-bottom: 1px solid #cfdae4;\n  \n  @media only screen and (max-width: ', ') {\n    margin-bottom: 0;\n    border-bottom: 0;\n  }\n']),
    _templateObject10 = _taggedTemplateLiteral(['\n  border-top: 1px solid ', ';\n  \n  padding: 15px 30px;\n  cursor: pointer;\n\n  &:hover {\n    background-color: ', ';\n  }\n\n  &.selected {\n    color: ', ';\n    background-color: ', ';\n  }\n'], ['\n  border-top: 1px solid ', ';\n  \n  padding: 15px 30px;\n  cursor: pointer;\n\n  &:hover {\n    background-color: ', ';\n  }\n\n  &.selected {\n    color: ', ';\n    background-color: ', ';\n  }\n']),
    _templateObject11 = _taggedTemplateLiteral(['\n  padding: 20px 30px;\n  background-color: ', ';\n  \n  a {\n    color: ', '\n    font-weight: 300;\n    font-size: 18px;\n    width: 100%;\n    display: block;\n    \n    i {\n      float: right;\n      font-size: 200%;\n    }\n  }\n'], ['\n  padding: 20px 30px;\n  background-color: ', ';\n  \n  a {\n    color: ', '\n    font-weight: 300;\n    font-size: 18px;\n    width: 100%;\n    display: block;\n    \n    i {\n      float: right;\n      font-size: 200%;\n    }\n  }\n']),
    _templateObject12 = _taggedTemplateLiteral(['\n  float: right;\n  width: 20px;\n  padding-top: 10px;\n'], ['\n  float: right;\n  width: 20px;\n  padding-top: 10px;\n']);

__webpack_require__(34);

__webpack_require__(35);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

__webpack_require__(12);

__webpack_require__(82);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _images = __webpack_require__(83);

var _variable = __webpack_require__(2);

var _reactResponsive = __webpack_require__(19);

var _reactResponsive2 = _interopRequireDefault(_reactResponsive);

var _color = __webpack_require__(3);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var INDUSTRY_TYPE = {
  IDENTITY: 'IDENTITY',
  IOT: 'IOT',
  MEDIA: 'MEDIA',
  SECURITY: 'SECURITY',
  REAL: 'REAL'
};
var TYPE_TO_DISPLAY = {
  IDENTITY: 'Identity',
  IOT: 'IoT',
  MEDIA: 'Media',
  SECURITY: 'Security',
  REAL: 'Real Assets'
};

var Hero = function (_React$Component) {
  _inherits(Hero, _React$Component);

  function Hero() {
    _classCallCheck(this, Hero);

    var _this = _possibleConstructorReturn(this, (Hero.__proto__ || Object.getPrototypeOf(Hero)).call(this));

    _this.state = {
      industryType: INDUSTRY_TYPE.IDENTITY
    };
    return _this;
  }

  _createClass(Hero, [{
    key: 'render',
    value: function render() {
      var _this2 = this;

      // TODO: remove this className here, use the standard container/containerInner
      return _react2.default.createElement(
        Container,
        { className: 'container', industryType: this.getParentClass() },
        _react2.default.createElement(
          ContainerInner,
          { className: 'container-inner' },
          _react2.default.createElement(
            IndustryMenu,
            null,
            _react2.default.createElement(
              IndustryMenuTitle,
              null,
              'ELASTOS',
              _react2.default.createElement(IndustryMenuTitleLine, { src: _images.images.titleLineImg })
            ),
            _react2.default.createElement(
              IndustryMenuSubtitle,
              null,
              'Industries'
            ),
            _react2.default.createElement(
              _reactResponsive2.default,
              { minWidth: _variable.breakPoint.mobile },
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.identityBgImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.iotBgImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.mediaBgImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.securityBgImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.realBgImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.identityInfoImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.iotInfoImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.mediaInfoImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.securityInfoImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.realInfoImg, as: 'image' }),
              _react2.default.createElement(
                IndustryMenuListContainer,
                null,
                _react2.default.createElement(
                  IndustryMenuListItem,
                  { onClick: function onClick() {
                      return _this2.changeIndustry(INDUSTRY_TYPE.IDENTITY);
                    }, className: this.state.industryType === INDUSTRY_TYPE.IDENTITY ? 'selected' : undefined },
                  TYPE_TO_DISPLAY.IDENTITY
                ),
                _react2.default.createElement(
                  IndustryMenuListItem,
                  { onClick: function onClick() {
                      return _this2.changeIndustry(INDUSTRY_TYPE.IOT);
                    }, className: this.state.industryType === INDUSTRY_TYPE.IOT ? 'selected' : undefined },
                  TYPE_TO_DISPLAY.IOT
                ),
                _react2.default.createElement(
                  IndustryMenuListItem,
                  { onClick: function onClick() {
                      return _this2.changeIndustry(INDUSTRY_TYPE.MEDIA);
                    }, className: this.state.industryType === INDUSTRY_TYPE.MEDIA ? 'selected' : undefined },
                  TYPE_TO_DISPLAY.MEDIA
                ),
                _react2.default.createElement(
                  IndustryMenuListItem,
                  { onClick: function onClick() {
                      return _this2.changeIndustry(INDUSTRY_TYPE.SECURITY);
                    }, className: this.state.industryType === INDUSTRY_TYPE.SECURITY ? 'selected' : undefined },
                  TYPE_TO_DISPLAY.SECURITY
                ),
                _react2.default.createElement(
                  IndustryMenuListItem,
                  { onClick: function onClick() {
                      return _this2.changeIndustry(INDUSTRY_TYPE.REAL);
                    }, className: this.state.industryType === INDUSTRY_TYPE.REAL ? 'selected' : undefined },
                  TYPE_TO_DISPLAY.REAL
                )
              )
            ),
            _react2.default.createElement(
              _reactResponsive2.default,
              { maxWidth: _variable.breakPoint.mobile },
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.identityBgMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.iotBgMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.mediaBgMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.securityBgMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.realBgMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.identityInfoMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.iotInfoMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.mediaInfoMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.securityInfoMobImg, as: 'image' }),
              _react2.default.createElement('link', { rel: 'preload', href: _images.images.realInfoMobImg, as: 'image' }),
              _react2.default.createElement(
                IndustryMenuListContainer,
                null,
                _react2.default.createElement(
                  MobileIndustryDropdown,
                  null,
                  _react2.default.createElement(
                    _dropdown2.default,
                    { overlay: this.getMobileIndustryMenu() },
                    _react2.default.createElement(
                      'a',
                      { className: 'ant-dropdown-link' },
                      TYPE_TO_DISPLAY[this.state.industryType],
                      _react2.default.createElement(IconDown, { src: _images.images.iconDownImg })
                    )
                  )
                )
              )
            )
          ),
          _react2.default.createElement(
            IndustryBody,
            null,
            _react2.default.createElement(IndustryInfographic, { className: this.getParentClass(), src: this.getInfographic() })
          )
        )
      );
    }
  }, {
    key: 'getMobileIndustryMenu',
    value: function getMobileIndustryMenu() {
      var _this3 = this;

      return _react2.default.createElement(
        _menu2.default,
        null,
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return _this3.changeIndustry(INDUSTRY_TYPE.IDENTITY);
              } },
            'Identity'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return _this3.changeIndustry(INDUSTRY_TYPE.IOT);
              } },
            'IoT'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return _this3.changeIndustry(INDUSTRY_TYPE.MEDIA);
              } },
            'Media'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return _this3.changeIndustry(INDUSTRY_TYPE.SECURITY);
              } },
            'Security'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return _this3.changeIndustry(INDUSTRY_TYPE.REAL);
              } },
            'Real Assets'
          )
        )
      );
    }
  }, {
    key: 'getParentClass',
    value: function getParentClass() {
      switch (this.state.industryType) {
        case INDUSTRY_TYPE.IDENTITY:
          return 'identity';
        case INDUSTRY_TYPE.IOT:
          return 'iot';
        case INDUSTRY_TYPE.MEDIA:
          return 'media';
        case INDUSTRY_TYPE.SECURITY:
          return 'security';
        case INDUSTRY_TYPE.REAL:
          return 'real';
      }
    }
  }, {
    key: 'changeIndustry',
    value: function changeIndustry(industryType) {
      this.setState({
        industryType: industryType
      });
    }
  }, {
    key: 'getInfographic',
    value: function getInfographic() {
      if (typeof window !== 'undefined') {
        var isMobile = window.innerWidth < parseFloat(_variable.breakPoint.mobile);
        switch (this.state.industryType) {
          case INDUSTRY_TYPE.IDENTITY:
            return isMobile ? _images.images.identityInfoMobImg : _images.images.identityInfoImg;
          case INDUSTRY_TYPE.IOT:
            return isMobile ? _images.images.iotInfoMobImg : _images.images.iotInfoImg;
          case INDUSTRY_TYPE.MEDIA:
            return isMobile ? _images.images.mediaInfoMobImg : _images.images.mediaInfoImg;
          case INDUSTRY_TYPE.SECURITY:
            return isMobile ? _images.images.securityInfoMobImg : _images.images.securityInfoImg;
          case INDUSTRY_TYPE.REAL:
            return isMobile ? _images.images.realInfoMobImg : _images.images.realInfoImg;
        }
      } else {
        switch (this.state.industryType) {
          case INDUSTRY_TYPE.IDENTITY:
            return _images.images.identityInfoImg;
          case INDUSTRY_TYPE.IOT:
            return _images.images.iotInfoImg;
          case INDUSTRY_TYPE.MEDIA:
            return _images.images.mediaInfoImg;
          case INDUSTRY_TYPE.SECURITY:
            return _images.images.securityInfoImg;
          case INDUSTRY_TYPE.REAL:
            return realInfoImg;
        }
      }
    }
  }]);

  return Hero;
}(_react2.default.Component);

exports.default = Hero;

var Container = _styledComponents2.default.div(_templateObject, function (props) {
  switch (props.industryType) {
    case 'identity':
      return 'background-image: url(' + _images.images.identityBgImg + ');';
    case 'iot':
      return 'background-image: url(' + _images.images.iotBgImg + ');';
    case 'media':
      return 'background-image: url(' + _images.images.mediaBgImg + ');';
    case 'security':
      return 'background-image: url(' + _images.images.securityBgImg + ');';
    case 'real':
      return 'background-image: url(' + _images.images.realBgImg + ');';
  }
}, _variable.breakPoint.mobile, function (props) {
  switch (props.industryType) {
    case 'identity':
      return 'background-image: url(' + _images.images.identityBgMobImg + ');';
    case 'iot':
      return 'background-image: url(' + _images.images.iotBgMobImg + ');';
    case 'media':
      return 'background-image: url(' + _images.images.mediaBgMobImg + ');';
    case 'security':
      return 'background-image: url(' + _images.images.securityBgMobImg + ');';
    case 'real':
      return 'background-image: url(' + _images.images.realBgMobImg + ');';
  }
});
var ContainerInner = _styledComponents2.default.div(_templateObject2, _variable.breakPoint.mobile);
var IndustryMenu = _styledComponents2.default.div(_templateObject3, _variable.breakPoint.mobile);
var IndustryMenuTitle = _styledComponents2.default.div(_templateObject4, _color.text.blue, _variable.breakPoint.mobile);
var IndustryMenuTitleLine = _styledComponents2.default.img(_templateObject5, _variable.breakPoint.mobile);
var IndustryMenuSubtitle = _styledComponents2.default.span(_templateObject6, _color.text.darkBlue, _variable.breakPoint.mobile);
// TODO: 1s fade?
var IndustryBody = _styledComponents2.default.div(_templateObject7, _variable.breakPoint.mobile);
var IndustryInfographic = _styledComponents2.default.img(_templateObject8, _variable.breakPoint.mobile);
var IndustryMenuListContainer = _styledComponents2.default.ul(_templateObject9, _variable.breakPoint.mobile);
var IndustryMenuListItem = _styledComponents2.default.ul(_templateObject10, _color.border.light, _color.border.light, _color.text.white, _color.bg.selected);
var MobileIndustryDropdown = _styledComponents2.default.div(_templateObject11, _color.bg.selected, _color.text.white);
var IconDown = _styledComponents2.default.img(_templateObject12);

/***/ }),
/* 82 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 83 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});
exports.images = undefined;

var _ecosystem_title_line = __webpack_require__(18);

var _ecosystem_title_line2 = _interopRequireDefault(_ecosystem_title_line);

var _industry_identity_info = __webpack_require__(84);

var _industry_identity_info2 = _interopRequireDefault(_industry_identity_info);

var _industry_iot_info = __webpack_require__(85);

var _industry_iot_info2 = _interopRequireDefault(_industry_iot_info);

var _industry_media_info = __webpack_require__(86);

var _industry_media_info2 = _interopRequireDefault(_industry_media_info);

var _industry_security_info = __webpack_require__(87);

var _industry_security_info2 = _interopRequireDefault(_industry_security_info);

var _industry_real_info = __webpack_require__(88);

var _industry_real_info2 = _interopRequireDefault(_industry_real_info);

var _industry_identity_info3 = __webpack_require__(89);

var _industry_identity_info4 = _interopRequireDefault(_industry_identity_info3);

var _industry_iot_info3 = __webpack_require__(90);

var _industry_iot_info4 = _interopRequireDefault(_industry_iot_info3);

var _industry_media_info3 = __webpack_require__(91);

var _industry_media_info4 = _interopRequireDefault(_industry_media_info3);

var _industry_security_info3 = __webpack_require__(92);

var _industry_security_info4 = _interopRequireDefault(_industry_security_info3);

var _industry_real_info3 = __webpack_require__(93);

var _industry_real_info4 = _interopRequireDefault(_industry_real_info3);

var _industry_identity_bg = __webpack_require__(94);

var _industry_identity_bg2 = _interopRequireDefault(_industry_identity_bg);

var _industry_iot_bg = __webpack_require__(95);

var _industry_iot_bg2 = _interopRequireDefault(_industry_iot_bg);

var _industry_media_bg_ = __webpack_require__(96);

var _industry_media_bg_2 = _interopRequireDefault(_industry_media_bg_);

var _industry_security_bg = __webpack_require__(97);

var _industry_security_bg2 = _interopRequireDefault(_industry_security_bg);

var _industry_real_bg = __webpack_require__(98);

var _industry_real_bg2 = _interopRequireDefault(_industry_real_bg);

var _industry_identity_bg3 = __webpack_require__(99);

var _industry_identity_bg4 = _interopRequireDefault(_industry_identity_bg3);

var _industry_iot_bg3 = __webpack_require__(100);

var _industry_iot_bg4 = _interopRequireDefault(_industry_iot_bg3);

var _industry_media_bg = __webpack_require__(101);

var _industry_media_bg2 = _interopRequireDefault(_industry_media_bg);

var _industry_security_bg3 = __webpack_require__(102);

var _industry_security_bg4 = _interopRequireDefault(_industry_security_bg3);

var _industry_real_bg3 = __webpack_require__(103);

var _industry_real_bg4 = _interopRequireDefault(_industry_real_bg3);

var _industry_header_arrow_down = __webpack_require__(104);

var _industry_header_arrow_down2 = _interopRequireDefault(_industry_header_arrow_down);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var images = exports.images = {
    titleLineImg: _ecosystem_title_line2.default,
    identityInfoImg: _industry_identity_info2.default,
    iotInfoImg: _industry_iot_info2.default,
    mediaInfoImg: _industry_media_info2.default,
    securityInfoImg: _industry_security_info2.default,
    realInfoImg: _industry_real_info2.default,
    identityInfoMobImg: _industry_identity_info4.default,
    iotInfoMobImg: _industry_iot_info4.default,
    mediaInfoMobImg: _industry_media_info4.default,
    securityInfoMobImg: _industry_security_info4.default,
    realInfoMobImg: _industry_real_info4.default,
    identityBgImg: _industry_identity_bg2.default,
    iotBgImg: _industry_iot_bg2.default,
    mediaBgImg: _industry_media_bg_2.default,
    securityBgImg: _industry_security_bg2.default,
    realBgImg: _industry_real_bg2.default,
    identityBgMobImg: _industry_identity_bg4.default,
    iotBgMobImg: _industry_iot_bg4.default,
    mediaBgMobImg: _industry_media_bg2.default,
    securityBgMobImg: _industry_security_bg4.default,
    realBgMobImg: _industry_real_bg4.default,
    iconDownImg: _industry_header_arrow_down2.default
};

/***/ }),
/* 84 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_identity_info.93b2960c.png";

/***/ }),
/* 85 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_iot_info.b88d1c6a.png";

/***/ }),
/* 86 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_media_info.56f5499b.png";

/***/ }),
/* 87 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_security_info.79fa22fa.png";

/***/ }),
/* 88 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_real_info.2df24f89.png";

/***/ }),
/* 89 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_identity_info.d42295f9.png";

/***/ }),
/* 90 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_iot_info.08afc02b.png";

/***/ }),
/* 91 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_media_info.76ef28e5.png";

/***/ }),
/* 92 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_security_info.733ce9fe.png";

/***/ }),
/* 93 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_real_info.eae528f4.png";

/***/ }),
/* 94 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_identity_bg.5319b05c.png";

/***/ }),
/* 95 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_iot_bg.3b4d18f7.png";

/***/ }),
/* 96 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_media_bg_2.686f2937.png";

/***/ }),
/* 97 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_security_bg.973afd31.png";

/***/ }),
/* 98 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_real_bg.fb5f114e.png";

/***/ }),
/* 99 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_identity_bg.0b918b12.png";

/***/ }),
/* 100 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_iot_bg.e5e6d366.png";

/***/ }),
/* 101 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_media_bg.75e719e0.png";

/***/ }),
/* 102 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_security_bg.11fdbcf2.png";

/***/ }),
/* 103 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/industry_real_bg.d40fb939.png";

/***/ }),
/* 104 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB0AAAAPCAYAAAAYjcSfAAAAwklEQVQ4jb3NMQrCQBCF4QmChY2VaGGjeDhvZC4QSJ0mXbqAaU2b0nv8MjDbRJfdjUkGHgvDzvsy4CEid1lvcgEUzlln1MkU1WyAYmG2MEcc6uByIbB04BjVbIFqZrCyXvGhDq5nAusx6EM1O6D5E2ys56vfhzq4nQi2PjCEavZAlwh2duftDaEO7iPBVwiMRTWHCLi3f8G+WFRzAgYPqPtjbFcKqjn/gAfbR/ekopor8DZQ30tqxxRUcwOe9qbdgnwAgdWAmykokTYAAAAASUVORK5CYII="

/***/ }),
/* 105 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n'], ['\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: relative;\n  padding-top: 60px;\n  '], ['\n  position: relative;\n  padding-top: 60px;\n  ']),
    _templateObject3 = _taggedTemplateLiteral(['\n  @media only screen and (max-width: ', ') {\n    padding-left: 20px;\n    padding-right: 20px;\n  }\n'], ['\n  @media only screen and (max-width: ', ') {\n    padding-left: 20px;\n    padding-right: 20px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  margin: 45px auto;\n  width: 950px;\n  height: 500px; \n'], ['\n  margin: 45px auto;\n  width: 950px;\n  height: 500px; \n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  @media only screen and (max-width: ', ') {\n    margin-top: 30px;\n  }\n'], ['\n  @media only screen and (max-width: ', ') {\n    margin-top: 30px;\n  }\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  :not(:last-child) {\n    border-right: 1px solid lightgray;\n  }\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    :not(:last-child) {\n      border-right: none;\n    }\n    font-size: 14px;\n    text-align: left;\n  }\n'], ['\n  :not(:last-child) {\n    border-right: 1px solid lightgray;\n  }\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    :not(:last-child) {\n      border-right: none;\n    }\n    font-size: 14px;\n    text-align: left;\n  }\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  font-size: 30px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    display: inline;\n    font-size: 14px;\n  }\n'], ['\n  font-size: 30px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    display: inline;\n    font-size: 14px;\n  }\n']),
    _templateObject8 = _taggedTemplateLiteral(['\n  font-size: 14px;\n  @media only screen and (max-width: ', ') {\n    display: inline;\n  }\n'], ['\n  font-size: 14px;\n  @media only screen and (max-width: ', ') {\n    display: inline;\n  }\n']),
    _templateObject9 = _taggedTemplateLiteral(['\n  /* width: calc(100% - 300px); */\n  max-width: 850px;\n  margin: 45px auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n'], ['\n  /* width: calc(100% - 300px); */\n  max-width: 850px;\n  margin: 45px auto;\n  @media only screen and (max-width: ', ') {\n    width: 100%;\n  }\n']),
    _templateObject10 = _taggedTemplateLiteral(['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 180px;\n  width: 180px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n    width: 325px;\n    height: 325px;\n    background-position: 50% 50%;\n    background-size: 100%;\n  }\n\n  background: url(\n    ', '\n  ) no-repeat;\n  background-size: contain;\n'], ['\n  position: relative;\n  background-color: #0F2D3B;\n  color: white;\n  height: 180px;\n  width: 180px;\n  display: flex;\n  flex-direction: column;\n  align-items: center;\n  justify-content: center;\n  @media only screen and (max-width: ', ') {\n    margin: 12px 0;\n    width: 325px;\n    height: 325px;\n    background-position: 50% 50%;\n    background-size: 100%;\n  }\n\n  background: url(\n    ', '\n  ) no-repeat;\n  background-size: contain;\n']),
    _templateObject11 = _taggedTemplateLiteral(['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n'], ['\n  display: flex;\n  align-items: center;\n  flex-direction: column;\n  justify-content: center;\n  height: 100%;\n']),
    _templateObject12 = _taggedTemplateLiteral(['\n  width: 35px;\n  :hover {\n    cursor: pointer;\n  }\n'], ['\n  width: 35px;\n  :hover {\n    cursor: pointer;\n  }\n']),
    _templateObject13 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 153px;\n  top: 370px;\n'], ['\n  position: absolute;\n  left: 153px;\n  top: 370px;\n']),
    _templateObject14 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 358px;\n  top: 520px;\n'], ['\n  position: absolute;\n  left: 358px;\n  top: 520px;\n']),
    _templateObject15 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 555px;\n  top: 370px;\n'], ['\n  position: absolute;\n  left: 555px;\n  top: 370px;\n']),
    _templateObject16 = _taggedTemplateLiteral(['\n  position: absolute;\n  left: 758px;\n  top: 520px;\n'], ['\n  position: absolute;\n  left: 758px;\n  top: 520px;\n']),
    _templateObject17 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n']),
    _templateObject18 = _taggedTemplateLiteral(['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n'], ['\n  font-size: 10px;\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n  }\n']);

__webpack_require__(7);

__webpack_require__(6);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _reactResponsive = __webpack_require__(19);

var _reactResponsive2 = _interopRequireDefault(_reactResponsive);

var _partner = __webpack_require__(106);

var _partner2 = _interopRequireDefault(_partner);

var _index = __webpack_require__(9);

var _index2 = _interopRequireDefault(_index);

var _ModalEco = __webpack_require__(113);

var _ModalEco2 = _interopRequireDefault(_ModalEco);

var _ModalDid = __webpack_require__(114);

var _ModalDid2 = _interopRequireDefault(_ModalDid);

var _ModalDma = __webpack_require__(115);

var _ModalDma2 = _interopRequireDefault(_ModalDma);

var _ModalHive = __webpack_require__(116);

var _ModalHive2 = _interopRequireDefault(_ModalHive);

var _variable = __webpack_require__(2);

var _ecosystem_expand_btn = __webpack_require__(24);

var _ecosystem_expand_btn2 = _interopRequireDefault(_ecosystem_expand_btn);

var _ecosystem_bg = __webpack_require__(117);

var _ecosystem_bg2 = _interopRequireDefault(_ecosystem_bg);

var _circle_purp = __webpack_require__(118);

var _circle_purp2 = _interopRequireDefault(_circle_purp);

var _circle_tan = __webpack_require__(119);

var _circle_tan2 = _interopRequireDefault(_circle_tan);

var _circle_green = __webpack_require__(120);

var _circle_green2 = _interopRequireDefault(_circle_green);

var _circle_blue = __webpack_require__(121);

var _circle_blue2 = _interopRequireDefault(_circle_blue);

__webpack_require__(12);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }
// import lineImage from '../../../images/ecosystem_title_line.png'


var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    var _this = _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));

    _this.state = {
      showEco: false,
      showHive: false,
      showDma: false,
      showDid: false
    };
    _this.toggleEco = function () {
      var showEco = _this.state.showEco;

      _this.setState({ showEco: !showEco });
    };
    _this.toggleHive = function () {
      var showHive = _this.state.showHive;

      _this.setState({ showHive: !showHive });
    };
    _this.toggleDma = function () {
      var showDma = _this.state.showDma;

      _this.setState({ showDma: !showDma });
    };
    _this.toggleDid = function () {
      var showDid = _this.state.showDid;

      _this.setState({ showDid: !showDid });
    };
    return _this;
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var _state = this.state,
          showEco = _state.showEco,
          showHive = _state.showHive,
          showDma = _state.showDma,
          showDid = _state.showDid;

      return _react2.default.createElement(
        Container,
        { className: 'container' },
        _react2.default.createElement(
          Inner,
          { className: 'container-inner' },
          _react2.default.createElement(
            Header,
            { type: 'flex', justify: 'space-around', gutter: { xs: 8, lg: 16 } },
            _react2.default.createElement(
              _col2.default,
              { xs: 24, sm: 12 },
              _react2.default.createElement(_index2.default, { title: 'Ecosystem' })
            ),
            _react2.default.createElement(
              _col2.default,
              { xs: 24, sm: 12 },
              _react2.default.createElement(
                StatList,
                { type: 'flex', justify: 'space-around', gutter: { xs: 8, lg: 16 } },
                _react2.default.createElement(
                  StatItem,
                  { xs: 24, sm: 8 },
                  _react2.default.createElement(
                    StatItemTitle,
                    null,
                    '1M'
                  ),
                  _react2.default.createElement(
                    StatItemText,
                    null,
                    'Carrier Nodes'
                  )
                ),
                _react2.default.createElement(
                  StatItem,
                  { xs: 24, sm: 8 },
                  _react2.default.createElement(
                    StatItemTitle,
                    null,
                    '300K'
                  ),
                  _react2.default.createElement(
                    StatItemText,
                    null,
                    'DIDs'
                  )
                ),
                _react2.default.createElement(
                  StatItem,
                  { xs: 24, sm: 8 },
                  _react2.default.createElement(
                    StatItemTitle,
                    null,
                    '20+'
                  ),
                  _react2.default.createElement(
                    StatItemText,
                    null,
                    'Projects/Devs'
                  )
                )
              )
            )
          ),
          _react2.default.createElement(
            _reactResponsive2.default,
            { minWidth: 950 },
            _react2.default.createElement(Body, { src: _ecosystem_bg2.default }),
            _react2.default.createElement(EcoExpandIcon, { src: _ecosystem_expand_btn2.default, type: 'blockchain', onClick: this.toggleEco }),
            _react2.default.createElement(HiveExpandIcon, { src: _ecosystem_expand_btn2.default, type: 'eth', onClick: this.toggleHive }),
            _react2.default.createElement(DidExpandIcon, { src: _ecosystem_expand_btn2.default, type: 'runtime', onClick: this.toggleDid }),
            _react2.default.createElement(DMAExpandIcon, { src: _ecosystem_expand_btn2.default, type: 'carrier', onClick: this.toggleDma })
          ),
          _react2.default.createElement(
            _reactResponsive2.default,
            { maxWidth: 950 },
            _react2.default.createElement(
              ListContainer,
              null,
              _react2.default.createElement(
                _row2.default,
                { type: 'flex', justify: 'space-around', gutter: { xs: 8, lg: 24 } },
                _react2.default.createElement(
                  Item,
                  { xs: 24, md: 6, type: 'blockchain', onClick: this.toggleEco },
                  _react2.default.createElement(
                    ItemContent,
                    null,
                    _react2.default.createElement(
                      ItemTitle,
                      null,
                      'ELASTOS ',
                      _react2.default.createElement('br', null),
                      'ECOSYSTEM'
                    ),
                    _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                    _react2.default.createElement(
                      ItemText,
                      null,
                      'Hybrid AuxPoW/PoS ',
                      _react2.default.createElement('br', null),
                      'Consenses'
                    )
                  )
                ),
                _react2.default.createElement(
                  Item,
                  { xs: 24, md: 6, type: 'eth', onClick: this.toggleHive },
                  _react2.default.createElement(
                    ItemContent,
                    null,
                    _react2.default.createElement(
                      ItemTitle,
                      null,
                      'ELASTOS ',
                      _react2.default.createElement('br', null),
                      'HIVE(IPFS)'
                    ),
                    _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                    _react2.default.createElement(
                      ItemText,
                      null,
                      'Solidity Smart Contracts ',
                      _react2.default.createElement('br', null),
                      'at 500-1500+ TPS'
                    )
                  )
                ),
                _react2.default.createElement(
                  Item,
                  { xs: 24, md: 6, type: 'runtime', onClick: this.toggleDma },
                  _react2.default.createElement(
                    ItemContent,
                    null,
                    _react2.default.createElement(
                      ItemTitle,
                      null,
                      'ELASTOS ',
                      _react2.default.createElement('br', null),
                      'BIZFRAMEWORK(DMA)'
                    ),
                    _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                    _react2.default.createElement(
                      ItemText,
                      null,
                      'Secure Platform for the ',
                      _react2.default.createElement('br', null),
                      'Modern Internet'
                    )
                  )
                ),
                _react2.default.createElement(
                  Item,
                  { xs: 24, md: 6, type: 'carrier', onClick: this.toggleDid },
                  _react2.default.createElement(
                    ItemContent,
                    null,
                    _react2.default.createElement(
                      ItemTitle,
                      null,
                      'ELASTOS ',
                      _react2.default.createElement('br', null),
                      'DID'
                    ),
                    _react2.default.createElement(Icon, { src: _ecosystem_expand_btn2.default, type: 'expand' }),
                    _react2.default.createElement(
                      ItemText,
                      null,
                      'Blockchain Powered ',
                      _react2.default.createElement('br', null),
                      'Secure P2P Network'
                    )
                  )
                )
              )
            )
          ),
          _react2.default.createElement(_partner2.default, null)
        ),
        _react2.default.createElement(_ModalEco2.default, { visible: showEco, onClose: this.toggleEco }),
        _react2.default.createElement(_ModalHive2.default, { visible: showHive, onClose: this.toggleHive }),
        _react2.default.createElement(_ModalDma2.default, { visible: showDma, onClose: this.toggleDma }),
        _react2.default.createElement(_ModalDid2.default, { visible: showDid, onClose: this.toggleDid })
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var Inner = _styledComponents2.default.div(_templateObject2);
var Header = (0, _styledComponents2.default)(_row2.default)(_templateObject3, _variable.breakPoint.mobile);
var Body = _styledComponents2.default.img(_templateObject4);
var StatList = (0, _styledComponents2.default)(_row2.default)(_templateObject5, _variable.breakPoint.mobile);
var StatItem = (0, _styledComponents2.default)(_col2.default)(_templateObject6, _variable.breakPoint.mobile);
var StatItemTitle = _styledComponents2.default.div(_templateObject7, _variable.breakPoint.mobile);
var StatItemText = _styledComponents2.default.div(_templateObject8, _variable.breakPoint.mobile);
var ListContainer = _styledComponents2.default.div(_templateObject9, _variable.breakPoint.mobile);
var Item = (0, _styledComponents2.default)(_col2.default)(_templateObject10, _variable.breakPoint.mobile, function (props) {
  return props.type === 'blockchain' && _circle_purp2.default || props.type === 'eth' && _circle_tan2.default || props.type === 'runtime' && _circle_green2.default || props.type === 'carrier' && _circle_blue2.default;
});
var ItemContent = _styledComponents2.default.div(_templateObject11);
var Icon = _styledComponents2.default.img(_templateObject12);
var EcoExpandIcon = (0, _styledComponents2.default)(Icon)(_templateObject13);
var HiveExpandIcon = (0, _styledComponents2.default)(Icon)(_templateObject14);
var DidExpandIcon = (0, _styledComponents2.default)(Icon)(_templateObject15);
var DMAExpandIcon = (0, _styledComponents2.default)(Icon)(_templateObject16);
var ItemTitle = _styledComponents2.default.div(_templateObject17, _variable.breakPoint.mobile);
var ItemText = _styledComponents2.default.div(_templateObject18, _variable.breakPoint.mobile);

/***/ }),
/* 106 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _col = __webpack_require__(4);

var _col2 = _interopRequireDefault(_col);

var _row = __webpack_require__(5);

var _row2 = _interopRequireDefault(_row);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  padding: 50px 0;\n  @media only screen and (max-width: ', ') {\n    padding: 40px 30px;\n    background-color: ', ';\n  }\n'], ['\n  padding: 50px 0;\n  @media only screen and (max-width: ', ') {\n    padding: 40px 30px;\n    background-color: ', ';\n  }\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  margin-bottom: 15px;\n'], ['\n  margin-bottom: 15px;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n\n  box-sizing: border-box;\n  max-height: 75px;\n\n  @media only screen and (max-width: ', ') {\n    \n    padding: 12px 24px;\n    max-width: 100%;\n    max-height: 75px;\n  }\n'], ['\n\n  box-sizing: border-box;\n  max-height: 75px;\n\n  @media only screen and (max-width: ', ') {\n    \n    padding: 12px 24px;\n    max-width: 100%;\n    max-height: 75px;\n  }\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    padding: 10px;\n  }\n'], ['\n  font-size: 12px;\n  margin-bottom: 10px;\n  @media only screen and (max-width: ', ') {\n    padding: 10px;\n  }\n']);

__webpack_require__(6);

__webpack_require__(7);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _HeaderWithLine = __webpack_require__(107);

var _HeaderWithLine2 = _interopRequireDefault(_HeaderWithLine);

var _variable = __webpack_require__(2);

var _color = __webpack_require__(3);

var _logo_topnetwork = __webpack_require__(109);

var _logo_topnetwork2 = _interopRequireDefault(_logo_topnetwork);

var _logo_uptick = __webpack_require__(110);

var _logo_uptick2 = _interopRequireDefault(_logo_uptick);

var _logo_wachsman = __webpack_require__(111);

var _logo_wachsman2 = _interopRequireDefault(_logo_wachsman);

var _logo_ioex = __webpack_require__(112);

var _logo_ioex2 = _interopRequireDefault(_logo_ioex);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }
// import lineImage from '../../../images/elastos_partners_line.png'


// import partner5 from '../../../images/logo_bitmain.png'
var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(StyledHeader, { title: 'ELASTOS PARTNERS' }),
        _react2.default.createElement(
          _row2.default,
          { type: 'flex', justify: 'space-between', gutter: 24 },
          _react2.default.createElement(
            StatItem,
            { xs: 11, md: 4 },
            _react2.default.createElement(StyledImg, { src: _logo_topnetwork2.default })
          ),
          _react2.default.createElement(
            StatItem,
            { xs: 11, md: 4 },
            _react2.default.createElement(StyledImg, { src: _logo_uptick2.default })
          ),
          _react2.default.createElement(
            StatItem,
            { xs: 11, md: 4 },
            _react2.default.createElement(StyledImg, { src: _logo_wachsman2.default })
          ),
          _react2.default.createElement(
            StatItem,
            { xs: 11, md: 4 },
            _react2.default.createElement(StyledImg, { src: _logo_ioex2.default })
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject, _variable.breakPoint.mobile, _color.bg.lightGray);
var StyledHeader = (0, _styledComponents2.default)(_HeaderWithLine2.default)(_templateObject2);
var StyledImg = _styledComponents2.default.img(_templateObject3, _variable.breakPoint.mobile);
var StatItem = (0, _styledComponents2.default)(_col2.default)(_templateObject4, _variable.breakPoint.mobile);

/***/ }),
/* 107 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  font-weight: 400;\n  font-size: 12px;\n  margin-bottom: 40px;\n'], ['\n  font-weight: 400;\n  font-size: 12px;\n  margin-bottom: 40px;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  width: 390px;\n  height: 1px;\n  padding-left: 10px;\n  padding-right: 10px;\n  @media only screen and (max-width: ', ') {\n    width: 80px;\n  }\n'], ['\n  width: 390px;\n  height: 1px;\n  padding-left: 10px;\n  padding-right: 10px;\n  @media only screen and (max-width: ', ') {\n    width: 80px;\n  }\n']);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _ecosystem_title_line = __webpack_require__(18);

var _ecosystem_title_line2 = _interopRequireDefault(_ecosystem_title_line);

var _variable = __webpack_require__(2);

__webpack_require__(12);

__webpack_require__(108);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      var title = this.props.title;

      return _react2.default.createElement(
        Container,
        null,
        _react2.default.createElement(HeaderLine, { src: _ecosystem_title_line2.default }),
        title || 'ELASTOS',
        _react2.default.createElement(HeaderLine, { src: _ecosystem_title_line2.default })
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject);
var HeaderLine = _styledComponents2.default.img(_templateObject2, _variable.breakPoint.mobile);

/***/ }),
/* 108 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 109 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 110 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/logo_uptick.69d59c19.png";

/***/ }),
/* 111 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/logo_wachsman.7c8feee2.png";

/***/ }),
/* 112 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/logo_ioex.036a7658.png";

/***/ }),
/* 113 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(13);

var _Modal2 = _interopRequireDefault(_Modal);

var _btn_close_green = __webpack_require__(17);

var _btn_close_green2 = _interopRequireDefault(_btn_close_green);

var _bucket_runtime_bg = __webpack_require__(29);

var _bucket_runtime_bg2 = _interopRequireDefault(_bucket_runtime_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Ecosystem';
var subtitle = 'A full suite of services for dApp developers';
var textArr = [{
    title: 'Elastos BizFramework(DMA)',
    text: ['', '']
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_green2.default,
                undefined: undefined,
                bgImg: _bucket_runtime_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 114 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(13);

var _Modal2 = _interopRequireDefault(_Modal);

var _btn_close_tan = __webpack_require__(27);

var _btn_close_tan2 = _interopRequireDefault(_btn_close_tan);

var _bucket_eth_bg = __webpack_require__(28);

var _bucket_eth_bg2 = _interopRequireDefault(_bucket_eth_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Sidechains';
var subtitle = 'Solidity Smart Contracts at 1500+ TPS';
var textArr = [{
    title: 'Ethereum but with dPoS',
    text: 'A redifined EVM leverages the Elastos DPoS super nodes to replace the slower PoW consensus with blazing fast DPoS at 1500+ TPS before any sharding.'
}, {
    title: 'Solidity Smart Contracts',
    text: 'The Elastos EVM will be continually update to be fully backwards compatible with Solidity Smart Contracts.'
}, {
    title: 'Dedicated Sidechains',
    text: 'Allocate dedicated TPS with your own sidechain. Then take that sidechain and invent your own multi-chain or scaling solution'
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_tan2.default,
                undefined: undefined,
                bgImg: _bucket_eth_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 115 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(13);

var _Modal2 = _interopRequireDefault(_Modal);

var _btn_close_green = __webpack_require__(17);

var _btn_close_green2 = _interopRequireDefault(_btn_close_green);

var _bucket_carrier_bg = __webpack_require__(30);

var _bucket_carrier_bg2 = _interopRequireDefault(_bucket_carrier_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var title = 'Elastos Runtime';
var subtitle = 'Secure Platform for the Modern Internet';
var textArr = [{
    title: 'Phoase 0 - Decentralized API - NOW',
    text: 'Simple, easy-to-use APIs to port existing apps to the Elastos Ecosystem. A wide variety of services are continually being added by partners and the Cyber Republic initiative'
}, {
    title: 'Phase 1 - Elastos Trinity - 2019',
    text: ['An official framework built on Ionic, which is bundled with SDKs, plugins and a built-in app launcher for quicl dApp development.', 'Access to decentralized services, such as Elastos Hive IPFS mean you can create real dApps with minimal blockchain experience.']
}, {
    title: 'Phase 2 - Elastos Runtime - 2020',
    text: 'With code from development for over 18 years, a revolutionary runtime provides a high performance secure sandbox for dApps. Network traffic is restricted through the Elastos Carrier network only, thus achieving the goal of a true decentralized network that is resistant to attacks.'
}, {
    title: 'Phase 3 - Elastos TEE - 2020+',
    text: ['The trusted execution environment(TEE) further secures Elastos dApps from malicious actors and opens the doors to a myriad of potential applicatons.', 'A hardware root of trust ensures the entire stack is verified by the blockchain.']
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_green2.default,
                undefined: undefined,
                bgImg: _bucket_carrier_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 116 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _Modal = __webpack_require__(13);

var _Modal2 = _interopRequireDefault(_Modal);

var _btn_close_purp = __webpack_require__(8);

var _btn_close_purp2 = _interopRequireDefault(_btn_close_purp);

var _bucket_blockchain_bg = __webpack_require__(26);

var _bucket_blockchain_bg2 = _interopRequireDefault(_bucket_blockchain_bg);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }
// import bodyImg from '../../../images/bucket_carrier_bg.png'


var title = 'Elastos Carrier';
var subtitle = 'Blockchain Powered Secure P2P Network';
var textArr = [{
    title: 'The Future of IoT',
    text: 'With Elastos Carrier end-to-end encryption comes standard. There are few reasons anyone but you should have access to your IoT devices, but in today\'s world companies will any excuse to harvest your data. Take back control with safe, secure networks of the Modern Internet.',
    bgcolor: '#397AA2'
}, {
    title: 'Own Your Own Data',
    text: 'All comunications on the Elastos Carrier netowrk is encrypted and authorized by blockchain IDs, decide who to share your data with and how to use it. Monetize your data or keep it private, it\'s your choice to make.',
    bgcolor: '#3A789E'
}, {
    title: 'Robust Relay Netowrk',
    text: 'The carrier network is designed to allow anyone to connect and without risk of data leaks. Demonstrated tunneling and relay functions allow even bypassing of certain firewalls.',
    bgcolor: '#2F6787'
}];

var _class = function (_React$Component) {
    _inherits(_class, _React$Component);

    function _class() {
        _classCallCheck(this, _class);

        return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
    }

    _createClass(_class, [{
        key: 'render',
        value: function render() {
            var _props = this.props,
                onClose = _props.onClose,
                visible = _props.visible;

            var props = {
                visible: visible,
                title: title,
                subtitle: subtitle,
                onClose: onClose,
                textArr: textArr,
                closeImg: _btn_close_purp2.default,
                undefined: undefined,
                bgImg: _bucket_blockchain_bg2.default
            };
            return _react2.default.createElement(_Modal2.default, props);
        }
    }]);

    return _class;
}(_react2.default.Component);

exports.default = _class;

/***/ }),
/* 117 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/ecosystem_bg.0fdc569f.png";

/***/ }),
/* 118 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/circle_purp.5ce17c40.png";

/***/ }),
/* 119 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/circle_tan.1d14e884.png";

/***/ }),
/* 120 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/circle_green.65e0d8e8.png";

/***/ }),
/* 121 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 122 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _button = __webpack_require__(123);

var _button2 = _interopRequireDefault(_button);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  background: url(', ') no-repeat;\n  background-color: #4cd3a2;\n  background-size: cover;\n  background-position-x: right;\n  background-position-y: center;\n  @media only screen and (max-width: ', ') {\n    background-image: url(', ');\n  }\n'], ['\n  background: url(', ') no-repeat;\n  background-color: #4cd3a2;\n  background-size: cover;\n  background-position-x: right;\n  background-position-y: center;\n  @media only screen and (max-width: ', ') {\n    background-image: url(', ');\n  }\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  position: relative;\n  text-align: left;\n  padding: 60px 80px;\n  @media only screen and (max-width: ', ') {\n    padding: 50px 20px;\n  }\n'], ['\n  position: relative;\n  text-align: left;\n  padding: 60px 80px;\n  @media only screen and (max-width: ', ') {\n    padding: 50px 20px;\n  }\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  margin-bottom: 15px;\n  margin-left: 80px;\n'], ['\n  margin-bottom: 15px;\n  margin-left: 80px;\n']),
    _templateObject4 = _taggedTemplateLiteral(['\n  width: 50px;\n  position: absolute;\n  top: 90px;\n  left: 0;\n\n  @media only screen and (max-width: ', ') {\n    position: initial;\n    top: 0;\n    display: block;\n    margin-top: 20px;\n  }\n'], ['\n  width: 50px;\n  position: absolute;\n  top: 90px;\n  left: 0;\n\n  @media only screen and (max-width: ', ') {\n    position: initial;\n    top: 0;\n    display: block;\n    margin-top: 20px;\n  }\n']),
    _templateObject5 = _taggedTemplateLiteral(['\n  background-color: ', ';\n  color: white;\n  border: none;\n\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n    font-weight: 300;\n    padding: 12px 24px;\n    box-sizing: content-box;\n    line-height: 1.0;\n  }\n'], ['\n  background-color: ', ';\n  color: white;\n  border: none;\n\n  @media only screen and (max-width: ', ') {\n    font-size: 16px;\n    font-weight: 300;\n    padding: 12px 24px;\n    box-sizing: content-box;\n    line-height: 1.0;\n  }\n']),
    _templateObject6 = _taggedTemplateLiteral(['\n  font-size: 14px;\n  margin-top: 20px;\n\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n'], ['\n  font-size: 14px;\n  margin-top: 20px;\n\n  @media only screen and (max-width: ', ') {\n    font-size: 24px;\n  }\n']),
    _templateObject7 = _taggedTemplateLiteral(['\n  font-size: 36px;\n  display: inline;\n  font-weight: lighter;\n  color: ', ';\n'], ['\n  font-size: 36px;\n  display: inline;\n  font-weight: lighter;\n  color: ', ';\n']);

__webpack_require__(124);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _TitleWithLine = __webpack_require__(9);

var _TitleWithLine2 = _interopRequireDefault(_TitleWithLine);

var _cr_logo = __webpack_require__(31);

var _cr_logo2 = _interopRequireDefault(_cr_logo);

var _cr_bg = __webpack_require__(125);

var _cr_bg2 = _interopRequireDefault(_cr_bg);

var _cr_bg3 = __webpack_require__(126);

var _cr_bg4 = _interopRequireDefault(_cr_bg3);

var _cr_title_line = __webpack_require__(36);

var _cr_title_line2 = _interopRequireDefault(_cr_title_line);

var _color = __webpack_require__(3);

var _variable = __webpack_require__(2);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var _class = function (_React$Component) {
  _inherits(_class, _React$Component);

  function _class() {
    _classCallCheck(this, _class);

    return _possibleConstructorReturn(this, (_class.__proto__ || Object.getPrototypeOf(_class)).apply(this, arguments));
  }

  _createClass(_class, [{
    key: 'render',
    value: function render() {
      return _react2.default.createElement(
        Container,
        { className: 'container' },
        _react2.default.createElement(
          Inner,
          { className: 'container-inner' },
          _react2.default.createElement(StyledHeader, { label: 'INTRODUCING', titleColor: 'navy', line: _cr_title_line2.default }),
          _react2.default.createElement(StyledImg, { src: _cr_logo2.default }),
          _react2.default.createElement(
            SubTitle,
            null,
            'cyber republic'
          ),
          _react2.default.createElement(
            Desc,
            null,
            'A decentralized governance model governed by the community and council.'
          ),
          _react2.default.createElement(
            StyledBtn,
            { type: 'round', size: 'large', href: 'https://cyberrepublic.org', target: '_blank', style: { fontSize: 12 } },
            'GET REWARDED TODAY'
          )
        )
      );
    }
  }]);

  return _class;
}(_react2.default.Component);

exports.default = _class;

var Container = _styledComponents2.default.div(_templateObject, _cr_bg2.default, _variable.breakPoint.mobile, _cr_bg4.default);
var Inner = _styledComponents2.default.div(_templateObject2, _variable.breakPoint.mobile);
var StyledHeader = (0, _styledComponents2.default)(_TitleWithLine2.default)(_templateObject3);
var StyledImg = _styledComponents2.default.img(_templateObject4, _variable.breakPoint.mobile);
var StyledBtn = (0, _styledComponents2.default)(_button2.default)(_templateObject5, _color.bg.dark, _variable.breakPoint.mobile);
var Desc = _styledComponents2.default.p(_templateObject6, _variable.breakPoint.mobile);
var SubTitle = _styledComponents2.default.h2(_templateObject7, _color.text.navy);

/***/ }),
/* 123 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/button");

/***/ }),
/* 124 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/button/style");

/***/ }),
/* 125 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/cr_bg.eacf95a0.png";

/***/ }),
/* 126 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/cr_bg.996b1402.png";

/***/ }),
/* 127 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});
exports.profilePhotos = undefined;

var _man_silhouette = __webpack_require__(128);

var _man_silhouette2 = _interopRequireDefault(_man_silhouette);

var _RongChen2x = __webpack_require__(129);

var _RongChen2x2 = _interopRequireDefault(_RongChen2x);

var _FengHan2x = __webpack_require__(130);

var _FengHan2x2 = _interopRequireDefault(_FengHan2x);

var _BenLee2x = __webpack_require__(131);

var _BenLee2x2 = _interopRequireDefault(_BenLee2x);

var _YipengSu2x = __webpack_require__(132);

var _YipengSu2x2 = _interopRequireDefault(_YipengSu2x);

var _KevinZhang2x = __webpack_require__(133);

var _KevinZhang2x2 = _interopRequireDefault(_KevinZhang2x);

var _ShunanYu2x = __webpack_require__(134);

var _ShunanYu2x2 = _interopRequireDefault(_ShunanYu2x);

var _JingyuNiu2x = __webpack_require__(135);

var _JingyuNiu2x2 = _interopRequireDefault(_JingyuNiu2x);

var _SongSjun2x = __webpack_require__(136);

var _SongSjun2x2 = _interopRequireDefault(_SongSjun2x);

var _KiranPachhai2x = __webpack_require__(137);

var _KiranPachhai2x2 = _interopRequireDefault(_KiranPachhai2x);

var _ClarenceLiu2x = __webpack_require__(138);

var _ClarenceLiu2x2 = _interopRequireDefault(_ClarenceLiu2x);

var _brianXinSq = __webpack_require__(139);

var _brianXinSq2 = _interopRequireDefault(_brianXinSq);

var _RebeccaZhu2x = __webpack_require__(140);

var _RebeccaZhu2x2 = _interopRequireDefault(_RebeccaZhu2x);

var _ZachWarsavage2x = __webpack_require__(141);

var _ZachWarsavage2x2 = _interopRequireDefault(_ZachWarsavage2x);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var profilePhotos = exports.profilePhotos = {
    PlaceholderImg: _man_silhouette2.default,
    RongChenImg: _RongChen2x2.default,
    FengHanImg: _FengHan2x2.default,
    BenLeeImg: _BenLee2x2.default,
    YipengSu: _YipengSu2x2.default,
    KevinZhang: _KevinZhang2x2.default,
    FengZhangImg: _BenLee2x2.default,
    RebeccaZhuImg: _RebeccaZhu2x2.default,
    ShunanYuImg: _ShunanYu2x2.default,
    JingyuNiuImg: _JingyuNiu2x2.default,
    SongSjunImg: _SongSjun2x2.default,
    KiranPachhaiImg: _KiranPachhai2x2.default,
    ClarenceLiuImg: _ClarenceLiu2x2.default,
    BrianXinImg: _brianXinSq2.default,
    ZachWarsavageImg: _ZachWarsavage2x2.default
};

/***/ }),
/* 128 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAWMAAAFjCAIAAABnoP22AAAACXBIWXMAAAsTAAALEwEAmpwYAAAFIGlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4gPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iQWRvYmUgWE1QIENvcmUgNS42LWMxNDUgNzkuMTYzNDk5LCAyMDE4LzA4LzEzLTE2OjQwOjIyICAgICAgICAiPiA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPiA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIiB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtbG5zOmRjPSJodHRwOi8vcHVybC5vcmcvZGMvZWxlbWVudHMvMS4xLyIgeG1sbnM6cGhvdG9zaG9wPSJodHRwOi8vbnMuYWRvYmUuY29tL3Bob3Rvc2hvcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIgeG1wOkNyZWF0b3JUb29sPSJBZG9iZSBQaG90b3Nob3AgQ0MgMjAxOSAoTWFjaW50b3NoKSIgeG1wOkNyZWF0ZURhdGU9IjIwMTktMDMtMTNUMDE6NDc6MzgrMDg6MDAiIHhtcDpNb2RpZnlEYXRlPSIyMDE5LTAzLTEzVDAxOjUxOjQxKzA4OjAwIiB4bXA6TWV0YWRhdGFEYXRlPSIyMDE5LTAzLTEzVDAxOjUxOjQxKzA4OjAwIiBkYzpmb3JtYXQ9ImltYWdlL3BuZyIgcGhvdG9zaG9wOkNvbG9yTW9kZT0iMyIgcGhvdG9zaG9wOklDQ1Byb2ZpbGU9InNSR0IgSUVDNjE5NjYtMi4xIiB4bXBNTTpJbnN0YW5jZUlEPSJ4bXAuaWlkOjBjZjIyOTQyLTE1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"

/***/ }),
/* 129 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/01-Rong-Chen@2x.28c3d76a.jpg";

/***/ }),
/* 130 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/02-Feng-Han@2x.cad3d7a4.jpg";

/***/ }),
/* 131 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/03-Ben-Lee@2x.d69053c0.jpg";

/***/ }),
/* 132 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/04-Yipeng-Su@2x.64d264ea.jpg";

/***/ }),
/* 133 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/39-Kevin-Zhang@2x.bd9af0c3.jpg";

/***/ }),
/* 134 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/05-Shunan-Yu@2x.d323acb6.jpg";

/***/ }),
/* 135 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/07-Jingyu-Niu@2x.ede0793c.jpg";

/***/ }),
/* 136 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/06-Song-sjun@2x.b51e2ae9.jpg";

/***/ }),
/* 137 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/41-Kiran-Pachhai@2x.a9b52897.jpg";

/***/ }),
/* 138 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/40-Clarence-Liu@2x.78c7a6b5.jpg";

/***/ }),
/* 139 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/brian-xin-sq.fb145d26.jpg";

/***/ }),
/* 140 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/09-Rebecca-Zhu@2x.1c020dd8.jpg";

/***/ }),
/* 141 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__.p + "static/42-Zach-Warsavage@2x.e2a5cc8e.jpg";

/***/ }),
/* 142 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 143 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _menu = __webpack_require__(32);

var _menu2 = _interopRequireDefault(_menu);

var _drawer = __webpack_require__(144);

var _drawer2 = _interopRequireDefault(_drawer);

var _icon = __webpack_require__(145);

var _icon2 = _interopRequireDefault(_icon);

var _dropdown = __webpack_require__(33);

var _dropdown2 = _interopRequireDefault(_dropdown);

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _templateObject = _taggedTemplateLiteral(['\n  font-size: 32px;\n  color: #0F2D3B;\n'], ['\n  font-size: 32px;\n  color: #0F2D3B;\n']),
    _templateObject2 = _taggedTemplateLiteral(['\n  padding-top: 24px;\n  font-size: 12px;\n  font-weight: 400;\n'], ['\n  padding-top: 24px;\n  font-size: 12px;\n  font-weight: 400;\n']),
    _templateObject3 = _taggedTemplateLiteral(['\n  width: 148px;\n  padding-top: 12px;\n'], ['\n  width: 148px;\n  padding-top: 12px;\n']);

__webpack_require__(34);

__webpack_require__(146);

__webpack_require__(147);

__webpack_require__(35);

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _styledComponents = __webpack_require__(1);

var _styledComponents2 = _interopRequireDefault(_styledComponents);

var _reactResponsive = __webpack_require__(19);

var _reactResponsive2 = _interopRequireDefault(_reactResponsive);

var _variable = __webpack_require__(2);

var _header_logo = __webpack_require__(148);

var _header_logo2 = _interopRequireDefault(_header_logo);

var _btn_dropdown = __webpack_require__(149);

var _btn_dropdown2 = _interopRequireDefault(_btn_dropdown);

var _header_arrow_right = __webpack_require__(150);

var _header_arrow_right2 = _interopRequireDefault(_header_arrow_right);

__webpack_require__(151);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _taggedTemplateLiteral(strings, raw) { return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var App = function (_React$Component) {
  _inherits(App, _React$Component);

  function App() {
    _classCallCheck(this, App);

    var _this = _possibleConstructorReturn(this, (App.__proto__ || Object.getPrototypeOf(App)).call(this));

    _this.state = {
      drawerVisible: false
    };
    return _this;
  }

  _createClass(App, [{
    key: 'render',
    value: function render() {
      var _this2 = this;

      return _react2.default.createElement(
        'div',
        null,
        _react2.default.createElement(
          'div',
          { className: 'top-header' },
          _react2.default.createElement(
            'div',
            null,
            _react2.default.createElement(
              'a',
              { href: '/developer' },
              'DEVELOPER ACCOUNT'
            ),
            ' \xA0',
            _react2.default.createElement('img', { src: _header_arrow_right2.default, className: 'arrow' })
          ),
          _react2.default.createElement(
            'div',
            null,
            _react2.default.createElement(
              'a',
              { href: 'https://cyberrepublic.org/', target: '_blank' },
              'JOIN CYBER REPUBLIC'
            ),
            ' \xA0',
            _react2.default.createElement('img', { src: _header_arrow_right2.default, className: 'arrow' })
          )
        ),
        _react2.default.createElement(
          'div',
          { className: 'header' },
          _react2.default.createElement(
            'div',
            { className: 'logo' },
            _react2.default.createElement(
              'a',
              { href: '/' },
              _react2.default.createElement(Logo, { src: _header_logo2.default })
            )
          ),
          _react2.default.createElement(
            _reactResponsive2.default,
            { minWidth: _variable.breakPoint.mobile },
            _react2.default.createElement(
              NavBar,
              { className: 'nav' },
              _react2.default.createElement(
                'a',
                { href: 'https://news.elastos.org/', target: '_blank' },
                'NEWS'
              ),
              _react2.default.createElement(
                'a',
                { href: 'https://blockchain.elastos.org/', target: '_blank' },
                'BLOCKCHAIN'
              ),
              _react2.default.createElement(
                'a',
                { href: '/team' },
                'TEAM'
              ),
              _react2.default.createElement(
                'a',
                { href: '/roadmap' },
                'ROADMAP'
              ),
              _react2.default.createElement(
                'a',
                { href: 'https://www.elastos.org/downloads/elastos_whitepaper_en.pdf', target: '_blank' },
                'WHITEPAPER'
              ),
              _react2.default.createElement(
                _dropdown2.default,
                { overlay: this.getLanguageMenu() },
                _react2.default.createElement(
                  'a',
                  { href: '#' },
                  'LANGUAGE ',
                  _react2.default.createElement('img', { src: _btn_dropdown2.default, className: 'lan-dropdown' })
                )
              )
            )
          ),
          _react2.default.createElement(
            _reactResponsive2.default,
            { maxWidth: _variable.breakPoint.mobile },
            _react2.default.createElement(
              DrawerButton,
              { onClick: function onClick() {
                  return _this2.setState({ drawerVisible: true });
                } },
              _react2.default.createElement(_icon2.default, { type: 'menu-fold' })
            ),
            _react2.default.createElement(
              _drawer2.default,
              { placement: 'right', closable: false, onClose: function onClose() {
                  return _this2.setState({ drawerVisible: false });
                }, visible: this.state.drawerVisible },
              this.getMobileMenu()
            )
          )
        )
      );
    }
  }, {
    key: 'getLanguageMenu',
    value: function getLanguageMenu() {
      return _react2.default.createElement(
        _menu2.default,
        null,
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return window.location.href = '/';
              } },
            'English'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { onClick: function onClick() {
                return window.location.href = '/zh';
              } },
            'Chinese'
          )
        )
      );
    }
  }, {
    key: 'getMobileMenu',
    value: function getMobileMenu() {
      return _react2.default.createElement(
        _menu2.default,
        null,
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: 'https://news.elastos.org/', rel: 'noopener noreferrer', target: '_blank' },
            'NEWS'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: 'https://blockchain.elastos.org/', rel: 'noopener noreferrer', target: '_blank' },
            'BLOCKCHAIN'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: '/team', rel: 'noopener noreferrer' },
            'TEAM'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: '/roadmap', rel: 'noopener noreferrer' },
            'ROADMAP'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: 'https://www.elastos.org/downloads/elastos_whitepaper_en.pdf', rel: 'noopener noreferrer', target: '_blank' },
            'WHITEPAPER'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          '\xA0'
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: '/#', rel: 'noopener noreferrer' },
            'English'
          )
        ),
        _react2.default.createElement(
          _menu2.default.Item,
          null,
          _react2.default.createElement(
            'a',
            { href: '/zh', rel: 'noopener noreferrer' },
            'Chinese'
          )
        )
      );
    }
  }]);

  return App;
}(_react2.default.Component);

exports.default = App;

var DrawerButton = _styledComponents2.default.a(_templateObject);
var NavBar = _styledComponents2.default.div(_templateObject2);
var Logo = _styledComponents2.default.img(_templateObject3);

/***/ }),
/* 144 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/drawer");

/***/ }),
/* 145 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/icon");

/***/ }),
/* 146 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/drawer/style");

/***/ }),
/* 147 */
/***/ (function(module, exports) {

module.exports = require("antd/lib/icon/style");

/***/ }),
/* 148 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 149 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACgAAAASCAYAAAApH5ymAAABP0lEQVRIicWWMU4DMRBF3xLq0OYGoQBBTYEi5RhwAyoicgsEFTcAKbdIlBOkQEi5QWposwRZ+kbRslkP3gl8aRt7PPPsb69dHJ1ekNAJ8KSQG+A1NcAz30FDXxd4ABbApb6F2roZYFn56gAL4ApYArdAZ6uvo7alYgoDWKt8VcCw/DPgGeg1FO0pZqYxu9Q6XwSsLr9Vu2xyyxcOSVja+8QMLVoBd4pzy3cIlMBny2Rs2eSlwFQGiyfAsWa9diyQq7VYAtMk7sEPYAycAdN/hJuKYSymH6f4DRgC19oDf6WVag7F8K26/+AGeAH6wKP26L5UqkZfNTfVOk03yTswAs6B+R4A58o9Uq1aNQFGhbty4Gh7tHNgudctgDjZnrSzDWBUru0mOz0Ao6y2/8pOT0AStmfZWSfLg9Uq74ctAF/ntF//kR292wAAAABJRU5ErkJggg=="

/***/ }),
/* 150 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAkAAAARCAYAAAAPFIbmAAAAzUlEQVQokYXPrW5CQRCG4ZefVNSgcCjuAFvRNBMIAoFAUsQ6bmGuYC+hdSMKEoFAQZZg0EgUGFwVBtGkodmEkxByzvYzm5k8mZ1B1D5ErUQiZWAMfIlapYiVb+8QmInaUwrF9IGFqD2nUEwbWIpaLYViXoC1qNVTKKYFbEStEYuSqF0T1x+AbtGkLE1g8h86Au/VBNgDneDdqWjSDniNIBZ5aAu8Be++s8YjWt2+ON8379Ec6AXvLo+jMzQFBsG7n7wFI/oERsG739wTgD/jgzCTQUkndgAAAABJRU5ErkJggg=="

/***/ }),
/* 151 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 152 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _react = __webpack_require__(0);

var _react2 = _interopRequireDefault(_react);

var _reactStatic = __webpack_require__(10);

__webpack_require__(153);

__webpack_require__(154);

var _elastos_logo_white = __webpack_require__(155);

var _elastos_logo_white2 = _interopRequireDefault(_elastos_logo_white);

var _footer_email_btn = __webpack_require__(156);

var _footer_email_btn2 = _interopRequireDefault(_footer_email_btn);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function App() {
  return _react2.default.createElement(
    'footer',
    { className: 'footer' },
    _react2.default.createElement(
      _reactStatic.Head,
      null,
      _react2.default.createElement('script', { type: 'text/javascript', src: 'js/elastos-v2.js' })
    ),
    _react2.default.createElement(
      'div',
      { className: 'container' },
      _react2.default.createElement(
        'div',
        { className: 'footer-contact' },
        _react2.default.createElement('img', { src: _elastos_logo_white2.default, className: 'footer-logo' }),
        _react2.default.createElement(
          'h2',
          { className: 'slogan' },
          'Welcome to the modern Internet'
        ),
        _react2.default.createElement('div', { className: 'fill' }),
        _react2.default.createElement(
          'h4',
          { className: 'resources' },
          'Resources'
        ),
        _react2.default.createElement(
          'div',
          { className: 'footer-links' },
          _react2.default.createElement(
            'ul',
            null,
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'downloads/elastos_whitepaper_en.pdf', target: '_blank' },
                'White Paper'
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://wallet.elastos.org', target: '_blank' },
                'Wallet'
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://blockchain.elastos.org/status', target: '_blank' },
                'Block Explorer'
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://github.com/elastos', target: '_blank' },
                'Elastos GitHub'
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://github.com/cyber-republic', target: '_blank' },
                'Cyber Republic Github'
              )
            )
          ),
          _react2.default.createElement(
            'ul',
            null,
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://github.com/elastos/Elastos.Community/tree/master/ElastosLogoAssets', target: '_blank' },
                'Logo Assets'
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://www.elastos.org/en/faq/', target: '_blank' },
                'FAQ'
              )
            )
          )
        ),
        _react2.default.createElement(
          'h4',
          { className: 'contact' },
          'Contact'
        ),
        _react2.default.createElement(
          'p',
          { className: 'contact-email' },
          _react2.default.createElement(
            'a',
            { href: 'mailto:contact@elastos.org' },
            'contact@elastos.org'
          )
        ),
        _react2.default.createElement(
          'div',
          { className: 'social-links' },
          _react2.default.createElement(
            'ul',
            null,
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://twitter.com/Elastos_org', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-twitter' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://t.me/elastosgroup', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-telegram' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://www.instagram.com/elastosofficial/', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-instagram' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://www.reddit.com/r/Elastos/', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-reddit' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://www.youtube.com/channel/UCy5AjgpQIQq3bv8oy_L5WTQ/', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-youtube' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://discordapp.com/invite/MHSUVZN', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-discord' })
              )
            ),
            _react2.default.createElement(
              'li',
              null,
              _react2.default.createElement(
                'a',
                { href: 'https://www.facebook.com/elastosorg/', target: '_blank' },
                _react2.default.createElement('span', { className: 'icon icon-facebook' })
              )
            )
          )
        )
      ),
      _react2.default.createElement(
        'div',
        { className: 'footer-signup' },
        _react2.default.createElement(
          'div',
          { className: 'footer-signup-wrap' },
          _react2.default.createElement(
            'h2',
            { className: 'exclusive' },
            'Get Exclusive Access and News'
          ),
          _react2.default.createElement(
            'form',
            { id: 'footer-form', className: 'global-form', name: 'mailing-list', action: 'https://elastos.us17.list-manage.com/subscribe/post-json?u=8d74b221b8912cf1478a69f37&id=0f5b89d273', method: 'get' },
            _react2.default.createElement(
              'fieldset',
              { className: 'email-wrap' },
              _react2.default.createElement('input', { className: 'email-input', style: { position: 'relative', top: '-12px' }, type: 'email', name: 'Email', 'data-type': 'req', placeholder: 'EMAIL' }),
              _react2.default.createElement(
                'button',
                { type: 'submit', className: 'arrow-submit' },
                ' ',
                _react2.default.createElement('img', { src: _footer_email_btn2.default })
              )
            ),
            _react2.default.createElement(
              'div',
              { className: 'form-msg' },
              _react2.default.createElement(
                'div',
                { className: 'thanks' },
                'Thank you! We will be in contact shortly.'
              )
            )
          )
        )
      )
    )
  );
}
exports.default = App;

/***/ }),
/* 153 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 154 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ }),
/* 155 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 156 */
/***/ (function(module, exports) {

module.exports = "data:image/png;base64,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"

/***/ }),
/* 157 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
    value: true
});

var _createBrowserHistory = __webpack_require__(158);

var _createBrowserHistory2 = _interopRequireDefault(_createBrowserHistory);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var genHistory = function genHistory() {
    if (typeof document !== 'undefined') {
        return (0, _createBrowserHistory2.default)();
    }
    return {};
};
exports.default = genHistory();

/***/ }),
/* 158 */
/***/ (function(module, exports) {

module.exports = require("history/createBrowserHistory");

/***/ }),
/* 159 */
/***/ (function(module, exports) {

// removed by extract-text-webpack-plugin

/***/ })
/******/ ]);
});
//# sourceMappingURL=static.549b7e9d.js.map