/**
 * Compensation engine.
 *
 * Pure, dependency-free functions that turn a seed profile plus the visitor's
 * hardware toggles into a full parameter set. The same code runs at build time
 * (static HTML, JSON-LD) and in the browser island (live gauges, preview and
 * download), so the page can never disagree with the file it hands out.
 */
import type {
  ComputedProfile, ComputedRow, ConfigGroup, ConfigState, FilamentProfile, GaugeModel,
  LaserProfile, NoteKey, ParamKey, PlotterProfile, Range, ResinProfile, Tone, VaultProfile,
} from '@/types/vault';
import { clamp, round } from './format';

// ───────────────────────────── Option sets ─────────────────────────────

const LASER_POWER_OPTIONS: Record<LaserProfile['laser']['type'], readonly number[]> = {
  diode: [5, 10, 20, 40],
  co2: [40, 55, 80, 100],
  fiber: [20, 30, 50, 60],
};

export function getConfigGroups(profile: VaultProfile): ConfigGroup[] {
  switch (profile.vertical) {
    case 'filament':
      return [
        {
          id: 'nozzle',
          defaultValue: '0.4',
          options: ['0.2', '0.4', '0.6', '0.8'].map((v) => ({ value: v, label: `${v} mm` })),
        },
        {
          id: 'hotend',
          defaultValue: profile.abrasive ? 'hardened' : 'brass',
          options: [
            { value: 'brass', labelKey: 'brass' },
            { value: 'hardened', labelKey: 'hardened' },
            { value: 'highflow', labelKey: 'highflow' },
          ],
        },
        {
          id: 'chamber',
          defaultValue:
            profile.chamber.requirement === 'required' || profile.chamber.requirement === 'recommended'
              ? 'enclosed'
              : 'open',
          options: [
            { value: 'open', labelKey: 'open' },
            { value: 'enclosed', labelKey: 'enclosed' },
          ],
        },
      ];
    case 'laser': {
      const set = new Set<number>(LASER_POWER_OPTIONS[profile.laser.type]);
      set.add(profile.laser.opticalPowerW);
      const options = [...set].sort((a, b) => a - b);
      return [
        {
          id: 'power',
          defaultValue: String(profile.laser.opticalPowerW),
          options: options.map((w) => ({ value: String(w), label: `${w} W` })),
        },
        {
          id: 'air',
          defaultValue: profile.airAssistRecommended ? 'on' : 'off',
          options: [
            { value: 'on', labelKey: 'on' },
            { value: 'off', labelKey: 'off' },
          ],
        },
      ];
    }
    case 'resin': {
      const set = new Set<string>(['0.03', '0.05', '0.10']);
      set.add(profile.baseLayerHeightMm.toFixed(2));
      return [
        {
          id: 'layer',
          defaultValue: profile.baseLayerHeightMm.toFixed(2),
          options: [...set].sort().map((v) => ({ value: v, label: `${v} mm` })),
        },
        {
          id: 'ambient',
          defaultValue: '25',
          options: ['20', '25', '30'].map((v) => ({ value: v, label: `${v} °C` })),
        },
      ];
    }
    case 'plotter':
      return [
        {
          id: 'tool',
          defaultValue: 'new',
          options: [
            { value: 'new', labelKey: 'toolNew' },
            { value: 'used', labelKey: 'toolUsed' },
            { value: 'worn', labelKey: 'toolWorn' },
          ],
        },
        {
          id: 'detail',
          defaultValue: 'standard',
          options: [
            { value: 'bold', labelKey: 'detailBold' },
            { value: 'standard', labelKey: 'detailStandard' },
            { value: 'intricate', labelKey: 'detailIntricate' },
          ],
        },
      ];
  }
}

export function defaultState(profile: VaultProfile): ConfigState {
  const state: ConfigState = {};
  for (const g of getConfigGroups(profile)) state[g.id] = g.defaultValue;
  return state;
}

/** Drops unknown groups/values so a hand-edited URL hash can never produce NaN parameters. */
export function sanitizeState(profile: VaultProfile, input: ConfigState): ConfigState {
  const state: ConfigState = {};
  for (const g of getConfigGroups(profile)) {
    const candidate = input[g.id];
    state[g.id] = candidate && g.options.some((o) => o.value === candidate) ? candidate : g.defaultValue;
  }
  return state;
}

// ───────────────────────────── Internals ─────────────────────────────

