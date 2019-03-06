import Base from './Base'
const translate = require('google-translate-api')


export default class extends Base {

  public async translate(param: any): Promise<Object> {
    console.log('translate start ------------')
    const { text } = param

    let res: any
    try {
      res = await translate(text, {to: 'en'})
    } catch (err) {
      console.error(err)
      return err
    }

    console.log('translate end ------------')
    return res.text;
  }
}
