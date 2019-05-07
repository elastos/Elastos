// To avoid XSS attacks of insert javascript:xxx
export const getSafeUrl = (url = '') => {
  if (url.indexOf('http') < 0) {
    return `https://${url}`
  }

  return url
}
