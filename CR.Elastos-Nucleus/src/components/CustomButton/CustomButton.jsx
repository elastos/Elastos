import React, { Component } from "react";
import { Button } from "react-bootstrap";
import cx from "classnames";
import PropTypes from "prop-types";
import { withRouter } from 'react-router-dom'


class CustomButton extends Component {
 constructor(props) {
    super(props);
    this.state = {};
    this.handleClick = this.handleClick.bind(this);
  }

 handleClick(){
    console.log(this.props.path);
     this.props.history.push(this.props.path);
 }

  render() {
    const { fill, simple, pullRight, round, block,info} = this.props;

    const btnClasses = cx({
      "btn-fill": fill,
      "btn-simple": simple,
      "pull-right": pullRight,
      "btn-block": block,
      "btn-round": round,
      "bsStyle": info
    });

    return (
      <Button className={btnClasses} onClick={this.handleClick}>
      {this.props.text}
      </Button>
    );
  }
}

CustomButton.propTypes = {
  fill: PropTypes.bool,
  simple: PropTypes.bool,
  pullRight: PropTypes.bool,
  block: PropTypes.bool,
  round: PropTypes.bool
};

export default withRouter(CustomButton);
