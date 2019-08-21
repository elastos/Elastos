import React, { Component } from "react";

import SyntaxHighlighter from "react-syntax-highlighter";
import { gruvboxDark } from "react-syntax-highlighter/dist/esm/styles/hljs";

import {Grid, Row, Col, FormGroup, ControlLabel, FormControl,Button} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import axios from "axios";
import {baseUrl} from "../utils/api";

class UserProfile extends Component {

  constructor () {

    super()

    this.state = {
      inputs: {
        hashKey: '',
        pubKey: '',
        sign:''
      },
      output: ''

    }

    this.handleClick = this.handleClick.bind(this);
  }

  changeHandler = event => {

      const key = event.target.name;
      const value = event.target.value;

      this.setState({
        inputs:{
          ...this.state.inputs,
            [key]: {
              ...this.state.inputs[key],
              value
            }
        }
      });


  }

  verifyMessage(){
    const endpoint = "service/sidechain/did/verify";
    axios.post(baseUrl+endpoint, {
      "msg" : this.state.inputs.hashKey.value,
      "pub" : this.state.inputs.pubKey.value,
      "sig" : this.state.inputs.sign.value
  },{
      headers:{
        "api_key":window.apiKey,
        "Content-Type": "application/json;"
      }
    })
  .then(function (response) {
    console.log(response);
    //TODO:
    //1.set the output here
      this.setState({
        output: response.data
      });
  })
  .catch(function (error) {
    console.log(error);
  });
  }

  handleClick() {
      //TODO:
      //1.check for the api key
         if (window.apiKey !== undefined) {
            this.verifyMessage()
         }
         else{
           this.setState({
             output:'API key has not been generated yet. Please generate one and verify your message'
           })
           console.log('api key not present')
         }
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="Verify contents of the message using DID Public key"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                         <FormGroup controlId="formControlsTextarea">
                          <ControlLabel>Message Hash</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter the message hash here"
                            name="hashKey"
                            value = {this.state.inputs.hashKey.value}
                            onChange = {this.changeHandler}
                          />
                          <ControlLabel>Public Key</ControlLabel>
                           <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="pubKey"
                            placeholder="Enter your public key here"
                            value = {this.state.inputs.pubKey.value}
                            onChange = {this.changeHandler}
                          />
                          <ControlLabel>Signature</ControlLabel>
                           <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="sign"
                            placeholder="Enter your signature here"
                            value = {this.state.inputs.sign.value}
                            onChange = {this.changeHandler}
                          />
                          <br/>
                          <Button onClick={this.handleClick} variant="primary" size="lg">Verify</Button>
                        </FormGroup>

                      </Col>
                    </Row>
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
            <Col md={6}>
              {this.state.output && (
                  <Card
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            value = {this.state.output}
                            readOnly
                          />

                      </Col>
                    </Row>
                    <div className="clearfix" />
                  </form>
                }
              />
              )}

            </Col>
          </Row>
          <Row>
            <Col md={12}>
              <Card
                title="Documentation"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <p>
                            <span className="category" />
                            Verify contents of the message using DID Public key
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`POST /api/1/service/sidechain/did/verify HTTP/1.1
Host: localhost:8888
Content-Type: application/json

{
    "msg": "E4BDA0E5A5BDEFBC8CE4B896E7958D",
    "pub": "02C3F59F337814C6715BBE684EC525B9A3CFCE55D9DEEC53E1EDDB0B352DBB4A54",
    "sig": "E6BB279CBD4727B41F2AA8B18E99B3F99DECBB8737D284FFDD408B356C912EE21AD478BCC0ABD65246938F17DDE64258FD8A9684C0649B23AE1318F7B9CEEEC7"
}
`}
                        </SyntaxHighlighter>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`HTTP/1.1 200 OK
Vary: Accept
Content-Type: application/json

{
    "result": true,
    "status": 200
}`}
                        </SyntaxHighlighter>
                      </Col>
                    </Row>

                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
          </Row>
          <Row>
            <Col md={12}>
              <Card
                title="Code Snippet"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <SyntaxHighlighter language="jsx" style={gruvboxDark}>
                          {`    api_key = request.headers.get('api_key')
        api_status = validate_api_key(api_key)
        if not api_status:
            data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
            return Response(json.dumps(data), 
                status=401,
                mimetype='application/json'
            )

        api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_VERIFY
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        myResponse = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return Response(json.dumps(myResponse), 
                status=myResponse['status'],
                mimetype='application/json'
            )
                          `}
                        </SyntaxHighlighter>
                      </Col>
                    </Row>

                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
          </Row>
        </Grid>
      </div>
    );
  }
}

export default UserProfile;
