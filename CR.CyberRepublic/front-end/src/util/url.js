// To avoid XSS attacks of insert javascript:xxx
export const getSafeUrl = (url = '') => {
  if (url.indexOf('http') < 0) {
    return `https://${url}`
  }

  return url
}

export const getSiteUrl = () => {
  let host = ''
  const env = process.env.NODE_ENV
  if (env === 'development') {
    host = 'http://localhost:3001'
  } else if (env === 'staging') {
    host = 'https://staging.cyberrepublic.org'
  } else {
    host = 'https://cyberrepublic.org'
  }
  return host
}