interface Draft {
  values: Partial<Record<ParamKey, number>>;
  notes: NoteKey[];
  gauges: Omit<GaugeModel, 'value'>[];
}

interface RowSpec {
  key: ParamKey;
  unit: string;
  decimals: number;
  tone: Tone;
}

function shifted(range: Range, delta: number, factor = 1): { min: number; max: number } {
  return { min: range.min * factor + delta, max: range.max * factor + delta };
}

// ───────────────────────────── Filament ─────────────────────────────

const NOZZLE: Record<string, { flow: number; temp: number; pa: number; retract: number }> = {
  '0.2': { flow: 0.3, temp: 0, pa: 2.0, retract: -0.2 },
  '0.4': { flow: 1, temp: 0, pa: 1, retract: 0 },
  '0.6': { flow: 1.35, temp: 5, pa: 0.6, retract: 0.2 },
  '0.8': { flow: 1.6, temp: 10, pa: 0.4, retract: 0.4 },
};
const HOTEND: Record<string, { flow: number; temp: number; pa: number }> = {
  brass: { flow: 1, temp: 0, pa: 1 },
  hardened: { flow: 0.92, temp: 5, pa: 1 },
  highflow: { flow: 1.5, temp: 5, pa: 1.1 },
};

const FILAMENT_ROWS: RowSpec[] = [
  { key: 'nozzleTemp', unit: '°C', decimals: 0, tone: 'heat' },
  { key: 'nozzleTempInitial', unit: '°C', decimals: 0, tone: 'heat' },
  { key: 'bedTemp', unit: '°C', decimals: 0, tone: 'heat' },
  { key: 'chamberTemp', unit: '°C', decimals: 0, tone: 'heat' },
  { key: 'maxFlow', unit: 'mm³/s', decimals: 1, tone: 'flow' },
  { key: 'lineWidth', unit: 'mm', decimals: 2, tone: 'flow' },
  { key: 'layerHeight', unit: 'mm', decimals: 2, tone: 'flow' },
  { key: 'maxSpeed', unit: 'mm/s', decimals: 0, tone: 'flow' },
  { key: 'retractLength', unit: 'mm', decimals: 1, tone: 'flow' },
  { key: 'retractSpeed', unit: 'mm/s', decimals: 0, tone: 'flow' },
  { key: 'pressureAdvance', unit: '', decimals: 3, tone: 'flow' },
  { key: 'fanMin', unit: '%', decimals: 0, tone: 'flow' },
  { key: 'fanMax', unit: '%', decimals: 0, tone: 'flow' },
  { key: 'flowRatio', unit: '', decimals: 2, tone: 'neutral' },
];

