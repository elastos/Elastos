import Base from './Base'
import { Translate } from '@google-cloud/translate'

export default class extends Base {
  public async translate(param: any): Promise<Object> {
    const { text, target } = param
    let translation: string

    // Your Google Cloud Platform project ID
    const projectId = process.env.TRANSLATION_PROJECT_ID
    const keyFilename = process.env.PWD + process.env.TRANSLATION_KEY_FILE

    // Instantiates a client
    const translate = new Translate({
      projectId,
      keyFilename
    })

    // The text to translate
    // The target language
    try {
      const results = await translate.translate(text, target || 'en')
      translation = results[0]
    } catch (err) {
      console.error('ERROR:', err)
    }

    return { translation }
  }
}
