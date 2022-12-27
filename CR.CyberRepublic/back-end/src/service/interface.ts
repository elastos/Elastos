import {Document} from 'mongoose'

export interface DataList {
    total: number,
    list: Document[],
    pageSize?: number,
    page?: number
}