function filament(p: FilamentProfile, s: ConfigState): Draft {
  const nozzleKey = s.nozzle ?? '0.4';
  const nz = NOZZLE[nozzleKey] ?? NOZZLE['0.4']!;
  const he = HOTEND[s.hotend ?? 'brass'] ?? HOTEND['brass']!;
  const enclosed = s.chamber === 'enclosed';
  const d = Number(nozzleKey);
  const notes: NoteKey[] = [];

  const tempDelta = nz.temp + he.temp;
  const nozzleTemp = clamp(p.nozzleTempC.rec + tempDelta, p.nozzleTempC.min, p.limits.nozzleMaxC);
  const nozzleWindow = shifted(p.nozzleTempC, tempDelta);

  let bedDelta = 0;
  let fanMin = p.fanPct.min;
  let fanMax = p.fanPct.max;
  let chamberTemp = 0;
  const wantsHeat = p.chamber.requirement === 'required' || p.chamber.requirement === 'recommended';
  if (enclosed) {
    chamberTemp = p.chamber.tempC;
    if (p.chamber.requirement === 'avoid') {
      // Low-Tg polymers heat-creep in a closed chamber: cool harder, drop the bed.
      bedDelta = -5;
      fanMin = clamp(fanMin + 15, 0, 100);
      chamberTemp = 0;
      notes.push('enclosedLowTg');
    }
  } else if (wantsHeat) {
    // No chamber for a shrink-prone polymer: keep the part warm from below and slow the fan.
    bedDelta = 5;
    fanMax = round(fanMax * 0.5, 0);
    fanMin = Math.min(fanMin, fanMax);
    notes.push('openHighTemp');
  }

  const maxFlow = round(Math.min(p.maxVolumetricSpeed * nz.flow * he.flow, p.limits.flowMax), 1);
  const lineWidth = round(d * 1.05, 2);
  const layerHeight = round(d * 0.5, 2);
  const maxSpeed = Math.floor(maxFlow / (lineWidth * layerHeight));

  if (d < 0.4) notes.push('nozzleSmall');
  if (d > 0.4) notes.push('nozzleLarge');
  if (s.hotend === 'hardened') notes.push('hardenedSteel');
  if (s.hotend === 'highflow') notes.push('highFlow');
  if (p.abrasive && s.hotend !== 'hardened') notes.push('abrasiveOnBrass');

  const bedTemp = clamp(p.bedTempC.rec + bedDelta, 0, p.limits.bedMaxC);
  const bedWindow = shifted(p.bedTempC, bedDelta);

  return {
    notes,
    values: {
      nozzleTemp,
      nozzleTempInitial: clamp(nozzleTemp + 5, p.nozzleTempC.min, p.limits.nozzleMaxC),
      bedTemp,
      chamberTemp,
      maxFlow,
      lineWidth,
      layerHeight,
      maxSpeed,
      retractLength: round(Math.max(0.2, p.retraction.lengthMm + nz.retract), 1),
      retractSpeed: p.retraction.speedMmS,
      pressureAdvance: round(p.pressureAdvance * nz.pa * he.pa, 3),
      fanMin,
      fanMax,
      flowRatio: p.flowRatio,
      dryTemp: p.drying.tempC,
      dryHours: p.drying.hours,
      density: p.densityGcm3,
    },
    gauges: [
      { key: 'nozzleTemp', kind: 'radial', safeMin: nozzleWindow.min, safeMax: Math.min(nozzleWindow.max, p.limits.nozzleMaxC), scaleMin: 150, scaleMax: p.limits.nozzleMaxC, unit: '°C', decimals: 0, tone: 'heat' },
      { key: 'maxFlow', kind: 'radial', safeMin: round(maxFlow * 0.5, 1), safeMax: maxFlow, scaleMin: 0, scaleMax: p.limits.flowMax, unit: 'mm³/s', decimals: 1, tone: 'flow' },
      { key: 'bedTemp', kind: 'bar', safeMin: bedWindow.min, safeMax: Math.min(bedWindow.max, p.limits.bedMaxC), scaleMin: 0, scaleMax: p.limits.bedMaxC, unit: '°C', decimals: 0, tone: 'heat' },
      { key: 'fanMax', kind: 'bar', safeMin: fanMin, safeMax: fanMax, scaleMin: 0, scaleMax: 100, unit: '%', decimals: 0, tone: 'flow' },
    ],
  };
}

// ───────────────────────────── Laser ─────────────────────────────

const LASER_ROWS: RowSpec[] = [
  { key: 'opticalPower', unit: 'W', decimals: 0, tone: 'heat' },
  { key: 'cutPower', unit: '%', decimals: 0, tone: 'heat' },
  { key: 'cutSpeed', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'cutPasses', unit: '×', decimals: 0, tone: 'neutral' },
  { key: 'lineEnergy', unit: 'J/mm', decimals: 2, tone: 'heat' },
  { key: 'kerf', unit: 'mm', decimals: 2, tone: 'neutral' },
  { key: 'engravePower', unit: '%', decimals: 0, tone: 'heat' },
  { key: 'engraveSpeed', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'engraveInterval', unit: 'mm', decimals: 2, tone: 'neutral' },
  { key: 'focusOffset', unit: 'mm', decimals: 1, tone: 'neutral' },
];

/** Speeds snap to a sensible machine increment so the preset reads like something a person dialled in. */
function snapSpeed(v: number): number {
  const step = v >= 3000 ? 100 : v >= 600 ? 50 : v >= 150 ? 10 : 5;
  return Math.max(step, Math.round(v / step) * step);
}

