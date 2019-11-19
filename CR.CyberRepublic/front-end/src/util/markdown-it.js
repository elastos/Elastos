import DOMPurify from 'dompurify'
import markdownIt from 'markdown-it'
import markdownItMermaid from '@liradb2000/markdown-it-mermaid'
import taskLists from 'markdown-it-task-lists'
import sub from 'markdown-it-sub'
import sup from 'markdown-it-sup'
import footnote from 'markdown-it-footnote'
import abbr from 'markdown-it-abbr'
import emoji from 'markdown-it-emoji'
import mark from 'markdown-it-mark'
import deflist from 'markdown-it-deflist'
import ins from 'markdown-it-ins'
import { getSiteUrl } from './url'

const mdi = markdownIt({
  html: true,
  breaks: true,
  linkify: true, // Autoconvert URL-like text to links
  typographer: true // Enable some language-neutral replacement + quotes beautification
})
  .use(markdownItMermaid)
  .use(taskLists)
  .use(footnote)
  .use(sub)
  .use(sup)
  .use(abbr)
  .use(emoji)
  .use(mark)
  .use(ins)
  .use(deflist)

const autolinkReferenceNumber = content => {
  const url = getSiteUrl()
  const patterns = [
    {
      regExp: /(elip|proposal|suggestion)\s+#([1-9]{1}\d*)/gi,
      subs: `$1 [#$2](${url}/$1s/$2)`
    },
    {
      regExp: /(提案)\s+#([1-9]{1}\d*)/gi,
      subs: `$1 [#$2](${url}/proposals/$2)`
    },
    {
      regExp: /(建议)\s+#([1-9]{1}\d*)/gi,
      subs: `$1 [#$2](${url}/suggestions/$2)`
    },
    {
      regExp: /#([1-9]{1}\d*)\s+(elip|proposal|suggestion)/gi,
      subs: `[#$1](${url}/$2s/$1) $2`
    },
    {
      regExp: /#([1-9]{1}\d*)\s+(提案)/gi,
      subs: `[#$1](${url}/proposals/$1) $2`
    },
    {
      regExp: /#([1-9]{1}\d*)\s+(建议)/gi,
      subs: `[#$1](${url}/suggestion/$1) $2`
    }
  ]
  return patterns.reduce((rs, item) => {
    return rs.replace(item.regExp, item.subs)
  }, content)
}

export const convertMarkdownToHtml = content => {
  const rs = content && autolinkReferenceNumber(content)
  return DOMPurify.sanitize(mdi.render(rs || ''))
}
