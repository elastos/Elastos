import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Modal, Row, Col } from 'antd'
import I18N from '@/I18N'
import TeamInfoForm from '@/module/form/TeamInfoForm/Container'
import TeamInfoList from './TeamInfoList'

class TeamInfoSection extends Component {
  constructor(props) {
    super(props)
    this.state = {
      visible: false,
      teamInfos: props.initialValue ? props.initialValue : []
    }
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  showModal = () => {
    this.setState({ visible: true, index: -1 })
  }

  handleDelete = index => {
    const { teamInfos } = this.state
    this.setState(
      {
        teamInfos: [...teamInfos.slice(0, index), ...teamInfos.slice(index + 1)]
      },
      () => {
        this.props.onChange({ teamInfo: this.state.teamInfos })
      }
    )
  }

  handleEdit = index => {
    this.setState({ index, visible: true })
  }

  handleSubmit = values => {
    const { teamInfos, index } = this.state
    if (index >= 0) {
      const rs = teamInfos.map((item, key) => {
        if (index === key) {
          return values
        }
        return item
      })
      this.setState({ teamInfos: rs, visible: false }, () => {
        this.props.onChange({ teamInfo: this.state.teamInfos })
      })
      return
    }
    this.setState(
      {
        teamInfos: [...teamInfos, values],
        visible: false
      },
      () => {
        this.props.onChange({ teamInfo: this.state.teamInfos })
      }
    )
  }

  render() {
    const { title } = this.props
    const { teamInfos, index } = this.state
    return (
      <Wrapper>
        <Row>
          <Col span={14}>{title}</Col>
          <Col
            span={10}
            style={{ display: 'flex', justifyContent: 'flex-end' }}
          >
            <Button
              onClick={this.showModal}
              style={{ color: '#000000', borderColor: '#000000' }}
            >
              {I18N.get('suggestion.plan.createTeamInfo')}
            </Button>
          </Col>
        </Row>
        {teamInfos.length ? (
          <TeamInfoList
            list={teamInfos}
            onDelete={this.handleDelete}
            onEdit={this.handleEdit}
          />
        ) : null}
        <Modal
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width="70%"
        >
          {this.state.visible === true ? (
            <TeamInfoForm
              item={index >= 0 ? teamInfos[index] : null}
              onSubmit={this.handleSubmit}
              onCancel={this.hideModal}
            />
          ) : null}
        </Modal>
      </Wrapper>
    )
  }
}

TeamInfoSection.propTypes = {
  title: PropTypes.string,
  onChange: PropTypes.func,
  initialValue: PropTypes.array
}

export default TeamInfoSection

const Wrapper = styled.div`
  margin-bottom: 24px;
`
