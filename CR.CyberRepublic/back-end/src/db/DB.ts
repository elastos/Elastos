import * as mongoose from 'mongoose'
import Test from './Test'
import User from './User'
import Team from './Team'
import User_Team from './User_Team'
import Task from './Task'
import Community from './Community'
import User_Community from './User_Community'
import Task_Candidate from './Task_Candidate'
import Submission from './Submission'
import Suggestion from './Suggestion'
import SuggestionDraft from './SuggestionDraft'
import CVote from './CVote'
import CVote_Tracking from './CVote_Tracking'
import CVote_Summary from './CVote_Summary'
import Permission from './Permission'
import PermissionRole from './Permission_Role'
import Release from './Release'
import Elip from './Elip'
import Elip_Review from './Elip_Review'
import Suggestion_Edit_History from './Suggestion_Edit_History'
import Vid from './Vid'
import Did from './Did'
import Council from './Council'
import Secretariat from './Secretariat'
import Ela_Transaction from './Ela_Transaction'
import Config from './Config'

import Log from './Log'

import { utilCrypto } from '../utility'
import * as uuid from 'uuid'

export default class {
  protected db: any
  public connection: mongoose.ConnectionBase

  constructor() {
    this.db = {}
  }

  public isConnected(): boolean {
    return this.connection && this.connection.readyState === 1
  }

  public async start(): Promise<mongoose.ConnectionBase> {
    const url = process.env.DB_URL
    const db = await mongoose.createConnection(url)
    this.connection = db

    // Setup callback
    this.connection.on('error', this.handleDBError)

    this.connection.on('disconnected', this.handleUnexpectedDisconnect)

    this.connection.on('reconnected', function() {
      console.log('MongoDB reconnected!')
    })

    this.initDB(db)
    await this.initTest()

    await this.prepareRecord()

    return db
  }

  private handleDBError() {
    return (error: any) => {
      console.log('Error is happenning', error)
    }
  }

  private handleUnexpectedDisconnect() {
    console.log('handleUnexpectedDisconnect')

    return (error: any) => {
      console.error('mongodb is disconnect', error)
      setTimeout(() => {
        this.start()
      }, 5000)
    }
  }

  public disconnect() {
    mongoose.connection.close()
  }

  private async initTest() {
    const u = await this.db.Test.find({})
    if (u.length < 1) {
      const rs = await this.db.Test.save({
        name: 'test',
        age: 100,
        time: Date.now()
      })
    }
  }

  private initDB(db) {
    this.db.Test = new Test(db)
    this.db.User = new User(db)
    this.db.Team = new Team(db)
    this.db.User_Team = new User_Team(db)
    this.db.Task_Candidate = new Task_Candidate(db)
    this.db.Task = new Task(db)
    this.db.Community = new Community(db)
    this.db.User_Community = new User_Community(db)
    this.db.Log = new Log(db)
    this.db.Submission = new Submission(db)
    this.db.Suggestion = new Suggestion(db)
    this.db.SuggestionDraft = new SuggestionDraft(db)
    this.db.CVote = new CVote(db)
    this.db.CVote_Tracking = new CVote_Tracking(db)
    this.db.CVote_Summary = new CVote_Summary(db)
    this.db.Permission = new Permission(db)
    this.db.Permission_Role = new PermissionRole(db)
    this.db.Release = new Release(db)
    this.db.Elip = new Elip(db)
    this.db.Elip_Review = new Elip_Review(db)
    this.db.Suggestion_Edit_History = new Suggestion_Edit_History(db)
    this.db.Vid = new Vid(db)
    this.db.Did = new Did(db)
    this.db.Council = new Council(db)
    this.db.Secretariat = new Secretariat(db)
    this.db.Ela_Transaction = new Ela_Transaction(db)
    this.db.Config = new Config(db)
  }

  public getModel(name: string) {
    const rs = this.db[name]
    if (!rs) {
      throw new Error('invalid model name : ' + name)
    }
    return rs
  }

  private async prepareRecord() {
    // create admin user
    const salt = uuid.v4()
    const password = utilCrypto.sha512(process.env.ADMIN_PASSWORD + salt)
    const doc = {
      username: process.env.ADMIN_USERNAME,
      password,
      salt,
      email: 'admin@ebp.com',
      role: 'ADMIN',
      active: true,
      profile: {
        firstName: 'Admin',
        lastName: 'Ebp',
        region: {
          country: 'China',
          city: ''
        }
      }
    }
    try {
      const rs = await this.db.User.save(doc)
      console.log('create admin user =>', rs)
    } catch (err) {
      if (err.code === 11000) {
        console.log('admin user already exists')
      } else {
        console.error(err)
      }
    }
  }
}
