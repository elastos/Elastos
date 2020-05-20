import React from 'react'
import styled from 'styled-components'
import { Popover, Modal } from 'antd'
import moment from 'moment'
import linkifyStr from 'linkifyjs/string'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import Signature from './Signature'
import { MILESTONE_STATUS } from '@/constant'
const {
  WAITING_FOR_REQUEST,
  REJECTED,
  WAITING_FOR_APPROVAL,
  WAITING_FOR_WITHDRAW
} = MILESTONE_STATUS

class PaymentList extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      toggle: false,
      stage: null,
      opinion: ''
    }
  }

  hideModal = () => {
    this.setState({ toggle: false })
  }

  showModal = (stage, opinion) => {
    this.setState({ toggle: true, stage, opinion })
  }

  isOwner() {
    const { user, proposer } = this.props
    return user.current_user_id === proposer._id
  }

  isVisible() {
    const { user } = this.props
    return this.isOwner() || user.is_secretary
  }

  renderMilestone = (item) => {
    const date = (
      <div className="square-date">
        {moment(item.date).format('MMM D, YYYY')}
      </div>
    )
    const version = (
      <div className="square-content">
        <p
          dangerouslySetInnerHTML={{
            __html: linkifyStr(item.version)
          }}
        />
      </div>
    )
    return (
      <Square>
        {date}
        {version}
      </Square>
    )
  }

  renderActions(item) {
    const { user } = this.props
    const status = item.status
    if (status === WAITING_FOR_REQUEST) {
      return (
        <td>
          <div
            className="action"
            onClick={() => {
              this.showModal(item.milestoneKey)
            }}
          >
            Apply
          </div>
        </td>
      )
    }
    if (status === REJECTED) {
      return <td>Reapply</td>
    }
    if (status === WAITING_FOR_APPROVAL) {
      return (
        user.is_secretary && (
          <td>
            <div
              className="action approve"
              onClick={() => {
                this.showModal(item.milestoneKey, 'APPROVED')
              }}
            >
              Approve
            </div>
            <div
              className="action reject"
              onClick={() => {
                this.showModal(item.milestoneKey, 'REJECTED')
              }}
            >
              Reject
            </div>
          </td>
        )
      )
    }
    if (status === WAITING_FOR_WITHDRAW) {
      return <td>Withdraw</td>
    }
  }

  renderPaymentItem(item, index) {
    const { milestone } = this.props
    const visible = this.isVisible()
    return (
      <StyledRow key={index}>
        <td>{index + 1}</td>
        <td>{item.type ? I18N.get(`suggestion.budget.${item.type}`) : ''}</td>
        <td>{item.amount}</td>
        <td>
          <MarkdownPreview
            content={item.reasons ? item.reasons : ''}
            style={{ p: { margin: '1em 0' } }}
          />
        </td>
        <td>
          {item.milestoneKey ? (
            <Popover
              content={this.renderMilestone(milestone[item.milestoneKey])}
            >
              <a>
                {`${I18N.get('suggestion.budget.milestone')} #${Number(
                  item.milestoneKey
                ) + 1}`}
              </a>
            </Popover>
          ) : null}
        </td>
        <td>
          <MarkdownPreview
            content={item.criteria ? item.criteria : ''}
            style={{ p: { margin: '1em 0' } }}
          />
        </td>
        <td>{item.status}</td>
        {visible && <td>{this.renderActions(item)}</td>}
      </StyledRow>
    )
  }

  ord_render() {
    const { list, proposalId, actions, user, withdrawalHistory } = this.props
    const visible = this.isVisible()
    return (
      <StyledTable>
        <StyledHead>
          <StyledRow>
            <th>{I18N.get('suggestion.budget.payment')}#</th>
            <th>{I18N.get('suggestion.budget.type')}</th>
            <th>
              {I18N.get('suggestion.budget.amount')}
              (ELA)
            </th>
            <th>{I18N.get('suggestion.budget.reasons')}</th>
            <th>{I18N.get('suggestion.budget.goal')}</th>
            <th>{I18N.get('suggestion.budget.criteria')}</th>
            <th>Status</th>
            {visible && <th>{I18N.get('suggestion.budget.action')}</th>}
          </StyledRow>
        </StyledHead>
        <tbody>
          {list &&
            list.map((item, index) => this.renderPaymentItem(item, index))}
        </tbody>
        <Modal
          maskClosable={false}
          visible={this.state.toggle}
          onCancel={this.hideModal}
          footer={null}
        >
          {this.state.toggle === true ? (
            <Signature
              stage={this.state.stage}
              proposalId={proposalId}
              applyPayment={actions.applyPayment}
              getPaymentSignature={actions.getPaymentSignature}
              hideModal={this.hideModal}
              isSecretary={user.is_secretary}
              opinion={this.state.opinion}
              reviewApplication={actions.reviewApplication}
              application={withdrawalHistory[withdrawalHistory.length - 1]}
            />
          ) : null}
        </Modal>
      </StyledTable>
    )
  }
}

export default PaymentList

const StyledTable = styled.table`
  margin-top: 16px;
  width: 100%;
  font-size: 13px;
  table-layout: auto;
`
const StyledHead = styled.thead`
  > tr {
    background: #0f2631;
  }
  th {
    line-height: 18px;
    padding: 16px;
    color: #fff;
  }
`
const StyledRow = styled.tr`
  width: 100%;
  background: #f2f6fb;
  > td {
    line-height: 18px;
    padding: 8px 16px;
    color: #000;
    overflow-wrap: break-word;
    > button {
      margin: 0 4px;
    }
    .action {
      color: #43af92;
      display: inline-block;
      &:hover {
        cursor: pointer;
      }
    }
    .reject {
      color: red;
      margin-left: 16px;
    }
  }
`
const Square = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  line-height: 20px;
  width: 295px;
  > div {
    margin-top: 4px;
    &.square-date {
      margin-top: 20px;
    }
    &.square-content {
      width: 100%;
      margin-bottom: 27px;
      padding: 0 22px;
      > p {
        padding: 0;
        text-align: center;
        overflow-wrap: break-word;
        white-space: normal;
      }
    }
  }
`
