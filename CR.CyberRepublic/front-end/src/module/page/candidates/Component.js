import React from 'react'
import { Col, Row, Avatar, Pagination, Spin, Popover } from 'antd'
import styled from 'styled-components'
import _ from 'lodash'
import I18N from '@/I18N'
import StandardPage from '../StandardPage'
import { bg, text } from '@/constants/color'
import Footer from '@/module/layout/Footer/Container'
import { logger } from '@/util'
import Connector from './svg/Connector'
import Square from './svg/Square'
import Circle from './svg/Circle'
import { checkPropTypes } from 'prop-types'

const RANK_TEXT = {
  0: 'TH',
  1: 'ST',
  2: 'ND',
  3: 'RD'
}

const PAGE_SIZE = 12

export default class extends StandardPage {
  constructor(props) {
    super(props)
    this.state = {
      list: [],
      totalVotes: 0,
      pageNum: 1,
      total: 0
    }
    this.ord_loading = _.debounce(this.ord_loading, 500)
  }

  async componentDidMount() {
    this.refetch(true)
  }

  handlePaginationChange = pageNum => {
    this.setState({pageNum}, () => this.refetch())
  }

  getQuery = () => {
    const {pageNum} = this.state
    return {pageNum, pageSize: PAGE_SIZE, state: 'active'}
  }

  refetch = async (isShowLoading = false) => {
    if (isShowLoading) this.ord_loading(true)
    const { listData } = this.props
    const param = this.getQuery()
    try {
      const result = await listData(param)
      this.setState({ list: result.crcandidatesinfo, totalVotes: result.totalvotes, total: result.totalcounts })
    } catch (error) {
      logger.error(error)
    }
    if (isShowLoading) this.ord_loading(false)
  }

  renderLoading() {
    return (
      <div className="flex-center">
        <Spin size="large" />
      </div>

    )
  }

  ord_renderContent() {
    const { list, total, loading} = this.state
    const chunkedList = _.chunk(list, 4)

    return (
      <Wrapper>
        <StyledConnector />
        <StyledSquare />
        <Circles>
          <Circle />
          <Circle />
        </Circles>
        <Container>
          <Header>{I18N.get('cs.candidates')}</Header>
          <Row gutter={24}>
            {loading
              ? this.renderLoading()
              : _.map(chunkedList, (row, rowIndex) => {
                const cols = _.map(row, this.renderCandidate)
                return (
                  <div key={rowIndex}>
                    {cols}
                  </div>
                )
              })}
          </Row>
          <StyledPagination>
            <Pagination
              defaultPageSize={PAGE_SIZE}
              total={total}
              onChange={this.handlePaginationChange}
            />
          </StyledPagination>
        </Container>
        <StyledCircle />
        <Footer />
      </Wrapper>
    )
  }

  renderCandidate = (col, colIndex) => {
    const voteRate = col.votes / this.state.totalVotes * 100
    return (
      <Col lg={6} md={8} sm={12} xs={24} key={colIndex}>
        <Card>
          <StyledAvatar>
            <Avatar src={col.url} shape="square" size={176} icon="user" />
            <Rank>
              <Number>{col.index + 1}</Number>
              <Suffix>
                {RANK_TEXT[col.index + 1]
                  ? RANK_TEXT[col.index + 1]
                  : RANK_TEXT[0]}
              </Suffix>
            </Rank>
          </StyledAvatar>
          <Info>
            <Popover content={_.toUpper(col.nickname)}>
              <Name className="wrap-content">{col.nickname}</Name>
            </Popover>
            <Meta>
              <Popover content={I18N.get(`area.${col.location}`)}>
                <div className="wrap-content country">
                  {I18N.get(`area.${col.location}`)}
                </div>
              </Popover>
              <Popover content={col.url}>
                <div className="wrap-content url">
                  <a href={col.url} target="_blank">
                    {col.url}
                  </a>
                </div>
              </Popover>
              <div className="vote">
                <Popover content={col.votes}>
                  <div className="wrap-content data data-vote">{col.votes}</div>
                </Popover>
                &nbsp;
                {I18N.get("council.candidate.votes")}
              </div>
              <div className="vote">
                <Popover content={voteRate}>
                  <div className="wrap-content data data-rate">{voteRate}</div>
                </Popover>
                {`% ${I18N.get("council.candidate.voteRate")}`}
              </div>
            </Meta>
          </Info>
        </Card>
      </Col>
    );
  }
}

