import React, { Component } from "react";
import { Grid, Row, Col } from "react-bootstrap";
import axios from "axios";
import { baseUrl } from "../utils/api.js";

class ApiKeygenerator extends Component {
  constructor() {
    super();
    this.handleClick = this.handleClick.bind(this);
    //this.handleOutsideClick = this.handleOutsideClick.bind(this);

    this.state = {
      isKeyGenerated: false,
      apiKey: ""
    };
  }

  getApiKeyFromServer() {
    const endpoint = "common/generateAPIKey";
    axios
      .get(baseUrl + endpoint, {
        //mode: "cors",
        params: {}
      })
      .then(response => {
        this.setState({
          isKeyGenerated: response.data.status === 200,
          apiKey: response.data["API Key"]
        });
        console.log(this.state.apiKey);
      });
  }

  handleClick() {
      //TODO:
        //Do we need to generate a new key every time the button gets clicked?
    if (!this.state.isKeyGenerated) {

        //console.log('key has not been generated yet.so call the api to get the key')
        this.getApiKeyFromServer()

    } else {
        console.log('key has already been generated. so do not call the api again')
    }
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col lg={6} sm={6}>
              <div>
                <button onClick={this.handleClick}>Generate key</button>
                {this.state.isKeyGenerated && (
                  <div>
                    <textarea value={this.state.apiKey} readOnly />
                  </div>
                )}
              </div>
            </Col>
          </Row>
        </Grid>
      </div>
    );
  }
}

export default ApiKeygenerator;

