const types = {
  unstyled: '',
  'header-one': '# ',
  'header-two': '## ',
  'header-three': '### ',
  'header-four': '#### ',
  'header-five': '##### ',
  'header-six': '###### ',
  'unordered-list-item': '* ',
  'ordered-list-item': '1. '
}

function convertJsonToMd(data) {
  try {
    const rs = JSON.parse(data)
    if (!rs) {
      return
    }
    return rs.blocks
      .map(block => {
        let line = block.text
        const strs = {}
        if (Object.keys(types).includes(block.type)) {
          // deal with inline styles
          if (block.inlineStyleRanges.length) {
            for (const el of block.inlineStyleRanges) {
              const start = el.offset
              const end = start + el.length

              if (el.style === 'HIGHLIGHT') {
                const highlight = line.slice(start, end).trim()
                strs[highlight] = `==${highlight}==`
              }

              if (el.style === 'BOLD') {
                const bold = line.slice(start, end).trim()
                strs[bold] = `**${bold}**`
              }

              if (el.style === 'ITALIC') {
                const italic = line.slice(start, end).trim()
                strs[italic] = `_${italic}_`
              }

              if (el.style === 'UNDERLINE') {
                const underline = line.slice(start, end).trim()
                strs[underline] = `++${underline}++`
              }
            }
          }
          // deal with links
          if (block.entityRanges.length) {
            for (const el of block.entityRanges) {
              const start = el.offset
              const end = start + el.length
              if (rs.entityMap[el.key].type === 'LINK') {
                const url = rs.entityMap[el.key].data.url.trim()
                const link = line.slice(start, end)
                strs[link] = `[${link}](${url})`
              }
            }
          }
          // replace matched string with markdown text
          for (const key of Object.keys(strs)) {
            line = line.replace(key, strs[key])
          }

          line = `${types[block.type]}${line}\n`
        }
        // deal with hr
        if (block.type === 'atomic' && block.text === '-') {
          line = '---\n'
        }
        // deal with blockquote
        if (block.type === 'blockquote') {
          line = `> ${line}`
        }
        // deal with base64 image
        if (block.type === 'atomic:image') {
          const caption = block.text && block.text.trim() ? `_${block.text.trim()}_` : ''
          line = `![](${block.data.src})\n${caption}`
        }
        // deal with todo list
        if (block.type === 'todo') {
          if (block.data.checked === true) {
            line = `- [ ] (${block.text.trim()})`
          } else {
            line = `- [x] (${block.text.trim()})`
          }
        }
        return line
      })
      .join('\n')
  } catch (err) {
    // console.log(err)
  }
}

module.exports = convertJsonToMd