function laser(p: LaserProfile, s: ConfigState): Draft {
  const watts = Number(s.power ?? p.laser.opticalPowerW) || p.laser.opticalPowerW;
  const r = watts / p.laser.opticalPowerW;
  const air = s.air === 'on';
  const notes: NoteKey[] = [];
  const vMax = p.laser.maxSpeedMmMin;

  // Keep the delivered energy per millimetre constant: E ∝ P · n / v.
  let passes = p.cut.passes;
  if (r < 0.6) {
    passes = Math.ceil((p.cut.passes * 0.6) / r);
    notes.push('laserUnderpowered');
  }
  const airFactor = air ? 1 : 0.85;
  let cutSpeedRaw = p.cut.speedMmMin.rec * r * (passes / p.cut.passes) * airFactor;
  let cutPower = p.cut.powerPct.rec;
  if (cutSpeedRaw > vMax) {
    cutPower = Math.max(10, Math.round((cutPower * vMax) / cutSpeedRaw));
    cutSpeedRaw = vMax;
    notes.push('laserSpeedCapped');
  }
  const cutSpeed = snapSpeed(cutSpeedRaw);

  let engraveSpeedRaw = p.engrave.speedMmMin.rec * r;
  let engravePower = p.engrave.powerPct.rec;
  if (engraveSpeedRaw > vMax) {
    engravePower = Math.max(5, Math.round((engravePower * vMax) / engraveSpeedRaw));
    engraveSpeedRaw = vMax;
    if (!notes.includes('laserSpeedCapped')) notes.push('laserSpeedCapped');
  }
  const engraveSpeed = snapSpeed(engraveSpeedRaw);

  if (!air) {
    notes.push('airOffChar');
    if (p.hazards.flame >= 3) notes.push('airOffFlame');
  }

  const speedFactor = cutSpeed / p.cut.speedMmMin.rec;
  const lineEnergy = round(((watts * cutPower) / 100 / (cutSpeed / 60)) * passes, 2);

  return {
    notes,
    values: {
      opticalPower: watts,
      thickness: p.thicknessMm,
      cutPower,
      cutSpeed,
      cutPasses: passes,
      lineEnergy,
      kerf: round(p.kerfMm + (air ? 0 : 0.03), 2),
      engravePower,
      engraveSpeed,
      engraveInterval: p.engrave.intervalMm,
      focusOffset: p.focusOffsetMm,
    },
    gauges: [
      { key: 'cutPower', kind: 'radial', safeMin: Math.min(p.cut.powerPct.min, cutPower), safeMax: p.cut.powerPct.max, scaleMin: 0, scaleMax: 100, unit: '%', decimals: 0, tone: 'heat' },
      { key: 'cutSpeed', kind: 'radial', safeMin: Math.min(snapSpeed(p.cut.speedMmMin.min * speedFactor), cutSpeed), safeMax: Math.min(Math.max(snapSpeed(p.cut.speedMmMin.max * speedFactor), cutSpeed), vMax), scaleMin: 0, scaleMax: scaleFor(Math.max(p.cut.speedMmMin.max * speedFactor, cutSpeed)), unit: 'mm/min', decimals: 0, tone: 'flow' },
      { key: 'engravePower', kind: 'bar', safeMin: Math.min(p.engrave.powerPct.min, engravePower), safeMax: p.engrave.powerPct.max, scaleMin: 0, scaleMax: 100, unit: '%', decimals: 0, tone: 'heat' },
      { key: 'engraveSpeed', kind: 'bar', safeMin: Math.min(snapSpeed(p.engrave.speedMmMin.min * r), engraveSpeed), safeMax: Math.min(Math.max(snapSpeed(p.engrave.speedMmMin.max * r), engraveSpeed), vMax), scaleMin: 0, scaleMax: vMax, unit: 'mm/min', decimals: 0, tone: 'flow' },
    ],
  };
}

/** Rounds a gauge full-scale value up to a readable number (1 / 2 / 2.5 / 5 × 10ⁿ). */
function scaleFor(maxValue: number): number {
  const target = maxValue * 1.5;
  const mag = 10 ** Math.floor(Math.log10(target));
  for (const m of [1, 2, 2.5, 5, 10]) if (m * mag >= target) return m * mag;
  return 10 * mag;
}

// ───────────────────────────── Resin ─────────────────────────────

const RESIN_ROWS: RowSpec[] = [
  { key: 'layerHeight', unit: 'mm', decimals: 2, tone: 'neutral' },
  { key: 'normalExposure', unit: 's', decimals: 1, tone: 'heat' },
  { key: 'bottomExposure', unit: 's', decimals: 0, tone: 'heat' },
  { key: 'bottomLayers', unit: '×', decimals: 0, tone: 'neutral' },
  { key: 'transitionLayers', unit: '×', decimals: 0, tone: 'neutral' },
  { key: 'liftDistance', unit: 'mm', decimals: 1, tone: 'flow' },
  { key: 'liftSpeed', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'zRetractSpeed', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'lightOffDelay', unit: 's', decimals: 1, tone: 'flow' },
  { key: 'restAfterRetract', unit: 's', decimals: 1, tone: 'flow' },
  { key: 'printRate', unit: 'mm/h', decimals: 1, tone: 'flow' },
];

