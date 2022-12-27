/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

class HomeSplash extends React.Component {
  render() {
    const {siteConfig, language = ''} = this.props;
    const {baseUrl, docsUrl} = siteConfig;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
    const langPart = `${language ? `${language}/` : ''}`;
    const docUrl = doc => `${baseUrl}${docsPart}${langPart}${doc}`;

    const SplashContainer = props => (
      <div className="homeContainer">
        <div className="homeSplashFade">
          <div className="wrapper homeWrapper">{props.children}</div>
        </div>
      </div>
    );

    const Logo = props => (
      <div className="projectLogo">
        <img src={props.img_src} alt="Project Logo" />
      </div>
    );

    const ProjectTitle = props => (
      <h2 className="projectTitle">
        <img src="/img/elastos_ethereum.png" alt="Elastos Logo" width="25%"/>
        <p style={{ marginTop: '1em' }}>Build Decentralized Apps for the future.</p>
        <small>Ethereum sidechain on Elastos has all the power of Ethereum with more security and lower transaction fee.</small>
      </h2>
    );

    const PromoSection = props => (
      <div className="section promoSection">
        <div className="promoRow">
          <div className="pluginRowBlock">{props.children}</div>
        </div>
      </div>
    );

    const Button = props => (
      <div className="pluginWrapper buttonWrapper">
        <a className="button" href={props.href} target={props.target}>
          {props.children}
        </a>
      </div>
    );

    return (
      <SplashContainer>
        <div className="elaBackground">
          {/* }<Logo img_src={`${baseUrl}img/undraw_monitor.svg`} /> -- */}
          <div className="inner">
            <ProjectTitle siteConfig={siteConfig} />
            <PromoSection>
              <Button target="_blank" href="http://elastosjs.com">Get Started</Button>
              {/* <Button href={docUrl('bounty/index.html')}><b>Calling Developers:</b> Port Your ETH DApp</Button>
              <Button href={docUrl('bounty/funding.html')}>Get Funded By Cyber Republic</Button> */}
            </PromoSection>
          </div>
        </div>
      </SplashContainer>
    );
  }
}

module.exports = HomeSplash;
