/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');
const HomeSplash = require(`${process.cwd()}` + `/core/HomeSplash`)

const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

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
        <h3 className="section-title">Tools we have</h3>
        <p className="center">
          <span className="normal">Block Explorer</span> &#x27A1; Testnet: <a href="https://testnet.elaeth.io" target="_blank">https://testnet.elaeth.io</a>, Mainnet: <a href="https://explorer.elaeth.io" target="_blank">https://explorer.elaeth.io</a>
        </p>
        <p className="center">
          <span className="normal">JSON RPC</span> &#x27A1; Testnet: <a href="https://rpc.elaeth.io" target="_blank">https://rpc.elaeth.io</a>, Mainnet: <a href="https://rpc.elaeth.io" target="_blank">https://mainrpc.elaeth.io</a>
        </p>

        <p className="center">
          <span className="normal">MetaMask</span> &#x27A1; Brings Ethereum to your browser. <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-chrome-7.7.0.zip" target="_blank">Chrome</a> / <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-firefox-7.7.0.zip" target="_blank">Firefox</a> / <a href="https://s3-ap-southeast-1.amazonaws.com/elaeth.io/ela+metamask/metamask-opera-7.7.0.zip" target="_blank">Opera</a>
        </p>

        <p className="center">
        <span class="normal">Remix</span> &#x27A1; same as above, point it at our new RPC port
        </p>

        <p className="center">
          <span class="normal">Myetherwallet</span> &#x27A1; accessible at <a href="https://wallet.elaeth.io" target="_blank">https://wallet.elaeth.io</a>
        </p>
      </div>
    </div>
    );

    const ElastosETH = () => (
      <Container
        padding={['top']}
        background="light">
        <h3 className="section-title">Learn How To Build Dapps</h3>
        <Block align="center" className="elastosETH" layout="threeColumn">
          {[
            {
              title: 'Elastosjs.com',
              content: `<a href="http://elastosjs.com" target="_blank"> Go > </a>`,
              // image: `${baseUrl}img/elastosJS.svg`,
              // imageAlign: 'right',
            },
            {
              title: 'Elastos Academy',
              content: `<a href="https://elastos.academy" target="_blank"> Go > </a>`,
              // image: `${baseUrl}img/metamask-logo-eyes.png`,
              // imageAlign: 'right',
            },
            {
              title: 'Elastos Developer Site',
              content: `<a href="https://developer.elastos.org" target="_blank"> Go > </a>`,
              // image: `${baseUrl}img/metamask-logo-eyes.png`,
              // imageAlign: 'right',
            },
          ]}
        </Block>
      </Container>
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
          {/* <ElastosETHHeader /> */}
          <ElastosETH />
          <ElastosBlockchain />
        </div>
      </div>
    );
  }
}

module.exports = Index;