const AMBIENT: Record<string, { exposure: number; lift: number; rest: number }> = {
  '20': { exposure: 1.15, lift: 0.75, rest: 0.5 },
  '25': { exposure: 1, lift: 1, rest: 0 },
  '30': { exposure: 0.92, lift: 1.1, rest: 0 },
};

function resin(p: ResinProfile, s: ConfigState): Draft {
  const h = Number(s.layer ?? p.baseLayerHeightMm) || p.baseLayerHeightMm;
  const amb = AMBIENT[s.ambient ?? '25'] ?? AMBIENT['25']!;
  const notes: NoteKey[] = [];

  // Working-curve rescale (Jacobs): cure depth Cd = Dp · ln(E / Ec)  ⇒  E₂ = E₁ · exp((h₂ − h₁) / Dp).
  const layerFactor = Math.exp((h - p.baseLayerHeightMm) / p.resin.penetrationDepthMm);
  const exposureFactor = layerFactor * amb.exposure;
  const normalExposure = round(p.normalExposureS.rec * exposureFactor, 1);
  const bottomExposure = round(p.bottomExposureS.rec * amb.exposure * Math.sqrt(layerFactor), 0);
  // Hold the burn-in zone thickness constant when the layer height changes.
  const bottomLayers = Math.max(3, Math.round((p.bottomLayers * p.baseLayerHeightMm) / h));
  const transitionLayers = Math.max(2, Math.round((p.transitionLayers * p.baseLayerHeightMm) / h));
  const liftSpeed = Math.min(Math.round((p.lift.speedMmMin.rec * amb.lift) / 5) * 5, p.printer.maxLiftSpeedMmMin);
  const retract = Math.min(Math.round((p.retractSpeedMmMin * amb.lift) / 10) * 10, p.printer.maxLiftSpeedMmMin * 2);
  const lightOffDelay = round(p.lightOffDelayS + amb.rest, 1);
  const rest = round(p.restAfterRetractS + amb.rest, 1);

  const layerSeconds =
    normalExposure + lightOffDelay + rest + (p.lift.distanceMm / liftSpeed) * 60 + (p.lift.distanceMm / retract) * 60;
  const printRate = round((h / layerSeconds) * 3600, 1);

  if (h < p.baseLayerHeightMm) notes.push('thinLayers');
  if (h > p.baseLayerHeightMm) notes.push('thickLayers');
  if (s.ambient === '20') notes.push('coldResin');
  if (s.ambient === '30') notes.push('warmResin');

  const expWindow = shifted(p.normalExposureS, 0, exposureFactor);
  const liftWindow = shifted(p.lift.speedMmMin, 0, amb.lift);
  const botWindow = shifted(p.bottomExposureS, 0, amb.exposure * Math.sqrt(layerFactor));

  return {
    notes,
    values: {
      layerHeight: h,
      normalExposure,
      bottomExposure,
      bottomLayers,
      transitionLayers,
      liftDistance: p.lift.distanceMm,
      liftSpeed,
      zRetractSpeed: retract,
      lightOffDelay,
      restAfterRetract: rest,
      printRate,
      washTime: p.post.washMin,
      cureTime: p.post.cureMin,
      xyResolution: p.printer.xyResolutionUm,
    },
    gauges: [
      { key: 'normalExposure', kind: 'radial', safeMin: round(expWindow.min, 1), safeMax: round(expWindow.max, 1), scaleMin: 0, scaleMax: Math.max(8, Math.ceil(p.normalExposureS.max * 2.4)), unit: 's', decimals: 1, tone: 'heat' },
      { key: 'liftSpeed', kind: 'radial', safeMin: Math.round(liftWindow.min), safeMax: Math.min(Math.round(liftWindow.max), p.printer.maxLiftSpeedMmMin), scaleMin: 0, scaleMax: p.printer.maxLiftSpeedMmMin, unit: 'mm/min', decimals: 0, tone: 'flow' },
      { key: 'bottomExposure', kind: 'bar', safeMin: Math.round(botWindow.min), safeMax: Math.round(botWindow.max), scaleMin: 0, scaleMax: Math.max(60, Math.ceil((p.bottomExposureS.max * 2) / 10) * 10), unit: 's', decimals: 0, tone: 'heat' },
      { key: 'lightOffDelay', kind: 'bar', safeMin: Math.max(0, lightOffDelay - 0.5), safeMax: lightOffDelay + 1, scaleMin: 0, scaleMax: 5, unit: 's', decimals: 1, tone: 'flow' },
    ],
  };
}

