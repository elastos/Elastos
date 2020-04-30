import React, { Component, Fragment } from 'react'
import _ from 'lodash'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Popover } from 'antd'
import moment from 'moment'
import linkifyStr from 'linkifyjs/string'
import MilestoneForm from '@/module/form/MilestoneForm/Container'
import I18N from '@/I18N'

class Milestones extends Component {
  constructor(props) {
    super(props)
    this.state = {
      milestones: props.initialValue ? props.initialValue : [],
      milestonesTrigger: props.initialValue
        ? this.milestonesTrigger(props.initialValue.length)
        : {},
      currentMilestonesTrigger: {
        index: -1,
        clicked: false,
        clickedSwitch: false
      },
      toggleCreateForm: false
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
    this.setState(
      {
        milestones: [...milestones, values],
        milestonesTrigger: {
          ...milestonesTrigger,
          [milestones.length]: { clicked: false, clickedSwitch: false }
        },
        toggleCreateForm: false
      },
      () => {
        onChange({ milestone: this.state.milestones })
        sessionStorage.setItem(
          'plan-milestone',
          JSON.stringify(this.state.milestones)
        )
      }
    )
  }

  handleEdit = (index, values) => {
    const { milestones } = this.state
    const { onChange } = this.props
    const rs = milestones.map((item, key) => {
      if (index === key) {
        return values
      }
      return item
    })
    this.setState({ milestones: rs, currentMilestonesTrigger: {} }, () => {
      onChange({ milestone: this.state.milestones })
      sessionStorage.setItem(
        'plan-milestone',
        JSON.stringify(this.state.milestones)
      )
    })
  }

  handleClickChange = (index, visible) => {
    this.setState({
      currentMilestonesTrigger: {
        index,
        clicked: visible,
        clickedSwitch: false
      }
    })
  }

  handleClickSwitchChange = index => {
    const { currentMilestonesTrigger } = this.state
    const isCurrentMilestone = currentMilestonesTrigger.index === index
    this.setState({
      currentMilestonesTrigger: {
        index,
        clicked: false,
        clickedSwitch: isCurrentMilestone
          ? !currentMilestonesTrigger.clickedSwitch
          : true
      }
    })
  }

  handleVisibleChange = visible => {
    this.setState({ toggleCreateForm: visible })
  }

  hideCreateForm = () => {
    this.setState({ toggleCreateForm: false })
  }

  getMilestoneTrigger = index => {
    const { currentMilestonesTrigger, milestonesTrigger } = this.state
    const isCurrentMilestone = currentMilestonesTrigger.index === index
    if (isCurrentMilestone) {
      return currentMilestonesTrigger
    }
    return milestonesTrigger[index]
  }

  renderSquare(item, index, isBigSquare = false, isAutoLink = false) {
    const number = (
      <div className="square-number">
        <span>{`#${index + 1}`}</span>
      </div>
    )
    const date = (
      <div className="square-date">
        {moment(item.date).format('MMM D, YYYY')}
      </div>
    )
    const version = (
      <div className="square-content">
        {isAutoLink ? (
          <p
            dangerouslySetInnerHTML={{
              __html: linkifyStr(item.version)
            }}
          />
        ) : (
          <p>{item.version}</p>
        )}
      </div>
    )
    const moreBtn = (
      <Button
        type="link"
        onClick={() => this.handleClickSwitchChange(index)}
        style={{ margin: '4px 0 16px 0' }}
      >
        {this.getMilestoneTrigger(index) && this.getMilestoneTrigger(index).clickedSwitch
          ? I18N.get('suggestion.plan.hideDetail')
          : I18N.get('suggestion.plan.showDetail')}
      </Button>
    )

    return (
      <Square className={isBigSquare ? 'big-square' : ''}>
        {number}
        {date}
        {version}
        {moreBtn}
      </Square>
    )
  }

  render() {
    const { /* milestones, */ toggleCreateForm } = this.state
    const { initialValue: milestones } = this.props
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
                   <Popover
                     content={this.renderSquare(item, index, true)}
                     trigger="click"
                     overlayStyle={{}}
                     visible={this.getMilestoneTrigger(index) && this.getMilestoneTrigger(index).clickedSwitch}
                     onVisibleChange={isVisible => this.handleClickSwitchChange(index)
                     }
                   >
                     <Square>{this.renderSquare(item, index)}</Square>
                   </Popover>
                   <MilestoneEdit>
                     <Popover
                       content={(
                         <MilestoneForm
                           index={index + 1}
                                 preItemDate={
                                 milestones[index - 1] &&
                                 moment(milestones[index - 1].date, 'YYYY-MM-DD')
                                 }
                                 item={{
                                   ...item,
                                   date: moment(item.date, 'YYYY-MM-DD'),
                                   index
                                 }}
                            onSubmit={this.handleEdit}
                                 hidePopover={() => {
                                   this.handleClickChange(index, false)
                                 }}
                         />
                       )}
                       trigger="click"
                       visible={this.getMilestoneTrigger(index) && this.getMilestoneTrigger(index).clicked}
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
                     <Popover
                       content={this.renderSquare(item, index, true)}
                       trigger="click"
                       overlayStyle={{}}
                       visible={this.getMilestoneTrigger(index) && this.getMilestoneTrigger(index).clickedSwitch}
                       onVisibleChange={isVisible => this.handleClickSwitchChange(index)
                       }
                     >
                       <Square>
                         {this.renderSquare(item, index, false, true)}
                       </Square>
                     </Popover>
                     <Circle />
                   </MilestoneItem>
                 </Fragment>
               )}
             </Milestone>
           ))}

          <Action visible={visible}>
            {visible && (
              <Popover
                content={(
                  <MilestoneForm
                    index={_.size(milestones) + 1}
                          preItemDate={
                          _.last(milestones) &&
                          moment(_.last(milestones).date, 'YYYY-MM-DD')
                          }
                    onSubmit={this.handleSubmit}
                          hidePopover={this.hideCreateForm}
                  />
                )}
                trigger="click"
                placement="top"
                visible={toggleCreateForm}
                onVisibleChange={this.handleVisibleChange}
              >
                <CircleButton>+</CircleButton>
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
  overflow-x: auto;
  overflow-y: visible;
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
`
const Square = styled.div`
  position: absolute;
  background: #0f2631;
  color: #fff;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  line-height: 20px;
  width: 100%;
  bottom: 12px;
  > div {
  margin-top: 4px;
  &.square-number {
  position: absolute;
  top: 0px;
  left: 0px;
  height: 24px;
  min-width: 28px;
  margin-top: 0px;
  padding: 3px 7px 4px 7px;
  background-color: rgba(255, 255, 255, 0.2);
  > span {
  font-family: Synthese;
  font-style: normal;
  font-weight: normal;
  font-size: 12px;
  line-height: 17px;
  text-align: center;
  color: #ffffff;
  }
  }
  &.square-date {
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
  color: #1de9b6;
  background-color: transparent;
  border-color: transparent;
  box-shadow: none;
  height: 17px;
  flex: 17px;
  margin: 0 0 13px 0;
  > span {
  vertical-align: top;
  font-family: Synthese;
  font-size: 12px;
  line-height: 17px;
  }
  }
  &.big-square {
  width: 295px;
  left: -135.5px;
  bottom: 5px;
  > div {
  &.square-date {
  margin-top: 20px;
  }
  &.square-content > p {
  overflow-wrap: break-word;
  white-space: normal;
  }
  }
  }
`
const MilestoneEdit = styled.div`
  position: relative;
  top: 97px;
  > button.ant-btn {
  background-color: #1de9b6;
  box-shadow: none;
  border-color: transparent;
  }
`
const Circle = styled.div`
  position: relative;
  top: 101px;
  height: 16px;
  width: 16px;
  border-radius: 50%;
  background: #1de9b6;
  margin: 8px auto -8px;
`
const Action = styled.div`
  padding-left: 60px;
  padding-right: ${props => (props.visible === false ? '0' : '150px')};
  border-bottom: 1px solid #ced6e3;
  position: relative;
  padding-bottom: 24px;
  &:after {
  content: '>';
  position: absolute;
  right: -2px;
  bottom: ${props => (props.visible === false ? '-8px' : '-17px')};
  color: #ced6e3;
  }
`
const CircleButton = styled.div`
  position: absolute;
  bottom: -12px;
  background: #ffffff;
  border: 1px solid #008d85;
  height: 24px;
  width: 24px;
  line-height: 22px;
  text-align: center;
  border-radius: 50%;
  cursor: pointer;
  font-size: 20px;
  font-weight: 600;
  color: #1de9b6;
  font-family: sans-serif;
`
