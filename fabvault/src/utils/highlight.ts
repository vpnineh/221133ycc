/** Tiny dependency-free syntax highlighters (JSON / XML / INI) → escaped HTML with `tk-*` spans. */
import type { FileFormat } from '@/types/vault';
import { escapeHtml } from './format';

const span = (cls: string, text: string): string => `<span class="tk-${cls}">${escapeHtml(text)}</span>`;

function json(src: string): string {
  const re = /("(?:\\.|[^"\\])*")(\s*:)?|(-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)|\b(true|false|null)\b|([{}[\],])/g;
  let out = '';
  let last = 0;
  for (let m = re.exec(src); m; m = re.exec(src)) {
    out += escapeHtml(src.slice(last, m.index));
    if (m[1] !== undefined) out += m[2] ? span('key', m[1]) + escapeHtml(m[2]) : span('str', m[1]);
    else if (m[3] !== undefined) out += span('num', m[3]);
    else if (m[4] !== undefined) out += span('num', m[4]);
    else out += span('punc', m[5] ?? '');
    last = re.lastIndex;
  }
  return out + escapeHtml(src.slice(last));
}

function xml(src: string): string {
  const re = /(<!--[\s\S]*?-->)|(<\?[\s\S]*?\?>)|(<\/?)([\w:-]+)((?:\s+[\w:-]+="[^"]*")*)(\s*\/?>)/g;
  let out = '';
  let last = 0;
  for (let m = re.exec(src); m; m = re.exec(src)) {
    out += escapeHtml(src.slice(last, m.index));
    if (m[1] !== undefined) out += span('com', m[1]);
    else if (m[2] !== undefined) out += span('com', m[2]);
    else {
      const attrs = (m[5] ?? '').replace(
        /(\s+)([\w:-]+)(=)("[^"]*")/g,
        (_all, ws: string, name: string, eq: string, value: string) => `${ws}${span('key', name)}${eq}${span('str', value)}`,
      );
      out += span('punc', m[3] ?? '') + span('tag', m[4] ?? '') + attrs + span('punc', m[6] ?? '');
    }
    last = re.lastIndex;
  }
  return out + escapeHtml(src.slice(last));
}

function ini(src: string): string {
  return src
    .split('\n')
    .map((line) => {
      if (line.startsWith('#')) return span('com', line);
      const m = /^(\S+)(\s*=\s*)(.*)$/.exec(line);
      if (!m) return escapeHtml(line);
      const value = m[3] ?? '';
      return span('key', m[1] ?? '') + escapeHtml(m[2] ?? '') + span(/^-?\d+(\.\d+)?$/.test(value) ? 'num' : 'str', value);
    })
    .join('\n');
}

export function highlight(source: string, format: FileFormat): string {
  switch (format) {
    case 'json': return json(source);
    case 'clb':
    case 'svg': return xml(source);
    case 'cfg': return ini(source);
  }
}
