/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

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
        <img src="/img/logo.png" alt="Elastos Logo" width="25%"/>
        <small>{siteConfig.tagline}</small>
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
        {/* }<Logo img_src={`${baseUrl}img/undraw_monitor.svg`} /> -- */}
        <div className="inner">
          <ProjectTitle siteConfig={siteConfig} />
          <PromoSection>
            <Button href="#try">Elastos Blockchain</Button>
            <Button href={docUrl('doc1.html')}>Cyber Republic DAO</Button>
            <Button href={docUrl('doc2.html')}>Get Funded By Cyber Republic</Button>
          </PromoSection>
        </div>
      </SplashContainer>
    );
  }
}

class Index extends React.Component {
  render() {

    const {config: siteConfig, language = ''} = this.props;
    const {baseUrl} = siteConfig;

    const Block = props => (
      <Container
        padding={['bottom', 'top']}
        id={props.id}
        background={props.background}>
        <GridBlock
          align="center"
          contents={props.children}
          layout={props.layout}
        />
      </Container>
    );

    const Infographic = () => (
      <div
        className="productShowcaseSection paddingBottom"
        style={{textAlign: 'center'}}>
        <h2>Feature Callout</h2>
        <MarkdownBlock>These are features of this project</MarkdownBlock>
      </div>
    );

    const ElastosBlockchain = () => (
      <Block background="dark">
        {[
          {
            content:
              'Each new Docusaurus project has **randomly-generated** theme colors.',
            image: `${baseUrl}img/undraw_youtube_tutorial.svg`,
            imageAlign: 'right',
            title: 'Randomly Generated Theme Colors',
          },
        ]}
      </Block>
    );

    const ElastosDark = () => (
    <div
    className="productShowcaseSection"
    style={{textAlign: 'center'}}>
      <img src="/img/Elastos_Logo_Horizontal_KO_RGB.png" width="30%"/>
    </div>
    );

    const Features = () => (
      <Block layout="fourColumn">
        {[
          {
            title: 'Secured By Bitcoin<br/>Through Merge Mining',
            content: 'Elastos has over 40% of BTC\'s hashpower',
            image: `${baseUrl}img/hashrate.jpg`,
            imageAlign: 'top',
          },
          {
            title: '3x More Expensive to<br/>Attack Than Ethereum',
            content: 'Our hybrid AuxPoW/DPoS Consensus',
            image: `${baseUrl}img/ethereum_logo.svg`,
            imageAlign: 'top',

          },
          {
            title: 'Solidity Smart Contracts<br/>On Our Own Secure Chain',
            content: 'Easy migrations, usable 400+ TPS speeds',
            image: `${baseUrl}img/ethereum_logo.svg`,
            imageAlign: 'top',
          }
        ]}
      </Block>
    );

    const Showcase = () => {
      return <Block background="light">
        {[
          {
            content:
            'Each new Docusaurus project has **randomly-generated** theme colors.',
            image: `${baseUrl}img/undraw_youtube_tutorial.svg`,
            imageAlign: 'right',
            title: 'Randomly Generated Theme Colors',
          },
        ]}
      </Block>
    };

    return (
      <div>
        <HomeSplash siteConfig={siteConfig} language={language} />
        <div className="mainContainer">
          <Features />
          <ElastosDark />
          <Infographic />
          <ElastosBlockchain />
          <Showcase />
        </div>
      </div>
    );
  }
}

module.exports = Index;
