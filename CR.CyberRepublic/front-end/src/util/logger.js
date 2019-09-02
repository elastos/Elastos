import Rollbar from 'rollbar'

const env = process.env.NODE_ENV
let rollbarToken = undefined
switch (env) {
  case 'staging':
    rollbarToken = 'c66f732b61dc4587bc7b8d52c7c38a8b'
    break
  case 'production':
    rollbarToken = 'c66f732b61dc4587bc7b8d52c7c38a8b'
    break
  default:
    break
}

export const rollbar = () => {
  let rollbar = undefined
  if (env === 'staging' || env === 'production') {
    rollbar = new Rollbar({
      accessToken: rollbarToken,
      captureUncaught: true,
      captureUnhandledRejections: true,
      payload: {
        environment: env,
        client: {
          javascript: {
            code_version: env,
            source_map_enabled: true,
            guess_uncaught_frames: true
          }
        }
      }
    })
  }
  return rollbar
}

export const error = error => {
  if (env === 'staging' || env === 'production') {
    rollbar().error(error)
  } else {
    console.error('dev err...', error)
  }
}
