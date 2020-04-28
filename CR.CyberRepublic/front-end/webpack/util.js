const path = require('path')

module.exports = {
  resolve: function(relativePath) {
    return path.resolve(__dirname, '..', relativePath)
  },
}
