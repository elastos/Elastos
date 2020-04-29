const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const page=props.page;
  const hideBanner = () => {
    GuiToggles.hideBanner(page);
  }

  return (
    <div id={page+'Banner'} className = {App.getBannerClass()}>
      <div></div>
      <div className="w50pct">{App.getBannerStatus()}</div>
      <div className="banner-button dark-hover flex_center" onClick={(e) => hideBanner()}>Ok</div>
    </div>
  );
}
