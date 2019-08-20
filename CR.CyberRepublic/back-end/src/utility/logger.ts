import * as Rollbar from 'rollbar'

export const rollbar = () => {
  let rollbar = undefined
  if (process.env.NODE_ENV !== 'dev') {
    rollbar = new Rollbar({
      accessToken: process.env.ROLLBAR_TOKEN,
      captureUncaught: true,
      captureUnhandledRejections: true,
      payload: { environment: process.env.NODE_ENV }
    })
  }
  return rollbar
}

export const error = (error: any) => {
  if (process.env.NODE_ENV === 'dev') {
    console.error('dev err...', error)
  } else {
    rollbar().error(error)
  }
}
