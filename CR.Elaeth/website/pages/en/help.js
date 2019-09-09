/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

function Help(props) {
  const {config: siteConfig, language = ''} = props;
  const {baseUrl, docsUrl} = siteConfig;
  const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
  const langPart = `${language ? `${language}/` : ''}`;
  const docUrl = doc => `${baseUrl}${docsPart}${langPart}${doc}`;

  const supportLinks = [
    {
      content: `Learn more using the [documentation on this site.](${docUrl(
        'docs',
      )})`,
      title: 'Browse Docs',
    },
    {
      content: 'Ask questions in our developers chat on Telegram at <a target="_blank" href="https://t.me/elastosdev">https://t.me/elastosdev</a>',
      title: 'Join the community',
    },
    {
      content: 'Find out what\'s new with Cyber Republic at <a target="_blank" href="https://cyberrepublic.press">https://cyberrepublic.press</a>',
      title: 'Stay up to date',
    },
  ];

  return (
    <div className="docMainWrapper wrapper">
      <Container className="mainContainer documentContainer postContainer">
        <div className="post">
          <header className="postHeader">
            <h1>Need help?</h1>
          </header>
          <p>This project is maintained by the <b>CR ETH Task Force.</b></p>
          <p>We will have more here soon, for now you can reach us at <a target="_blank" href="mailto:contact@cyberrepublic.org">contact@cyberrepublic.org</a></p>
          <GridBlock contents={supportLinks} layout="threeColumn" />
        </div>
      </Container>
    </div>
  );
}

module.exports = Help;
