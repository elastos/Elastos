import React from 'react'
import styled from 'styled-components'
import moment from 'moment/moment'
import { Collapse } from 'antd'
import I18N from '@/I18N'
import { breakPoint } from '@/constants/breakPoint'
import { ELIP_STATUS, DATE_FORMAT } from '@/constant'
import userUtil from '@/util/user'

const { Panel } = Collapse

const ReviewHistory = ({ reviews }) => {
  if (!reviews.length) {
    return null
  }
  return (
    <StyledCollapse expandIconPosition="right">
      <Panel header={I18N.get('elip.text.reviewDetails')}>
        {reviews.map(el => {
          const commenterName = el.createdBy
            ? `${userUtil.formatUsername(el.createdBy)}, `
            : ''
          return (
            <StyledRow key={el._id}>
              <Content status={el.status} style={{ padding: 20 }}>
                <Comment>
                  {el.status === ELIP_STATUS.REJECTED
                    ? el.comment.split('\n').map((item) => {
                      return (
                        <span>
                          {item}
                          <br/>
                        </span>
                      )
                    })
                    : 'APPROVED'}
                </Comment>
                <Meta>
                  {commenterName}
                  {moment(el.createdAt).format(DATE_FORMAT)}
                </Meta>
              </Content>
              <Status status={el.status}>
                {I18N.get(`elip.text.${el.status.toLowerCase()}`)}
              </Status>
            </StyledRow>
          )
        })}
      </Panel>
    </StyledCollapse>
  )
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
    margin-top: 16px;
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
const StyledRow = styled.div`
  margin-bottom: 24px;
  display: flex;
  align-items: center;
`
const Content = styled.div`
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
  width: 100%;
`
const Status = styled.div`
  position: absolute;
  right: -88px;
  width: 64px;
  text-align: center;
  height: 16px;
  font-size: 8px;
  line-height: 16px;
  color: #fff;
  background: ${props => {
    return props.status === ELIP_STATUS.REJECTED ? '#be1313' : '#008d85'
  }};
  @media only screen and (max-width: ${breakPoint.mobile}) {
    display: none !important;
  }
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
