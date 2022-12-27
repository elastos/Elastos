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
        <img src="/img/elastosJS.svg" alt="Elastos Logo" width="25%"/>
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
            <Button href={docUrl('getting-started/intro.html')}>Get Started</Button>
            <Button href={docUrl('bounty/index.html')}><b>Calling Developers:</b> Port Your ETH DApp</Button>
            <Button href={docUrl('bounty/funding.html')}>Get Funded By Cyber Republic</Button>
          </PromoSection>
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
          ```
          Elastos replaces the EVM's PoW consensus with its own DPoS consensus allowing
          for much higher TPS. Our DPoS is backed by voting through our native PoW token (ELA) with over
          40% of Bitcoin's hashpower. This is a separate chain from Ethereum but more performant and secure.
          ```

        </MarkdownBlock>
        <a href="https://coinmarketcap.com/currencies/elastos" target="_blank">
          Our Native Token (ELA)
        </a>
        <MarkdownBlock>
          ```
          This puts Elastos' hourly cost for a 51% attack at over "3x" that of Ethereum,
          without factoring in additional robustness guarantees through DPoS validators.
          (BTC: 900k+, ELA: 300k+, ETH: 120k - USD/hour)
          ```
        </MarkdownBlock>
        <a href="https://www.noderators.org/elastossummary" target="_blank">
          Elastos Hashrate Statistics - https://www.noderators.org/elastossummary
        </a>
      </div>
    </div>
    );

    const ElastosETHHeader = () => (
    <div
    className="productShowcaseSection elaBackground"
    style={{textAlign: 'center'}}>
      <img src="/img/Elastos_Logo_Horizontal_KO_RGB.png" style={{width: '30vw'}}/>
      <br/>
      ELA ETH Sidechain for Ethereum Developers
    </div>
    );

    const ElastosETH = () => (
    <Block align="left" className="elastosETH elaBackground darkBackground">
      {[
        {
          title: 'A familiar stack for Ethereum Developers',
          content: `<span class="normal" style="font-size: 13px;">Elastos is building equivalent versions of your favorite tools</span>

<span class="normal">Block Explorer</span> &#x27A1; use our testnet block explorer at <a href="https://testnet.elaeth.io" target="_blank">https://testnet.elaeth.io</a>, mainet at <a href="https://explorer.elaeth.io" target="_blank">https://explorer.elaeth.io</a>

<span class="normal">MetaMask</span> &#x27A1; directly works on our new RPC port at <a href="https://rpc.elaeth.io" target="_blank">https://rpc.elaeth.io</a>(testnet) and <a href="https://rpc.elaeth.io" target="_blank">https://mainrpc.elaeth.io</a>(mainnet)

<span class="normal">Remix</span> &#x27A1; same as above, point it at our new RPC port

<span class="normal">Myetherwallet</span> &#x27A1; accessible at <a href="https://wallet.elaeth.io" target="_blank">https://wallet.elaeth.io</a>


<a class="button" href=${docUrl('advanced/intro')}>View Our Advanced Tutorial for Ethereum Developers</a>`,
          image: `${baseUrl}img/metamask-logo-eyes.png`,
          imageAlign: 'right',
        },
      ]}
    </Block>
    );

    const Features = () => (
      <Block className="elastosFeatures" layout="fourColumn">
        {[
          {
            title: 'Secured By Bitcoin<br/>Through Merge Mining',
            content: 'Elastos has over 40% of BTC\'s hashpower',
            image: `${baseUrl}img/ela_hashrate.png`,
            imageAlign: 'top',
          },
          {
            title: '3x More Expensive to<br/>Attack Than Ethereum',
            content: 'Our hybrid AuxPoW/DPoS Consensus',
            image: `${baseUrl}img/bucket_eth_info_blk.png`,
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
