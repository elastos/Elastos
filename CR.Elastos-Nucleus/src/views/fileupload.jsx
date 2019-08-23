import React, { Component } from "react";
import SyntaxHighlighter from "react-syntax-highlighter";
import { gruvboxDark } from "react-syntax-highlighter/dist/esm/styles/hljs";

import {Grid, Row, Col, FormGroup, Button, ControlLabel, FormControl, ProgressBar} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";

import "../assets/css/fileupload.css"
import axios from "axios";
import {baseUrl} from "../utils/api";


class UserProfile extends Component {

  constructor() {
    super();
      this.state = {
        apiKey:'',
        selectedFile: null,
        output:'',
        loaded:0
      }

    this.handleClick = this.handleClick.bind(this);
  }

  changeHandler = event=>{

    if(event.target.name !== "apiKey"){
      this.setState({
        selectedFile: event.target.files[0]
      })
    }else{
      this.setState({
        apiKey: event.target.value
      })
    }

  }

  uploadFileToServer(){

    const data = new FormData()
    data.append('file', this.state.selectedFile)

    const endpoint = "console/upload";

    axios
      .post(
        baseUrl + endpoint,
        {data},
          {
         onUploadProgress: ProgressEvent => {
           this.setState({
           loaded: (ProgressEvent.loaded / ProgressEvent.total*100),
          })
         },
      headers: {
            api_key: this.state.apiKey
          }
    }).then(response => {
                  this.setState({
                    status: "SUCCESS",
                    output: JSON.stringify(response.data,null, 2)
                  });
          })
          .catch((error) => {
              // Error
              if (error.response) {
                  // The request was made and the server responded with a status code
                  // that falls out of the range of 2xx
                  // console.log(error.response.data);
                  // console.log(error.response.status);
                  // console.log(error.response.headers);
                  this.setState({
                      status: "FAILURE",
                      output: JSON.stringify(error.response.data, null, 2)
                  })
              } else if (error.request) {
                  // The request was made but no response was received
                  // `error.request` is an instance of XMLHttpRequest in the
                  // browser and an instance of
                  // http.ClientRequest in node.js
                  this.setState({
                      status: "FAILURE",
                      output: error.request
                  })
                  console.log(error.request);
              } else {
                  // Something happened in setting up the request that triggered an Error
                  this.setState({
                      status: "FAILURE",
                      output: error.message
                  })
                  console.log('Error', error.message);
              }


          });

  }


  handleClick() {
    //TODO:
    //1.check for the api key
    this.uploadFileToServer();
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="File Upload"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <form method="post" action="#" id="#">
                          <ControlLabel>API Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter your API Key here"
                            name="apiKey"
                            value={this.state.apiKey}
                            onChange={this.changeHandler}
                          />
                          <br />

                          <ControlLabel>Upload Your File </ControlLabel>
                          <div className="form-group files">
                            <label></label>
                            <input type="file" name="file" onChange={this.changeHandler}/>
                          </div>

                          <Button variant="primary"  size="lg" onClick={this.handleClick}>Upload
                          </Button>

                          <ProgressBar striped variant="success" now={this.state.loaded}  label={`${ Math.round(this.state.loaded,2) }%`}>
                          </ProgressBar>
                        </form>
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
                  title="Message content"
                  content={
                      <Row>
                        <Col md={12}>
                          <ControlLabel>Status : {this.state.status}</ControlLabel>
                          <FormControl
                            rows="10"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="output"
                            value={this.state.output}
                            readOnly
                          />
                        </Col>
                      </Row>
                  }
                  />)}
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
                            Uploads a file to the hive.
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`POST /api/1/console/upload HTTP/1.1
Host: localhost:8888
Content-Type: multipart/form-data; boundary=--------------------------942705689964164935672351

headers:{
    "api_key":564732BHU,
}

request.files{
    "file":"sample.txt"
}`}
                        </SyntaxHighlighter>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`HTTP/1.1 200 OK
Vary: Accept
Content-Type: application/json

{
    "Name": "file",
    "Hash": "QmVVmyFP6YLi32BczoTymaV8vgeRG2BP7TV8uvEd3M7qh3",
    "Size": "20"
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

    api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.HIVE_ADD
    headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
    req_data = request.form.to_dict()
    myResponse = requests.get(api_url_base, files=req_data, headers=headers).json()
    return Response(json.dumps(myResponse), 
        status=200,
            mimetype='application/json'
          )`}
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