const Wrapper = styled.div`
  background: ${bg.navy};
  position: relative;
`
const StyledConnector = styled(Connector)`
  margin-top: 40px;
`
const StyledSquare = styled(Square)`
  position: absolute;
  right: 60px;
  top: 50px;
`
const StyledCircle = styled(Circle)`
  position: absolute;
  left: 40px;
  top: calc(100% / 2 - 48px);
`
const Circles = styled.div`
  position: absolute;
  right: 345px;
  top: 117px;
`
const Container = styled.div`
  max-width: 888px;
  margin: 0 auto;
`
const Header = styled.div`
  position: relative;
  margin: 27px 0 80px;
  width: 211px;
  height: 64px;
  font-family: 'komu-a', sans-serif;
  font-size: 64px;
  line-height: 64px;
  color: ${text.green};
`
const Card = styled.div`
  width: 201px;
  height: 325px;
  background: ${bg.darkNavy};
  margin: 28px auto;
`
const StyledAvatar = styled.div`
  width: 176px;
  height: 176px;
  position: relative;
  top: -30px;
  background: ${bg.obsidian};
`
const Rank = styled.div`
  padding: 4px 4px 4px 6px;
  min-width: 36px;
  height: 36px;
  position: absolute;
  top: 158px;
  right: -18px;
  background: #18ffff;
  color: #000000;
  display: flex;
  justify-content: center;
`

const Number = styled.div`
  font-family: komu-a;
  font-size: 36px;
  line-height: 36px;
`
const Suffix = styled.div`
  font-family: komu-a;
  font-size: 14px;
  line-height: 14px;
`
const Info = styled.div`
  margin-top: -6px;
  padding-left: 16px;
  padding-right: 25px;
  .wrap-content {
    white-space: nowrap; 
    overflow: hidden;
    text-overflow: ellipsis; 
  }
`
const Meta = styled.div`
  height: 96px;
  margin-top: 10px;
  font-family: Synthese;
  font-size: 14px;
  line-height: 24px;
  color: #f6f9fd;
  opacity: 0.9;
  .country {
    height: 25%;
  }
  .url {
    height: 25%;
    text-overflow: unset;
  }
  .vote {
    height: 25%;
    display: flex;
    .data {
      font-weight: bold;
      color: ${text.white};
    }
    .data-vote {
      max-width: 70%;
    }
    .data-rate {
      max-width: 25%;
    }
  }
}
`
const Name = styled.div`
  height: 30px;
  font-family: komu-a;
  font-size: 30px;
  line-height: 30px;
  color: ${text.white};
`

const StyledPagination = styled.div`
  margin-bottom: 90px;
  text-align: center;
  .ant-pagination-prev .ant-pagination-item-link,
  .ant-pagination-next .ant-pagination-item-link {
    border-color: ${bg.navy};
    background-color: ${bg.navy};
  }
  .ant-pagination-item {
    background-color: ${bg.navy};
    a {
      color: ${text.green};
    }
    &:focus,
    &:hover {
      a {
        color: ${text.white};
      }
    }
  }
  .ant-pagination-item-active a {
    color: ${text.white};
    border-bottom: 2px solid ${text.white};
  }
  .ant-pagination-jump-prev
    .ant-pagination-item-container
    .ant-pagination-item-link-icon,
  .ant-pagination-jump-next
    .ant-pagination-item-container
    .ant-pagination-item-link-icon {
    color: ${text.green};
  }
  .ant-pagination-jump-prev
    .ant-pagination-item-container
    .ant-pagination-item-ellipsis,
  .ant-pagination-jump-next
    .ant-pagination-item-container
    .ant-pagination-item-ellipsis {
    color: ${text.green};
    &:focus,
    &:hover {
      color: ${text.green};
    }
  }
`
