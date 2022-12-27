const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const onLinkClick = props.onLinkClick;
  return (
  <div id="news" className="overflow_auto scrollbar">
    <p className="newstitle">news</p>

    <table>
      <tbody>
      {
        App.getParsedRssFeed().map((item, index) => {
          return (<tr key={index}>
            <td className="padding_10px txtable-row br10">
              <a className="exit_link" target="_blank" href={item.link} onClick={(e) => onLinkClick(e)}>
                <p className="article-days">{item.pubDate}</p>
                <p className="article-title">{item.title}</p>
              </a>
            </td>
          </tr>)
        })
      }

      </tbody>
    </table>
  </div>
  )
}
