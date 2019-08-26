import React from 'react'
import styled from 'styled-components'
import { Collapse, Row, Col } from 'antd'
import I18N from '@/I18N'
import { ELIP_STATUS } from '@/constant'
const { Panel } = Collapse

class ReviewHistory extends React.Component {
  render() {
    return (
      <StyledCollapse expandIconPosition="right">
        <Panel header={I18N.get('elip.text.reviewDetails')}>
          <StyledRow>
            <LabelCol span={3} />
            <WrapperCol span={17} status="REJECTED">
              <div style={{ padding: 20 }}>
                <Comment>rejected</Comment>
                <Meta>author</Meta>
              </div>
            </WrapperCol>
            <Col span={2}>
              <Status status="REJECTED">{I18N.get('elip.text.rejected')}</Status>
            </Col>
          </StyledRow>
          <StyledRow>
            <LabelCol span={3} />
            <WrapperCol span={17} status="APPROVED">
              <div style={{ padding: 20 }}>
                <Comment>APPROVED</Comment>
                <Meta>author</Meta>
              </div>
            </WrapperCol>
            <Col span={2} status="APPROVED">
              <Status>{I18N.get('elip.text.approved')}</Status>
            </Col>
          </StyledRow>
        </Panel>
      </StyledCollapse>
    )
  }
}

export default ReviewHistory

export const StyledCollapse = styled(Collapse)`
  border: none !important;
  .ant-collapse-content-box {
    padding: 0 !important;
    .ant-row {
      display: flex;
      align-items: center;
    }
  }
  .ant-collapse-content {
    border: none !important;
  }
  .ant-collapse-header {
    margin-top: 38px;
    text-align: center;
    padding-left: 0 !important;
    color: #008d85 !important;
    background-color: white;
    .ant-collapse-arrow {
      right: calc(50% - 80px) !important;
    }
    .ant-collapse-arrow svg {
      margin-top: 3px;
    }
  }
  > .ant-collapse-item {
    border-bottom: none !important;
  }
`
const StyledRow = styled(Row)`
  margin-bottom: 24px;
  display: flex;
  align-items: center;
`
const LabelCol = styled(Col)`
  min-width: 120px;
  text-align: right;
  font-size: 18px;
  margin-right: 20px;
`
const WrapperCol = styled(Col)`
  border: 1px solid rgba(0, 0, 0, 0.1);
  border-left: ${props => {
    return props.status === ELIP_STATUS.REJECTED
      ? '4px solid #be1313'
      : '4px solid #008d85'
  }};
  background: ${props => {
    return props.status === ELIP_STATUS.REJECTED
      ? 'rgba(252, 192, 192, 0.2)'
      : 'rgba(29, 233, 182, 0.1)'
  }};
`
const Status = styled.div`
  width: 63px;
  text-align: center;
  height: 16px;
  margin-left: 20px;
  font-size: 8px;
  line-height: 16px;
  color: #fff;
  background: ${props => {
    return props.status === ELIP_STATUS.REJECTED ? '#be1313' : '#008d85'
  }};
`
const Comment = styled.div`
  font-size: 14px;
  line-height: 20px;
`
const Meta = styled.div`
  margin-top: 10px;
  font-size: 12px;
  line-height: 17px;
  color: rgba(3, 30, 40, 0.4);
`
