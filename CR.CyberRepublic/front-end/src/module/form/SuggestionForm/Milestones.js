import React, { Component, Fragment } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Popover } from 'antd'
import moment from 'moment'
import MilestoneForm from '@/module/form/MilestoneForm/Container'
import I18N from '@/I18N'

class Milestones extends Component {
  constructor(props) {
    super(props)
    this.state = {
      milestones: props.initialValue ? props.initialValue : [],
      milestonesTrigger: props.initialValue
        ? this.milestonesTrigger(props.initialValue.length)
        : {}
    }
  }

  milestonesTrigger = size => {
    const triggers = {}
    for (let i = 0; i < size; i++) {
      triggers[i] = { clicked: false, clickedSwitch: false }
    }
    return triggers
  }

  handleSubmit = values => {
    const { milestones, milestonesTrigger } = this.state
    const { onChange } = this.props
    this.setState({ milestones: [...milestones, values] }, () => {
      onChange({ milestone: milestones })
    })
    this.setState({
      milestonesTrigger: {
        ...milestonesTrigger,
        [milestones.length]: { clicked: false, clickedSwitch: false }
      }
    })
  }

  handleEdit = (index, values) => {
    const { milestones, milestonesTrigger } = this.state
    const { onChange } = this.props
    const rs = milestones.map((item, key) => {
      if (index === key) {
        return values
      }
      return item
    })
    this.setState({ milestones: rs,
      milestonesTrigger: {
        ...milestonesTrigger,
        [index]: { clicked: false, clickedSwitch: false }
      } }, () => {
      onChange({ milestone: milestones })
    })
  }

  handleClickChange = (index, visible) => {
    const { milestonesTrigger } = this.state
    this.setState({
      milestonesTrigger: {
        ...milestonesTrigger,
        [index]: { ...milestonesTrigger[index], clicked: visible }
      }
    })
  }

  handleClickSwitchChange = (index) => {
    const { milestonesTrigger } = this.state
    this.setState({
      milestonesTrigger: {
        ...milestonesTrigger,
        [index]: { ...milestonesTrigger[index], clickedSwitch: !milestonesTrigger[index].clickedSwitch }
      }
    })
  }

  render() {
    const { milestones, milestonesTrigger } = this.state
    const { editable } = this.props
    const visible = editable === false ? editable : true
    return (
      <Wrapper>
        <Timeline>
          {milestones &&
            milestones.map((item, index) => (
              <Milestone key={index}>
                {visible ? (
                  <MilestoneItem>
                    <Square
                      className={
                        milestonesTrigger[index].clickedSwitch
                          ? 'big-square'
                          : ''
                      }
                    >
                      <div>{moment(item.date).format('MMM D, YYYY')}</div>
                      <div className="square-content"><p>{item.version}</p></div>
                      <Button
                        type="link"
                        onClick={() => this.handleClickSwitchChange(index)}
                      >
                        {milestonesTrigger[index].clickedSwitch
                          ? I18N.get('suggestion.plan.hideDetail')
                          : I18N.get('suggestion.plan.showDetail')}
                      </Button>
                    </Square>
                    <MilestoneEdit>
                      <Popover
                        content={(
                          <MilestoneForm
                            item={{
                              ...item,
                              date: moment(item.date, 'YYYY-MM-DD'),
                              index
                            }}
                            onSubmit={this.handleEdit}
                          />
                        )}
                        trigger="click"
                        visible={milestonesTrigger[index].clicked}
                        onVisibleChange={isVisible => this.handleClickChange(index, isVisible)
                        }
                        placement="top"
                      >
                        <Button
                          type="primary"
                          size="small"
                          shape="circle"
                          icon="edit"
                        />
                      </Popover>
                    </MilestoneEdit>
                  </MilestoneItem>
                ) : (
                  <Fragment>
                    <MilestoneItem>
                      <Square
                        className={
                          milestonesTrigger[index].clickedSwitch
                            ? 'big-square'
                            : ''
                        }
                      >
                        <div>{moment(item.date).format('MMM D, YYYY')}</div>
                        <div className="square-content"><p>{item.version}</p></div>
                        <Button
                          type="link"
                          onClick={() => this.handleClickSwitchChange(index)}
                        >
                          {milestonesTrigger[index].clickedSwitch
                            ? I18N.get('suggestion.plan.hideDetail')
                            : I18N.get('suggestion.plan.showDetail')}
                        </Button>
                      </Square>
                      <MilestoneEdit>
                        <Circle />
                      </MilestoneEdit>
                    </MilestoneItem>
                  </Fragment>
                )}
              </Milestone>
            ))}

          <Action visible={visible}>
            {visible && (
              <Popover
                content={<MilestoneForm onSubmit={this.handleSubmit} />}
                trigger="click"
                placement="top"
              >
                <Button size="small" shape="circle" icon="plus" />
              </Popover>
            )}
          </Action>
        </Timeline>
      </Wrapper>
    )
  }
}

Milestones.propTypes = {
  onChange: PropTypes.func,
  initialValue: PropTypes.array,
  editable: PropTypes.bool
}

export default Milestones

const Wrapper = styled.div`
  margin-bottom: 32px;
  padding-bottom: 24px;
`

const Timeline = styled.div`
  display: flex;
`
const Milestone = styled.div`
  padding-right: 16px;
  border-bottom: 1px solid #ced6e3;
  button {
    display: block;
    margin: 8px auto -12px;
  }
`
const MilestoneItem = styled.div`
  position: relative;
  width: 150px;
  height: 109px;

  > div.big-square {
    position: absolute;
    left: -72.5px;
    z-index: 1;
    width: 295px;

    > div {
      &:first-child {
        margin-top: 20px;
      }
      &.square-content {
        width: 100%;
        padding: 0 20px;
        > p {
          padding: 0;
          text-align: center;
          overflow-wrap: break-word;
          white-space: normal;
        }
      }
    }
  }
`
const Square = styled.div`
  position: absolute;
  width: 100%;
  bottom: 20px;
  background: #0f2631;
  color: #fff;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  line-height: 20px;
  > div {
    margin-top: 4px;
    &:first-child {
      margin-top: 14px;
    }
    &.square-content {
      width: 100%;
      padding: 0 22px;
      > p {
        padding: 0;
        text-align: center;
        text-overflow: ellipsis;
        white-space: nowrap;
        overflow: hidden;
      }
    }
  }
  > button.ant-btn {
    display: inline-block;
    color: #1DE9B6;
    background-color: transparent;
    border-color: transparent;
    -webkit-box-shadow: none;
    box-shadow: none;
    height: 17px;
    margin: 0 0 13px 0;
    > span {
      vertical-align: top;
      font-family: Synthese;
      font-size: 12px;
      line-height: 17px;
    }
  }
`
const MilestoneEdit = styled.div`
  position: relative;
  top: 97px;
`
const Circle = styled.div`
  height: 16px;
  width: 16px;
  border-radius: 50%;
  background: #1de9b6;
  margin: 8px auto -8px;
`
const Action = styled.div`
  padding-left: 110px;
  padding-right: ${props => (props.visible === false ? '0' : '150px')};
  border-bottom: 1px solid #ced6e3;
  position: relative;
  padding-bottom: 24px;
  > button {
    position: absolute;
    bottom: -12px;
    background: #0f2631;
    border-color: unset;
    > i {
      color: #1de9b6;
    }
  }
  &:after {
    content: '>';
    position: absolute;
    right: -2px;
    bottom: ${props => (props.visible === false ? '-8px' : '-17px')};
    color: #ced6e3;
  }
`