/*
const {render} = ReactDOM;
const {Component, PropTypes} = React;
const {h1, h2, div, input, textarea, a, svg, g, line, text} = styled.default;
const {darken, lighten, transparentize} = polished;
const {grid, template} = GridTemplateParser;

// helpers
const clamp = (value, min, max) => Math.min(Math.max(value, min), max);

// styles
const colors = {
  primary: '#263238',
  secondary: '#1DE9B6',
};

const StyledApp = div`
  display: grid;
  grid-template-columns: 25rem auto;
  grid-template-rows: auto;
  grid-template-areas: "sidebar main";
  width: 100%;
  height: 100vh;
`;

const StyledSidebar = div`
  display: flex;
  flex-direction: column;
  grid-area: sidebar;
  background: ${darken(.1, colors.primary)};
  overflow: hidden;
`;

const StyledMain = div`
  display: flex;
  flex-direction: column;
  grid-area: main;
  padding: 2rem;
  background: ${darken(.05, colors.primary)};
`;

const StyledMainInner = div`
  flex: 1;
  position: relative;
`;

const StyledGrid = svg`
  position: absolute;
  top: 0;
  right: 0;
  bottom: 0;
  left: 0;
  width: 100%;
  height: 100%;
`;

const StyledGridText = text`
  font-family: 'Roboto Mono', monospace;
  font-weight: 500;
  font-size: 1rem;
  text-anchor: middle;
  alignment-baseline: middle;
  fill: ${transparentize(.75, colors.secondary)};
`;

const StyledGridLine = line`
  stroke: ${darken(.01, colors.primary)};
  stroke-width: 1px;
`;

const StyledPreview = div`
  z-index: 5;
  position: relative;
  display: grid;
  grid-template-columns: repeat(${props => props.width}, 1fr);
  grid-template-rows: repeat(${props => props.height}, 1fr);
  grid-template-areas: ${props => props.tpl};
  width: 100%;
  height: 100%;
`;

const StyledTrack = div`
  position: relative;
  grid-area: ${props => props.area};
  cursor: ${props =>
    props.grabbing ? 'grabbing' : 'grab'};
  background: ${transparentize(.97, colors.secondary)};
`;

const StyledHandler = div`
  position: absolute;
  top: ${({position}) =>
    position === 'bottom' ? 'auto' : 0};
  right: ${({position}) =>
    position === 'left' ? 'auto' : 0};
  bottom: ${({position}) =>
    position === 'top' ? 'auto' : 0};
  left: ${({position}) =>
    position === 'right' ? 'auto' : 0};
  width: ${({position, size}) =>
    position === 'left' || position === 'right' ? size : '100%'};
  height: ${({position, size}) =>
    position === 'top' || position === 'bottom' ? size : '100%'};
  cursor: ${({position}) =>
    position === 'left' || position === 'right' ? 'col-resize' : 'row-resize'};
  background: ${colors.secondary};
`;

const StyledHint = div`
  padding: 2rem;
`;

const StyledHintTitle = h1`
  padding-bottom: 1rem;
  font-weight: 500;
  font-size: 1.5rem;
  color: ${colors.secondary};
`;

const StyledHintDescription = div`
  line-height: 1.6;
  font-size: 1rem;
  color: ${lighten(.6, colors.primary)};
`;

const StyledTemplate = div`
  flex: 1;
  display: flex;
  flex-direction: column;
  padding: 2rem;
`;

const StyledTemplateTitle = h2`
  padding-bottom: 1.5rem;
  text-transform: uppercase;
  font-size: .85rem;
  font-weight: 500;
  color: ${colors.secondary};
  letter-spacing: .1rem;
`;

const StyledTemplateControl = div`
  flex: 1;
`;

const StyledTemplateInput = textarea`
  width: 100%;
  height: 100%;
  padding: 2rem;
  background: ${darken(.125, colors.primary)};
  border-radius: 2px;
  border: none;
  resize: none;
  line-height: 1.5;
  font-family: 'Roboto Mono', monospace;
  font-size: .85rem;
  color: #fff;
  transition: background .2s;

  &:focus {
    outline: 0;
    background: ${darken(.15, colors.primary)};
  }
`;

const StyledSettings = div`
  display: flex;
  align-items: center;
  justify-content: center;
  padding-bottom: 2rem;

  &::before,
  &::after {
    content: '';
    flex: 1;
    display: block;
    height: 1px;
    background: ${colors.primary};
  }
`;

const StyledSettingDivider = div`
  text-align: center;
  font-family: 'Roboto Mono';
  font-weight: 500;
  font-size: 1.05rem;
  color: ${lighten(.05, colors.primary)};
`;

const StyledSettingInput = input`
  width: 4rem;
  padding: .4rem .6rem;
  margin: 0 .75rem;
  background: ${darken(.1, colors.primary)};
  border: none;
  border-radius: 2px;
  text-align: center;
  font-family: 'Roboto Mono';
  font-size: .8rem;
  color: #fff;
  transition: background .2s;

  &:focus {
    outline: 0;
    background: ${darken(.125, colors.primary)};
  }
`;

const StyledFoot = div`
  padding: 0 2rem 2rem;
`;

const StyledLink = a`
  display: flex;
  align-items: center;
  justify-content: center;
  text-decoration: none;
  font-weight: 500;
  font-size: .8rem;
  color: ${colors.secondary};
  transition: color .2s;

  &:hover {
    color: #fff;
  }
  &::before,
  &::after {
    content: '';
    flex: 1;
    display: block;
    height: 1px;
    background: ${colors.primary};
  }
  &::before {
    margin-right: .75rem;
  }
  &::after {
    margin-left: .75rem;
  }
`;

function Sidebar(props) {
  return (
    <StyledSidebar>
      <Hint />
      <Template {...props} />
      <Foot />
    </StyledSidebar>
  );
}

function Hint() {
  return (
    <StyledHint>
      <StyledHintTitle>
        CSS Grid Template Builder
      </StyledHintTitle>
      <StyledHintDescription>
        A simple tool to build complex CSS Grid templates.
        Edit the template string below or drag the areas in the preview.
        The changes will reflect in both sides.
      </StyledHintDescription>
    </StyledHint>
  );
}

function Foot() {
  return (
    <StyledFoot>
      <StyledLink href="https://twitter.com/a_dugois" target="_blank">
        Follow me on Twitter!
      </StyledLink>
    </StyledFoot>
  );
}

function Template({tpl, setTracks}) {
  return (
    <StyledTemplate>
      <StyledTemplateTitle>
        Template areas
      </StyledTemplateTitle>
      <StyledTemplateControl>
        <Text value={tpl} onBlur={setTracks}>
          {props => <StyledTemplateInput {...props} />}
        </Text>
      </StyledTemplateControl>
    </StyledTemplate>
  );
}

function Main({tpl, width, height, areas, setArea, setWidth, setHeight}) {
  return (
    <StyledMain>
      <Settings
        width={width}
        height={height}
        setWidth={setWidth}
        setHeight={setHeight} />
      <StyledMainInner>
        <Grid
          width={width}
          height={height}
          areas={areas} />
        <Preview
          tpl={tpl}
          width={width}
          height={height}
          areas={areas}
          setArea={setArea} />
      </StyledMainInner>
    </StyledMain>
  );
}

function Settings({width, height, setWidth, setHeight}) {
  return (
    <StyledSettings>
      <Text value={width} onBlur={setWidth}>
        {props => <StyledSettingInput {...props} />}
      </Text>
      <StyledSettingDivider>x</StyledSettingDivider>
      <Text value={height} onBlur={setHeight}>
        {props => <StyledSettingInput {...props} />}
      </Text>
    </StyledSettings>
  );
}

function Track({area, column, row, grabbing, onMouseDown, onHandlerMouseDown}) {
  return (
    <StyledTrack
      area={area}
      grabbing={grabbing}
      onMouseDown={onMouseDown}>
      <Handler position="top" onMouseDown={onHandlerMouseDown('top')} />
      <Handler position="right" onMouseDown={onHandlerMouseDown('right')} />
      <Handler position="bottom" onMouseDown={onHandlerMouseDown('bottom')} />
      <Handler position="left" onMouseDown={onHandlerMouseDown('left')} />
    </StyledTrack>
  );
}

function Handler({position, onMouseDown}) {
  return (
    <StyledHandler
      size="6px"
      position={position}
      onMouseDown={onMouseDown} />
  );
}

class App extends Component {
  state = {
    tracks: {
      width: 4,
      height: 6,
      areas: {
        head: {
          column: { start: 1, end: 5, span: 4 },
          row: { start: 1, end: 2, span: 1 },
        },
        aside: {
          column: { start: 1, end: 2, span: 1 },
          row: { start: 2, end: 4, span: 2 },
        },
        main: {
          column: { start: 2, end: 5, span: 3 },
          row: { start: 2, end: 6, span: 4 },
        },
        foot: {
          column: { start: 1, end: 5, span: 4 },
          row: { start: 6, end: 7, span: 1 },
        },
      },
    },
  };

  setTracks = evt => {
    this.setState(() => ({tracks: grid(evt.target.value)}));
  };

  integer = (value, previous, min, max) => {
    const int = parseInt(value);
    const safe = isNaN(int) ? previous : clamp(int, min, max);

    return safe;
  };

  setWidth = evt => {
    this.setState(({tracks}) => ({
      tracks: {
        ...tracks,
        width: this.integer(evt.target.value, tracks.width, 1, 100),
      },
    }));
  };

  setHeight = evt => {
    this.setState(({tracks}) => ({
      tracks: {
        ...tracks,
        height: this.integer(evt.target.value, tracks.height, 1, 100),
      },
    }));
  };

  setArea = (key, value) => {
    this.setState(({tracks}) => ({
      tracks: {
        ...tracks,
        areas: {
          ...tracks.areas,
          [key]: value,
        },
      },
    }));
  };

  render() {
    const {tracks} = this.state;
    const {width, height, areas} = tracks;
    const tpl = template(tracks);

    return (
      <StyledApp>
        <Sidebar
          tpl={tpl}
          setTracks={this.setTracks} />
        <Main
          tpl={tpl}
          width={width}
          height={height}
          areas={areas}
          setArea={this.setArea}
          setWidth={this.setWidth}
          setHeight={this.setHeight} />
      </StyledApp>
    );
  }
}

class Text extends Component {
  static defaultProps = {
    value: '',
    onFocus: () => {},
    onBlur: () => {},
    onChange: () => {},
  };

  state = {
    isFocused: false,
    value: this.props.value,
  };

  componentWillReceiveProps({value}) {
    this.setState(() => ({value}));
  }

  handleFocus = evt => {
    evt.persist();

    this.props.onFocus(evt);
    this.setState(() => ({isFocused: true}));
  };

  handleBlur = evt => {
    evt.persist();

    this.props.onBlur(evt);
    this.setState(() => ({isFocused: false}));
  };

  handleChange = evt => {
    evt.persist();

    const {value} = evt.target;

    this.props.onChange(evt);
    this.setState(() => ({value}));
  };

  render() {
    const {isFocused} = this.state;
    const value = isFocused ? this.state.value : this.props.value;

    return this.props.children({
      value,
      onFocus: this.handleFocus,
      onBlur: this.handleBlur,
      onChange: this.handleChange,
    });
  }
}

class Preview extends Component {
  constructor() {
    super();

    this.dx = 0;
    this.dy = 0;
  }

  state = {
    isDragging: false,
    draggedArea: null,
    draggedPosition: null,
  };

  componentDidMount() {
    document.addEventListener('mouseup', this.handleMouseUp);
    document.addEventListener('mousemove', this.handleMouseMove);
  }

  componentWillUnmount() {
    document.removeEventListener('mouseup', this.handleMouseUp);
    document.removeEventListener('mousemove', this.handleMouseMove);
  }

  handleMouseUp = evt => {
    if (this.state.isDragging) {
      this.setState(() => ({
        isDragging: false,
        draggedArea: null,
        draggedPosition: null,
      }));
    }
  };

  handleMouseMove = evt => {
    const {width, height} = this.props;
    const {isDragging, draggedArea, draggedPosition} = this.state;

    if (isDragging) {
      const rect = this.node.getBoundingClientRect();
      const x = Math.round((evt.clientX - rect.left) / rect.width * width);
      const y = Math.round((evt.clientY - rect.top) / rect.height * height);

      switch (true) {
        case typeof draggedPosition === 'string':
          return this.moveHandler(x, y);
        case typeof draggedArea === 'string':
          return this.moveTrack(x, y);
      }
    }
  };

  makeTrackMouseDown = draggedArea => evt => {
    evt.preventDefault();

    const {width, height, areas} = this.props;
    const area = areas[draggedArea];
    const rect = this.node.getBoundingClientRect();

    const x = Math.round((evt.clientX - rect.left) / rect.width * width);
    const y = Math.round((evt.clientY - rect.top) / rect.height * height);

    this.dx = x - area.column.start + 1;
    this.dy = y - area.row.start + 1;

    this.setState(() => ({isDragging: true, draggedArea}));
  };

  makeHandlerMouseDown = draggedArea => draggedPosition => evt => {
    evt.preventDefault();
    this.setState(() => ({isDragging: true, draggedArea, draggedPosition}));
  };

  moveTrack = (x, y) => {
    const {width, height, areas, setArea} = this.props;
    const {draggedArea} = this.state;
    const area = areas[draggedArea];

    const top = this.findAdjacentArea('top', draggedArea);
    const right = this.findAdjacentArea('right', draggedArea);
    const bottom = this.findAdjacentArea('bottom', draggedArea);
    const left = this.findAdjacentArea('left', draggedArea);

    const columnStart = clamp(
      x - this.dx + 1,
      typeof left === 'string' ? areas[left].column.end : 1,
      (typeof right === 'string' ? areas[right].column.start : width + 1) - area.column.span,
    );

    const rowStart = clamp(
      y - this.dy + 1,
      typeof top === 'string' ? areas[top].row.end : 1,
      (typeof bottom === 'string' ? areas[bottom].row.start : height + 1) - area.row.span,
    );

    if (columnStart !== area.column.start || rowStart !== area.row.start) {
      const columnEnd = columnStart + area.column.span;
      const rowEnd = rowStart + area.row.span;

      return setArea(draggedArea, {
        column: {
          ...area.column,
          start: columnStart,
          end: columnEnd,
        },
        row: {
          ...area.row,
          start: rowStart,
          end: rowEnd,
        },
      });
    }
  };

  moveHandler = (x, y) => {
    const {width, height, areas, setArea} = this.props;
    const {draggedPosition, draggedArea} = this.state;
    const area = areas[draggedArea];
    const adj = this.findAdjacentArea(draggedPosition, draggedArea);

    if (draggedPosition === 'top') {
      const start = clamp(
        y + 1,
        typeof adj === 'string' ? areas[adj].row.end : 1,
        area.row.end - 1,
      );

      return setArea(draggedArea, {
        ...area,
        row: {
          ...area.row,
          span: area.row.end - start,
          start,
        },
      });
    }

    if (draggedPosition === 'right') {
      const end = clamp(
        x + 1,
        area.column.start + 1,
        typeof adj === 'string' ? areas[adj].column.start : width + 1,
      );

      return setArea(draggedArea, {
        ...area,
        column: {
          ...area.column,
          span: end - area.column.start,
          end,
        },
      });
    }

    if (draggedPosition === 'bottom') {
      const end = clamp(
        y + 1,
        area.row.start + 1,
        typeof adj === 'string' ? areas[adj].row.start : height + 1,
      );

      return setArea(draggedArea, {
        ...area,
        row: {
          ...area.row,
          span: end - area.row.start,
          end,
        },
      });
    }

    if (draggedPosition === 'left') {
      const start = clamp(
        x + 1,
        typeof adj === 'string' ? areas[adj].column.end : 1,
        area.column.end - 1,
      );

      return setArea(draggedArea, {
        ...area,
        column: {
          ...area.column,
          span: area.column.end - start,
          start,
        },
      });
    }
  };

  findAdjacentArea = (direction, area) => {
    const {areas} = this.props;
    const {column, row} = areas[area];
    const keys = Object.keys(areas);

    if (direction === 'top') {
      return keys.find(key =>
        areas[key].row.end === row.start &&
        areas[key].column.start < column.end &&
        areas[key].column.end > column.start
      );
    }

    if (direction === 'right') {
      return keys.find(key =>
        areas[key].column.start === column.end &&
        areas[key].row.start < row.end &&
        areas[key].row.end > row.start
      );
    }

    if (direction === 'bottom') {
      return keys.find(key =>
        areas[key].row.start === row.end &&
        areas[key].column.start < column.end &&
        areas[key].column.end > column.start
      );
    }

    if (direction === 'left') {
      return keys.find(key =>
        areas[key].column.end === column.start &&
        areas[key].row.start < row.end &&
        areas[key].row.end > row.start
      );
    }
  };

  render() {
    const {tpl, width, height, areas} = this.props;
    const {isDragging, draggedArea, draggedPosition} = this.state;

    return (
      <StyledPreview
        innerRef={node => this.node = node}
        tpl={tpl}
        width={width}
        height={height}>
        {Object.keys(areas).map(area => (
          <Track
            key={area}
            area={area}
            column={areas[area].column}
            row={areas[area].row}
            grabbing={isDragging && draggedArea === area && typeof draggedPosition !== 'string'}
            onMouseDown={this.makeTrackMouseDown(area)}
            onHandlerMouseDown={this.makeHandlerMouseDown(area)} />
        ))}
      </StyledPreview>
    );
  }
}

class Grid extends Component {
  renderArea = area => {
    const {width, height, areas} = this.props;
    const {row, column} = areas[area];

    return Array.from(
      {length: row.span},
      (_, r) => Array.from(
        {length: column.span},
        (_, c) => (
          <StyledGridText
            key={`area${r}${c}`}
            x={`${(column.start + c - .5) / width * 100}%`}
            y={`${(row.start + r - .5) / height * 100}%`}>
            {area}
          </StyledGridText>
        ),
      ),
    );
  };

  renderCols = (_, index) => {
    const {width} = this.props;

    return (
      <StyledGridLine
        key={index}
        x1={`${(index + 1) / width * 100}%`}
        y1="0%"
        x2={`${(index + 1) / width * 100}%`}
        y2="100%" />
    );
  };

  renderRows = (_, index) => {
    const {height} = this.props;

    return (
      <StyledGridLine
        key={index}
        x1="0%"
        y1={`${(index + 1) / height * 100}%`}
        x2="100%"
        y2={`${(index + 1) / height * 100}%`} />
    );
  };

  render() {
    const {
      width,
      height,
      areas,
    } = this.props;

    return (
      <StyledGrid>
        <g>{Object.keys(areas).map(this.renderArea)}</g>
        <g>{Array.from({length: width - 1}, this.renderCols)}</g>
        <g>{Array.from({length: height - 1}, this.renderRows)}</g>
      </StyledGrid>
    );
  }
}

render(
  <App />,
  document.querySelector('#root'),
);*/
