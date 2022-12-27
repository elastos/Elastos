/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

class Footer extends React.Component {
  docUrl(doc, language) {
    const baseUrl = this.props.config.baseUrl;
    const docsUrl = this.props.config.docsUrl;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
    const langPart = `${language ? `${language}/` : ''}`;
    return `${baseUrl}${docsPart}${langPart}${doc}`;
  }

  pageUrl(doc, language) {
    const baseUrl = this.props.config.baseUrl;
    return baseUrl + (language ? `${language}/` : '') + doc;
  }

  render() {
    return (
      <footer className="nav-footer" id="footer">
        <section className="sitemap">
          <a href={this.props.config.baseUrl} className="nav-home">
            {this.props.config.footerIcon && (
              <img
                src={this.props.config.baseUrl + this.props.config.footerIcon}
                alt={this.props.config.title}
                width="58"
                height="52"
              />
            )}
          </a>
          <div>
            <h5>Developer Resources</h5>
            <a href="https://developer.elastos.org" target="_blank">
              Official Developer Site
            </a>
            <a href="https://elastos.academy" target="_blank">
              Elastos Academy
            </a>
            <a href="https://www.elastos.org" target="_blank">
              Elastos Official Website
            </a>
            <a href="https://github.com/elastos" target="_blank">
              Elastos Github
            </a>
          </div>
          <div>
            <h5>Community</h5>
            <a href="https://t.me/elastosdev" target="_blank">
              Developers Telegram
            </a>
            <a href="https://t.me/elastosgroup" target="_blank">
              Elastos Telegram
            </a>
            <a href="https://twitter.com/Cyber__Republic" target="_blank">
              Twitter
            </a>
            <a href="https://www.reddit.com/r/Elastos" target="_blank">
              Reddit
            </a>
            <a href="mailto:contact@cyberrepublic.org" target="_blank">
              Contact Us
            </a>
          </div>
          <div>
            <h5>More</h5>
            <a href="https://news.elastos.org" target="_blank">Elastos News</a>
            <a href="https://elanews.net" target="_blank">Community News</a>
            <a href="https://github.com/cyber-republic/elastosjs" target="_blank">Our GitHub</a>
            {/*<a
              className="github-button"
              href={this.props.config.repoUrl}
              data-icon="octicon-star"
              data-count-href="/facebook/docusaurus/stargazers"
              data-show-count="true"
              data-count-aria-label="# stargazers on GitHub"
              aria-label="Star this project on GitHub">
              Star
            </a>
            */}
          </div>
        </section>

        <section className="center text-white">
          This is a community developed website, all information, materials and views expressed are not that of Elastos Foundation.
        </section>
        <section className="copyright">{this.props.config.copyright}</section>
      </footer>
    );
  }
}

module.exports = Footer;
