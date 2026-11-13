/** Shared gauge geometry — identical maths on the server render and in the island. */
import type { GaugeModel } from '@/types/vault';
import { clamp } from './format';

/** Radial gauges sweep 270°, starting bottom-left (−135° from 12 o'clock). */
export const SWEEP = 270;
export const START_ANGLE = -135;

export interface GaugeGeometry {
  /** 0..1 positions along the scale. */
  value: number;
  safeStart: number;
  safeEnd: number;
  /** Needle rotation in degrees (0 = 12 o'clock). */
  needle: number;
  /** stroke-dasharray / offset for the safe-window arc on a path with pathLength = 100. */
  dashArray: string;
  dashOffset: string;
}

export function gaugeGeometry(g: GaugeModel): GaugeGeometry {
  const span = g.scaleMax - g.scaleMin || 1;
  const pos = (v: number): number => clamp((v - g.scaleMin) / span, 0, 1);
  const value = pos(g.value);
  const safeStart = pos(Math.min(g.safeMin, g.safeMax));
  const safeEnd = pos(Math.max(g.safeMin, g.safeMax));
  const len = Math.max(0.6, (safeEnd - safeStart) * 100);
  return {
    value,
    safeStart,
    safeEnd,
    needle: START_ANGLE + value * SWEEP,
    dashArray: `${len.toFixed(2)} 200`,
    dashOffset: (-safeStart * 100).toFixed(2),
  };
}

/** SVG path for the 270° arc of radius `r` centred on (cx, cy). */
export function arcPath(cx: number, cy: number, r: number): string {
  const pt = (deg: number): string => {
    const rad = ((deg - 90) * Math.PI) / 180;
    return `${(cx + r * Math.cos(rad)).toFixed(3)} ${(cy + r * Math.sin(rad)).toFixed(3)}`;
  };
  return `M${pt(START_ANGLE)}A${r} ${r} 0 1 1 ${pt(START_ANGLE + SWEEP)}`;
}

/** Tick marks around the dial: `count` intervals, every `majorEvery`-th one long. */
export function dialTicks(cx: number, cy: number, rOuter: number, count: number, majorEvery: number): string {
  let d = '';
  for (let i = 0; i <= count; i++) {
    const deg = START_ANGLE + (SWEEP * i) / count;
    const rad = ((deg - 90) * Math.PI) / 180;
    const len = i % majorEvery === 0 ? 5 : 2.5;
    const x1 = cx + rOuter * Math.cos(rad);
    const y1 = cy + rOuter * Math.sin(rad);
    const x2 = cx + (rOuter - len) * Math.cos(rad);
    const y2 = cy + (rOuter - len) * Math.sin(rad);
    d += `M${x1.toFixed(2)} ${y1.toFixed(2)}L${x2.toFixed(2)} ${y2.toFixed(2)}`;
  }
  return d;
}
