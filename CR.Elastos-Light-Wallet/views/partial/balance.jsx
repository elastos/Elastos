const React = require('react');

module.exports = (props) => {
  const App = props.App;
  return (
      <div id="balance" className="pricearea">
        <p className="balance">balance</p>
        <p className="usd-head">USD</p>
        <p className="usd-balance">{App.getUSDBalance()}</p>
        <p className="ela-balance gradient-font">{App.getELABalance()} ELA</p>
      </div>

  )
}
