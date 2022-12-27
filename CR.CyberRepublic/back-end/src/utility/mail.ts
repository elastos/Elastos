import * as Mailgun from 'mailgun-js'
import * as _ from 'lodash'

export default {

    /**
     *
     * @param {any} to
     * @param {any} toName
     * @param {any} subject
     * @param {any} body
     * @param {any} replyTo - {name, email}
     * @param {any} recVariables - multiple receipent metadata
     * @returns {Promise<any>}
     */
    async send(options) {
        const {to, toName, subject, body, replyTo, recVariables} = options

        if (!process.env.MAILGUN_API_KEY || !process.env.MAILGUN_URL) {
            return
        }

        const mailgun = Mailgun({
            apiKey: process.env.MAILGUN_API_KEY,
            domain: process.env.MAILGUN_URL
        })

        const data = {
            from: 'Cyber Republic - Elastos <no-reply@elastosjs.com>',
            to: _.isArray(to) ? to : `${toName} <${to}>`,
            subject: subject,
            html: body,
            'recipient-variables': _.isArray(to) && recVariables
        }

        if (replyTo && !_.isEmpty(replyTo)) {
            data['h:Reply-To'] = `${replyTo.name} <${replyTo.email}>`
        }

        if (process.env.NODE_ENV === 'dev') {
            console.log('Debug - Sending Mail:', data)
        }

        return new Promise((resolve, reject) => {

            resolve()

            mailgun.messages().send(data, function (err, body) {
                if (err) {
                    console.error(err)
                    reject(err)
                    return
                }
                // console.log(body);
                resolve()
            })
        })

    }
}
