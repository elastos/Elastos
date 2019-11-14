import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Popover } from 'antd'
import moment from 'moment'
import MilestoneForm from '@/module/form/MilestoneForm/Container'

class Milestones extends Component {
  constructor(props) {
    super(props)
    this.state = {
      milestones: [
        { date: '2019-6-30', version: 'Alpha version' },
        { date: '2019-8-30', version: 'Beta version' }
      ]
    }
  }

  handleVisibleChange = () => {}

  handleSubmit = () => {}

  handleEdit = () => {}

  render() {
    const { milestones } = this.state
    return (
      <Wrapper>
        <Timeline>
          {milestones.map((item, index) => (
            <Milestone key={index}>
              <Square>
                <div>{moment(item.date).format('MMM D')}</div>
                <div>{item.version}</div>
              </Square>
              <Popover
                content={
                  <MilestoneForm
                    item={{ ...item, date: moment(item.date, 'YYYY-MM-DD') }}
                    onSubmit={this.handleEdit}
                  />
                }
                trigger="click"
                placement="top"
                onVisibleChange={this.handleVisibleChange}
              >
                <Circle />
              </Popover>
            </Milestone>
          ))}

          <Action>
            <Popover
              content={<MilestoneForm onSubmit={this.handleSubmit} />}
              trigger="click"
              placement="top"
              onVisibleChange={this.handleVisibleChange}
            >
              <Button size="small" shape="circle" icon="plus" />
            </Popover>
          </Action>
        </Timeline>
      </Wrapper>
    )
  }
}

Milestones.propTypes = {
  onChang: PropTypes.func,
  initialValue: PropTypes.array
}

export default Milestones

const Wrapper = styled.div`
  margin-bottom: 32px;
  overflow-x: auto;
  overflow-y: hidden;
  padding-bottom: 24px;
`

const Timeline = styled.div`
  display: flex;
`
const Milestone = styled.div`
  padding-right: 16px;
  border-bottom: 1px solid #ced6e3;
`
const Square = styled.div`
  width: 150px;
  height: 88px;
  background: #0f2631;
  color: #fff;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  line-height: 20px;
  > div {
    margin-bottom: 6px;
    &:last-child {
      margin-bottom: 0;
    }
  }
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
  padding-right: 150px;
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
    bottom: -17px;
    color: #ced6e3;
  }
`
