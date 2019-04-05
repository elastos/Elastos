

//! - GLOBAL: CONTACT FORM

let formSent = false
let formURL

// wait for form to mount before binding
setTimeout(() => {
  $('.signup-form').submit(function () {
    if (validateForm($(this))) {
      sendForm($(this))
    }
    return false
  })
}, 1000)

function sendForm(formObj) {

  formURL = formObj.attr('action')

  // animation actions
  formObj.addClass('sending')

  const formData = formObj.serialize()

  $.ajax({
    url: formURL,
    type: 'GET',
    data: formData,
    dataType: 'jsonp',
    jsonp: 'c',
    contentType: 'application/json; charset=utf-8',

    success(data) {
      formSent = true
      console.log(data.result)
      setTimeout(() => {
        formObj.removeClass('sending').addClass('thanks')
        TweenMax.to(formObj.find('input[name="EMAIL"]'), 0.3, {opacity: 0,
          onCompleteParams: [formObj.find('input[name="EMAIL"]')],
          onComplete(t) {
            t.val('We will be in contact shortly!')
            TweenMax.to(t, 0.3, {opacity: 1})
            TweenMax.to(t, 0.3, {delay: 2, opacity: 0})
            TweenMax.to(t, 0.3, {delay: 2.3,
              opacity: 1,
              onStart() {
                t.val('')
                formObj.removeClass('thanks')
              }})
          }})
        // TweenMax.to('.thanks', .5, {'opacity':1, 'display':'block'})
      }, 1500)
    }
  })

}

function validateForm(formObj) {
  let vNum = 0
  $(formObj).find('[data-type="req"]').each(function() {
    if ($(this).val() == '') {
      vNum++
    }
  })
  if (vNum == 0) {
    return true
  }
  formObj.find('.email-wrap').addClass('error')
  return false

}

$('input').focus(function() {
  $(this).parents('.email-wrap').removeClass('error')
  formSent = false
})

