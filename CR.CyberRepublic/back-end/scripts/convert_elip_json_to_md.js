// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
// node scripts/convert_elip_json_to_md.js

require('../dist/src/config')

const types = {
  'unstyled': '',
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
    return rs.blocks.map(block => {
      let line = block.text
      const strs = {}
      if (Object.keys(types).includes(block.type)) {
        // deal with inline styles
        if (block.inlineStyleRanges.length) {
          for (const el of block.inlineStyleRanges) {
            const start = el.offset
            const end = start + el.length

            if (el.style === 'HIGHLIGHT') {
              const highlight = line.slice(start, end)
              strs[highlight] = `==${highlight}==`
            }

            if (el.style === 'BOLD') {
              const bold = line.slice(start, end)
              strs[bold] = `**${bold}**`
            }

            if (el.style === 'ITALIC') {
              const italic = line.slice(start, end)
              strs[italic] = `_${italic}_`
            }

            if (el.style === 'UNDERLINE') {
              const underline = line.slice(start, end)
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
              const url = rs.entityMap[el.key].data.url
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
        line = `![${block.text}](${block.data.src})\n`
      }
      // deal with todo list
      if (block.type === 'todo') {
        if (block.data.checked === true) {
          line = `- [ ] (${block.text})`
        } else {
          line = `- [x] (${block.text})`
        }
      }
      return line
    }).join('\n')
  } catch (err) {
    throw err
  }
}

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_elip = DB.getModel('Elip')
  try {
    let elips = await db_elip.find()
    for (const elip of elips) {
      try {
        const md = convertJsonToMd(elip.description)
        if (md) {
          await db_elip.update(
            { _id: elip._id },
            { $set: { description: md } }
          )
        }
      } catch (err) {
        console.log(`elip ${elip._id} error`, err)
      }
    }
  } catch (err) {
    console.error(err)
    process.exit(1)
  }
  console.log('done!')
  process.exit(1)
})()
