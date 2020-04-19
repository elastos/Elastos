const React = require('react');

module.exports = (props) => {
  const App = props.App;
  return (
  <div id="news">
    <table className="overflow_auto scrollbar">
      <tbody>
      {
        App.getParsedRssFeed().map((item, index) => {
          return (<tr key={index}>
            <td>
              <p className="article-days">{item.pubDate}</p>
              <p className="article-title">{item.title}</p>
            </td>
          </tr>)
        })
      }

      </tbody>
    </table>
  </div>
  )
}

// <div id="news" className="bordered w250px h110px bgcolor_black_hover">
//   <a className="exit_link" target="_blank" href="https://news.elastos.org/feed/">News</a>
// </div>
//   )
// }
