const querystring = require('querystring')
const https = require('https')
const { debug } = require('../helpers/debug')
const { generateErrorResponse } = require('../helpers/generate-response')

function validateCaptcha (app, captchaResponse) {
	const config = app.config
	return new Promise((resolve, reject) => {
		const isDebug = app.config.debug
		const secret = config.Captcha.secret
	    const post_data_json = {
	      secret,
	      "response": captchaResponse
	    }

	    const post_data = querystring.stringify(post_data_json)

	    debug(isDebug, post_data_json)
	    debug(isDebug, post_data)

	    const post_options = {
	        host: 'www.google.com',
	        port: '443',
	        path: '/recaptcha/api/siteverify',
	        method: 'POST',
	        headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
	    }

	    debug(isDebug, post_options)

	    const post_req = https.request(post_options, function (res) {
	        res.setEncoding('utf8')
	        let output = ''
	        res.on('data', function (chunk) {
	          output += chunk
	        })

	        res.on('end', function () {
	            debug(isDebug, "##############")
	            debug(isDebug, 'Output from validateCaptcha: ')
	            debug(isDebug, output)
	            debug(isDebug, "##############")
	            if (output) {
	              debug(isDebug, JSON.parse(output))
	              resolve(JSON.parse(output))
	            } else {
	              resolve()
	            }
	        })
	    })

	    post_req.on('error', function (error) {
	        debug(isDebug, error)
	        reject(error)
	    })
	    post_req.write(post_data, 'binary', function(error) {
	      if (error) debug(isDebug, error)
	    })
	    post_req.end()
	})
}

module.exports = { validateCaptcha }