// ───────────────────────────── Plotter & CNC ─────────────────────────────

const KNIFE_ROWS: RowSpec[] = [
  { key: 'force', unit: 'gf', decimals: 0, tone: 'heat' },
  { key: 'toolSpeed', unit: 'mm/s', decimals: 0, tone: 'flow' },
  { key: 'bladeDepth', unit: 'mm', decimals: 2, tone: 'neutral' },
  { key: 'bladeSetting', unit: '', decimals: 0, tone: 'neutral' },
  { key: 'passes', unit: '×', decimals: 0, tone: 'neutral' },
];
const CNC_ROWS: RowSpec[] = [
  { key: 'spindleRpm', unit: 'rpm', decimals: 0, tone: 'heat' },
  { key: 'feedRate', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'plungeRate', unit: 'mm/min', decimals: 0, tone: 'flow' },
  { key: 'depthPerPass', unit: 'mm', decimals: 2, tone: 'neutral' },
  { key: 'stepover', unit: '%', decimals: 0, tone: 'neutral' },
  { key: 'chipLoad', unit: 'mm', decimals: 3, tone: 'flow' },
  { key: 'bitDiameter', unit: 'mm', decimals: 3, tone: 'neutral' },
];

const TOOL: Record<string, { force: number; feed: number }> = {
  new: { force: 1, feed: 1 },
  used: { force: 1.1, feed: 0.9 },
  worn: { force: 1.2, feed: 0.75 },
};
const DETAIL: Record<string, { speed: number; force: number }> = {
  bold: { speed: 1.2, force: 1 },
  standard: { speed: 1, force: 1 },
  intricate: { speed: 0.6, force: 0.95 },
};

function plotter(p: PlotterProfile, s: ConfigState): Draft {
  const tool = TOOL[s.tool ?? 'new'] ?? TOOL['new']!;
  const detail = DETAIL[s.detail ?? 'standard'] ?? DETAIL['standard']!;
  const notes: NoteKey[] = [];
  if (s.tool === 'worn') notes.push('toolWorn');
  if (s.detail === 'intricate') notes.push('detailIntricate');
  if (s.detail === 'bold') notes.push('detailBold');

  const proc = p.process;
  if (proc.kind === 'knife') {
    const forceFactor = tool.force * detail.force;
    const force = Math.min(Math.round((proc.forceG.rec * forceFactor) / 5) * 5, proc.limits.forceMaxG);
    const speed = Math.max(5, Math.min(Math.round((proc.speedMmS.rec * detail.speed) / 5) * 5, proc.limits.speedMaxMmS));
    const fw = shifted(proc.forceG, 0, forceFactor);
    const sw = shifted(proc.speedMmS, 0, detail.speed);
    return {
      notes,
      values: {
        force,
        toolSpeed: speed,
        bladeDepth: proc.bladeDepthMm,
        bladeSetting: proc.bladeSetting,
        passes: proc.passes + (s.tool === 'worn' && proc.cutType === 'through-cut' ? 1 : 0),
        thickness: p.thicknessMm,
      },
      gauges: [
        { key: 'force', kind: 'radial', safeMin: Math.round(fw.min), safeMax: Math.min(Math.round(fw.max), proc.limits.forceMaxG), scaleMin: 0, scaleMax: proc.limits.forceMaxG, unit: 'gf', decimals: 0, tone: 'heat' },
        { key: 'toolSpeed', kind: 'radial', safeMin: Math.round(sw.min), safeMax: Math.min(Math.round(sw.max), proc.limits.speedMaxMmS), scaleMin: 0, scaleMax: proc.limits.speedMaxMmS, unit: 'mm/s', decimals: 0, tone: 'flow' },
        { key: 'bladeDepth', kind: 'bar', safeMin: Math.max(0, p.thicknessMm * 0.9), safeMax: proc.bladeDepthMm * 1.15, scaleMin: 0, scaleMax: Math.max(1, round(proc.bladeDepthMm * 3, 1)), unit: 'mm', decimals: 2, tone: 'neutral' },
      ],
    };
  }

  const feedFactor = tool.feed * Math.min(detail.speed, 1.1);
  const feed = Math.min(Math.round((proc.feedMmMin.rec * feedFactor) / 10) * 10, proc.limits.feedMaxMmMin);
  const rpm = proc.spindleRpm.rec;
  const fw = shifted(proc.feedMmMin, 0, feedFactor);
  return {
    notes,
    values: {
      spindleRpm: rpm,
      feedRate: feed,
      plungeRate: Math.round((proc.plungeMmMin * tool.feed) / 10) * 10,
      depthPerPass: round(proc.depthPerPassMm * (s.tool === 'worn' ? 0.8 : 1), 2),
      stepover: s.detail === 'intricate' ? Math.min(proc.stepoverPct, 30) : proc.stepoverPct,
      chipLoad: round(feed / (rpm * proc.flutes), 3),
      bitDiameter: proc.bitDiameterMm,
      thickness: p.thicknessMm,
    },
    gauges: [
      { key: 'spindleRpm', kind: 'radial', safeMin: proc.spindleRpm.min, safeMax: proc.spindleRpm.max, scaleMin: 0, scaleMax: proc.limits.rpmMax, unit: 'rpm', decimals: 0, tone: 'heat' },
      { key: 'feedRate', kind: 'radial', safeMin: Math.round(fw.min), safeMax: Math.min(Math.round(fw.max), proc.limits.feedMaxMmMin), scaleMin: 0, scaleMax: proc.limits.feedMaxMmMin, unit: 'mm/min', decimals: 0, tone: 'flow' },
      { key: 'depthPerPass', kind: 'bar', safeMin: proc.depthPerPassMm * 0.5, safeMax: proc.depthPerPassMm * 1.1, scaleMin: 0, scaleMax: round(proc.bitDiameterMm, 2), unit: 'mm', decimals: 2, tone: 'neutral' },
    ],
  };
}

