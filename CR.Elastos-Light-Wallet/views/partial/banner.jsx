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
      <div className="w70pct h90pct overflow_auto flex_center">{App.getBannerStatus()}</div>
      <div className="banner-button scale-hover cursor_def flex_center marginright_15px" onClick={(e) => hideBanner()}>Ok</div>
    </div>
  );
}
