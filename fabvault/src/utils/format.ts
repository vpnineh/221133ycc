/** Locale-neutral numeric formatting shared by the server render and the client island. */

export function round(value: number, decimals: number): number {
  const f = 10 ** decimals;
  return Math.round((value + Number.EPSILON) * f) / f;
}

export function clamp(value: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, value));
}

/** Fixed decimals without trailing zeros beyond what the parameter needs (`0.80` → `0.8`, `220.0` → `220`). */
export function fmt(value: number, decimals: number): string {
  const fixed = round(value, decimals).toFixed(decimals);
  return decimals > 0 ? fixed.replace(/\.?0+$/, '') || '0' : fixed;
}

export function fmtUnit(value: number, decimals: number, unit: string): string {
  const n = fmt(value, decimals);
  if (!unit) return n;
  return unit === '%' || unit === '°C' ? `${n}${unit === '%' ? '%' : ' °C'}` : `${n} ${unit}`;
}

export function signed(delta: number, decimals: number): string {
  const r = round(delta, decimals);
  if (r === 0) return '±0';
  return `${r > 0 ? '+' : '−'}${fmt(Math.abs(r), decimals)}`;
}

/** `{name}` interpolation for dictionary templates. Unknown keys are left visible so verification can catch them. */
export function tpl(template: string, vars: Record<string, string | number>): string {
  return template.replace(/\{(\w+)\}/g, (match, key: string) => {
    const v = vars[key];
    return v === undefined ? match : String(v);
  });
}

export function slugify(input: string): string {
  return input
    .toLowerCase()
    .normalize('NFKD')
    .replace(/[^\w\s.-]/g, '')
    .replace(/[\s._]+/g, '-')
    .replace(/-+/g, '-')
    .replace(/^-|-$/g, '');
}

/** FNV-1a 32-bit — deterministic revision hash, identical in Node and the browser. */
export function fnv1a(input: string): string {
  let h = 0x811c9dc5;
  for (let i = 0; i < input.length; i++) {
    h ^= input.charCodeAt(i);
    h = Math.imul(h, 0x01000193);
  }
  return (h >>> 0).toString(16).padStart(8, '0');
}

export function escapeHtml(input: string): string {
  return input
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

export function escapeXml(input: string): string {
  return escapeHtml(input).replace(/'/g, '&apos;');
}