// ───────────────────────────── Public API ─────────────────────────────

function rowSpecs(profile: VaultProfile): RowSpec[] {
  switch (profile.vertical) {
    case 'filament': return FILAMENT_ROWS;
    case 'laser': return LASER_ROWS;
    case 'resin': return RESIN_ROWS;
    case 'plotter': return profile.process.kind === 'knife' ? KNIFE_ROWS : CNC_ROWS;
  }
}

function draft(profile: VaultProfile, state: ConfigState): Draft {
  switch (profile.vertical) {
    case 'filament': return filament(profile, state);
    case 'laser': return laser(profile, state);
    case 'resin': return resin(profile, state);
    case 'plotter': return plotter(profile, state);
  }
}

export function computeProfile(profile: VaultProfile, input?: ConfigState): ComputedProfile {
  const state = sanitizeState(profile, input ?? {});
  const current = draft(profile, state);
  const baseline = draft(profile, defaultState(profile));

  const rows: ComputedRow[] = rowSpecs(profile).map((spec) => ({
    ...spec,
    value: current.values[spec.key] ?? 0,
    base: baseline.values[spec.key] ?? 0,
  }));

  const gauges: GaugeModel[] = current.gauges.map((g) => {
    const value = current.values[g.key] ?? 0;
    // A gauge may never draw outside its own scale, whatever the inputs were.
    const scaleMax = Math.max(g.scaleMax, value, g.safeMax);
    return {
      ...g,
      scaleMax,
      value,
      safeMin: clamp(g.safeMin, g.scaleMin, scaleMax),
      safeMax: clamp(g.safeMax, g.scaleMin, scaleMax),
    };
  });

  return { state, rows, gauges, notes: current.notes, values: current.values };
}

/** Every note the engine can emit for this profile — rendered hidden so the island only toggles visibility. */
export function possibleNotes(profile: VaultProfile): NoteKey[] {
  const seen = new Set<NoteKey>();
  const groups = getConfigGroups(profile);
  const walk = (index: number, state: ConfigState): void => {
    const group = groups[index];
    if (!group) {
      for (const n of draft(profile, state).notes) seen.add(n);
      return;
    }
    for (const option of group.options) walk(index + 1, { ...state, [group.id]: option.value });
  };
  walk(0, {});
  return [...seen];
}
