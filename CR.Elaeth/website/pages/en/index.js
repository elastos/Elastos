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
        {/* <img src="/img/elastosJS.svg" alt="Elastos Logo" width="25%"/> */}
        <small>{siteConfig.title}</small>
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
          {/* <PromoSection>
            <Button href={docUrl('getting-started/intro.html')}>Get Started</Button>
            <Button href={docUrl('bounty/index.html')}><b>Calling Developers:</b> Port Your ETH DApp</Button>
            <Button href={docUrl('bounty/funding.html')}>Get Funded By Cyber Republic</Button>
          </PromoSection> */}
        </div>
      </SplashContainer>
    );
  }
}

class Index extends React.Component {
  render() {

    const {config: siteConfig, language = ''} = this.props;
    const {baseUrl, docsUrl} = siteConfig;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
    const langPart = `${language ? `${language}/` : ''}`;
    const docUrl = doc => `${baseUrl}${docsPart}${langPart}${doc}`;

    const Block = props => (
      <Container
        padding={['bottom', 'top']}
        id={props.id}
        className={props.className}
        background={props.background}>
        <GridBlock
          align={props.align || 'center'}
          contents={props.children}
          layout={props.layout}
        />
      </Container>
    );

    const Explainer = () => (
    <div
    className="explainer paddingBottom"
    style={{textAlign: 'center'}}>
      <div className="wrapper">
        <MarkdownBlock>
          ETH Task Force is setup in July 2019 which dedicated to initiatives involving Elastos and Ethereum. As the Elastos ETH Sidechain goes live, there is a need to start building up the developer documentation, tutorials, demo apps and business development with other Ethereum projects.
        </MarkdownBlock>
        <MarkdownBlock>
          Till now, ETH Task Force has built/port several key infrastructure services for ETH Sidechain. We are happy to see more developers get involved into the community and using our services.
        </MarkdownBlock>
      </div>
    </div>
    );

    const ElastosETHHeader = () => (
    <div
    className="productShowcaseSection elaBackground"
    style={{textAlign: 'center'}}>
      <img src="/img/Elastos_Logo_Horizontal_KO_RGB.png" style={{width: '30vw'}}/>
    </div>
    );

    const ElastosETH = () => (
    <Block align="center" className="elastosETH elaBackground darkBackground">
      {[
        {
          title: 'What we built',
          content: `
<span class="normal">Block Explorer</span> &#x27A1; Testnet: <a href="https://testnet.elaeth.io" target="_blank">https://testnet.elaeth.io</a>, Mainnet: <a href="https://explorer.elaeth.io" target="_blank">https://explorer.elaeth.io</a>

<span class="normal">JSON RPC</span> &#x27A1; Testnet: <a href="https://rpc.elaeth.io" target="_blank">https://rpc.elaeth.io</a>, Mainnet: <a href="https://rpc.elaeth.io" target="_blank">https://mainrpc.elaeth.io</a>

<span class="normal">MetaMask</span> &#x27A1; Brings Ethereum to your browser. <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-chrome-7.7.0.zip" target="_blank">Chrome</a> / <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-firefox-7.7.0.zip" target="_blank">Firefox</a> / <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-opera-7.7.0.zip" target="_blank">Opera</a>

<span class="normal">Remix</span> &#x27A1; same as above, point it at our new RPC port

<span class="normal">Myetherwallet</span> &#x27A1; accessible at <a href="https://wallet.elaeth.io" target="_blank">https://wallet.elaeth.io</a>

<a target="_blank" class="button" href="https://elastosjs.com/docs/advanced/intro">View Our Advanced Tutorial for Ethereum Developers</a>`,
          // image: `${baseUrl}img/metamask-logo-eyes.png`,
          // imageAlign: 'right',
        },
      ]}
    </Block>
    );

    const Features = () => (
      <Block className="elastosFeatures" layout="fourColumn">
        {[
          {
            title: 'Blockchain Explorer',
            content: 'Able to view information of transaction, block and smart contract',
            image: `${baseUrl}img/explorer.png`,
            imageAlign: 'top',
          },
          {
            title: 'Testnet Faucet',
            content: 'Get free test token to facilitate your development',
            image: `${baseUrl}img/faucet.png`,
            imageAlign: 'top',

          },
          {
            title: 'ELA Metamask',
            content: 'Modified Metamask that added sidechain rpc without need to add them by yourself',
            image: `${baseUrl}img/metamask-logo-eyes.png`,
            imageAlign: 'top',
          }
        ]}
      </Block>
    );

    const ElastosBlockchain = () => {
      return <Block className="elastosBlockchain">
        {[
          {
            title: 'Build on the Modern Internet',
            content: `
<ul>
<li>Elastos is open source with over 100 repos - <a href="https://github.com/elastos" target="_blank">github.com/elastos</a></li>
<li>Our Ethereum Sidechain is only one of multiple services, including Elastos Carrier (P2P decentralized persistent), Elastos Hive (IPFS/Storage Solutions) and more.</li>
<li>We are a global network of over 70 developers building the Modern Internet</li>
</ul>
<br/><a class="button" href="https://elastos.academy" target="_blank">Want to Learn More? Visit the Elastos Academy</a>
<a class="button" href="https://t.me/elastosgroup" target="_blank">Join Us on Telegram <i class="icon icon-telegram"></i></a></a>
`,
            image: `${baseUrl}img/elastos_merge_2.png`,
            imageAlign: 'left',

          },
        ]}
      </Block>
    };

    return (
      <div>
        <HomeSplash siteConfig={siteConfig} language={language} />
        <div className="mainContainer">
          <Explainer />
          <Features />
          <ElastosETHHeader />
          <ElastosETH />
          <ElastosBlockchain />
        </div>
      </div>
    );
  }
}

module.exports = Index;
