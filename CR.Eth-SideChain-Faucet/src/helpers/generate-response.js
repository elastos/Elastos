function generateErrorResponse (response, err) {
    const out = {
      error: {
        code: 500 || err.code,
        title: 'Error' || err.title,
        message: 'Internal server error' || err.message
      }
    };
    console.log(err)
    response.send(out)
}

module.exports = { generateErrorResponse }