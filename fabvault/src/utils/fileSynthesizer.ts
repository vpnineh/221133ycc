/**
 * Zero-bloat file synthesizer.
 *
 * No preset file is ever stored on the server. Each download is assembled in
 * memory from the profile record + the live compensation result and handed to
 * the browser as a Blob, so one static HTML page can serve an unbounded number
 * of hardware-specific files without adding to the Cloudflare Pages file count.
 *
 * Every generator is a pure string function (usable in Node for verification);
 * only `downloadFile` touches browser APIs.
 */
import type {
  ComputedProfile, FilamentProfile, LaserProfile, ParamKey, PlotterProfile, ResinProfile,
  SynthesizedFile, VaultProfile,
} from '@/types/vault';
import { escapeXml, fmt, fnv1a, round, slugify } from './format';

export const GENERATOR = 'FabVault File Synthesizer';
export const GENERATOR_VERSION = '1.0.0';

type Values = ComputedProfile['values'];

function need(values: Values, key: ParamKey): number {
  const v = values[key];
  if (v === undefined || Number.isNaN(v)) throw new Error(`Missing computed parameter "${key}"`);
  return v;
}

function stateSuffix(computed: ComputedProfile): string {
  return Object.values(computed.state)
    .filter((v): v is string => typeof v === 'string' && v.length > 0)
    .map((v) => v.replace('.', ''))
    .join('-');
}

function baseName(profile: VaultProfile, computed: ComputedProfile): string {
  return slugify(`fabvault ${profile.group.name} ${profile.name} ${stateSuffix(computed)}`);
}

/** Deterministic fingerprint of the exact parameter set inside a generated file. */
export function contentHash(profile: VaultProfile, computed: ComputedProfile): string {
  return fnv1a(`${profile.id}@${profile.revision.version}:${JSON.stringify(computed.values)}`);
}

// ───────────────────────────── 1 · OrcaSlicer / Bambu Studio filament JSON ─────────────────────────────

const HOTEND_LABEL: Record<string, string> = { brass: 'Brass', hardened: 'Hardened Steel', highflow: 'High-Flow' };

export function synthFilamentJson(profile: FilamentProfile, computed: ComputedProfile): SynthesizedFile {
  const v = computed.values;
  const s = (key: ParamKey, decimals: number): [string] => [fmt(need(v, key), decimals)];
  const nozzle = computed.state.nozzle ?? '0.4';
  const hotend = HOTEND_LABEL[computed.state.hotend ?? 'brass'] ?? 'Brass';
  const presetName = `FabVault ${profile.group.name} ${profile.name} @${nozzle} ${hotend}`;
  const bed = s('bedTemp', 0);
  const chamber = need(v, 'chamberTemp');

  // Slicer preset schema: scalar metadata as strings, per-extruder values as single-element string arrays.
  const preset = {
    type: 'filament',
    name: presetName,
    from: 'User',
    instantiation: 'true',
    inherits: '',
    is_custom_defined: '0',
    version: `${profile.revision.version}.0.0`,
    filament_settings_id: [presetName],
    filament_id: `FV${contentHash(profile, computed).slice(0, 6).toUpperCase()}`,
    filament_vendor: [profile.group.name],
    filament_type: [profile.family],
    filament_diameter: [fmt(profile.diameterMm, 2)],
    filament_density: [fmt(profile.densityGcm3, 2)],
    filament_flow_ratio: s('flowRatio', 2),
    filament_max_volumetric_speed: s('maxFlow', 1),
    nozzle_temperature: s('nozzleTemp', 0),
    nozzle_temperature_initial_layer: s('nozzleTempInitial', 0),
    nozzle_temperature_range_low: [fmt(profile.nozzleTempC.min, 0)],
    nozzle_temperature_range_high: [fmt(Math.min(profile.nozzleTempC.max + 10, profile.limits.nozzleMaxC), 0)],
    hot_plate_temp: bed,
    hot_plate_temp_initial_layer: bed,
    textured_plate_temp: bed,
    textured_plate_temp_initial_layer: bed,
    eng_plate_temp: bed,
    eng_plate_temp_initial_layer: bed,
    cool_plate_temp: [fmt(Math.max(0, need(v, 'bedTemp') - 20), 0)],
    cool_plate_temp_initial_layer: [fmt(Math.max(0, need(v, 'bedTemp') - 20), 0)],
    chamber_temperature: [fmt(chamber, 0)],
    activate_chamber_temp_control: [chamber > 0 ? '1' : '0'],
    filament_retraction_length: s('retractLength', 1),
    filament_retraction_speed: s('retractSpeed', 0),
    filament_deretraction_speed: s('retractSpeed', 0),
    fan_min_speed: s('fanMin', 0),
    fan_max_speed: s('fanMax', 0),
    close_fan_the_first_x_layers: [profile.fanPct.max >= 60 ? '1' : '3'],
    enable_pressure_advance: ['1'],
    pressure_advance: s('pressureAdvance', 3),
    required_nozzle_HRC: [profile.abrasive ? '40' : '3'],
    filament_is_support: ['0'],
    compatible_printers: [] as string[],
    filament_notes: `${GENERATOR} ${GENERATOR_VERSION} | rev ${profile.revision.version} | ${nozzle} mm ${hotend} | line ${fmt(need(v, 'lineWidth'), 2)} mm, layer ${fmt(need(v, 'layerHeight'), 2)} mm, max ${fmt(need(v, 'maxSpeed'), 0)} mm/s | dry ${profile.drying.tempC} C / ${profile.drying.hours} h`,
  };

  return {
    filename: `${baseName(profile, computed)}.json`,
    mime: 'application/json',
    format: 'json',
    content: `${JSON.stringify(preset, null, 2)}\n`,
  };
}

// ───────────────────────────── 2 · LightBurn .clb material library ─────────────────────────────

function clbSetting(
  type: 'Cut' | 'Scan',
  index: number,
  fields: Array<[tag: string, value: string]>,
): string {
  const lines = fields.map(([tag, value]) => `                <${tag} Value="${escapeXml(value)}"/>`);
  return [
    `            <CutSetting type="${type}">`,
    `                <index Value="${index}"/>`,
    `                <name Value="C${String(index).padStart(2, '0')}"/>`,
    ...lines,
    '            </CutSetting>',
  ].join('\n');
}

export function synthLightBurnClb(profile: LaserProfile, computed: ComputedProfile): SynthesizedFile {
  const v = computed.values;
  const air = computed.state.air === 'on' ? '1' : '0';
  const watts = need(v, 'opticalPower');
  // LightBurn stores speed in mm/s regardless of the unit shown in its UI.
  const mmPerSec = (key: ParamKey): string => fmt(need(v, key) / 60, 3);
  const material = `${profile.name} — ${profile.group.name} ${fmt(watts, 0)}W`;
  const thickness = profile.thicknessMm.toFixed(4);

  const cut = clbSetting('Cut', 0, [
    ['maxPower', fmt(need(v, 'cutPower'), 0)],
    ['maxPower2', fmt(need(v, 'cutPower'), 0)],
    ['minPower', fmt(need(v, 'cutPower'), 0)],
    ['speed', mmPerSec('cutSpeed')],
    ['numPasses', fmt(need(v, 'cutPasses'), 0)],
    ['kerf', fmt(need(v, 'kerf') / 2, 3)],
    ['zOffset', fmt(need(v, 'focusOffset'), 2)],
    ['airAssist', air],
    ['runBlower', air],
    ['priority', '1'],
  ]);
  const scan = clbSetting('Scan', 1, [
    ['maxPower', fmt(need(v, 'engravePower'), 0)],
    ['maxPower2', fmt(need(v, 'engravePower'), 0)],
    ['minPower', fmt(need(v, 'engravePower'), 0)],
    ['speed', mmPerSec('engraveSpeed')],
    ['numPasses', fmt(profile.engrave.passes, 0)],
    ['interval', fmt(need(v, 'engraveInterval'), 3)],
    ['bidir', '1'],
    ['airAssist', '0'],
    ['runBlower', '0'],
    ['priority', '0'],
  ]);

  const content = [
    '<?xml version="1.0" encoding="UTF-8"?>',
    `<!-- ${GENERATOR} ${GENERATOR_VERSION} | rev ${profile.revision.version} | ${fmt(watts, 0)} W ${profile.laser.type} | air assist ${air === '1' ? 'on' : 'off'} | hash ${contentHash(profile, computed)} -->`,
    '<LightBurnLibrary>',
    `    <Material name="${escapeXml(material)}">`,
    `        <Entry Thickness="${thickness}" Desc="Cut ${fmt(profile.thicknessMm, 1)} mm" NoThickTitle="">`,
    cut,
    '        </Entry>',
    '        <Entry Thickness="-1.0000" Desc="Engrave" NoThickTitle="Engrave">',
    scan,
    '        </Entry>',
    '    </Material>',
    '</LightBurnLibrary>',
    '',
  ].join('\n');

  return {
    filename: `${baseName(profile, computed)}.clb`,
    mime: 'application/xml',
    format: 'clb',
    content,
  };
}

// ───────────────────────────── 3 · Chitubox / Lychee resin .cfg ─────────────────────────────

export function synthResinCfg(profile: ResinProfile, computed: ComputedProfile): SynthesizedFile {
  const v = computed.values;
  const lift = need(v, 'liftDistance');
  const pairs: Array<[key: string, value: string]> = [
    ['profile_name', `FabVault ${profile.resin.brand} ${profile.name} @${fmt(need(v, 'layerHeight'), 2)}mm`],
    ['machine_name', profile.group.name],
    ['resin_name', `${profile.resin.brand} ${profile.name}`],
    ['resin_type', profile.resin.type],
    ['resin_density', fmt(profile.resin.densityGcm3, 2)],
    ['layer_thickness', fmt(need(v, 'layerHeight'), 3)],
    ['normal_exposure_time', fmt(need(v, 'normalExposure'), 1)],
    ['bottom_exposure_time', fmt(need(v, 'bottomExposure'), 0)],
    ['bottom_layer_count', fmt(need(v, 'bottomLayers'), 0)],
    ['transition_layer_count', fmt(need(v, 'transitionLayers'), 0)],
    ['z_lift_distance', fmt(lift, 1)],
    ['z_lift_speed', fmt(need(v, 'liftSpeed'), 0)],
    ['z_retract_speed', fmt(need(v, 'zRetractSpeed'), 0)],
    ['bottom_lift_distance', fmt(lift + 1, 1)],
    ['bottom_lift_speed', fmt(Math.round(need(v, 'liftSpeed') * 0.75), 0)],
    ['bottom_retract_speed', fmt(need(v, 'zRetractSpeed'), 0)],
    ['light_off_delay', fmt(need(v, 'lightOffDelay'), 1)],
    ['bottom_light_off_delay', fmt(need(v, 'lightOffDelay') + 1, 1)],
    ['rest_time_after_retract', fmt(need(v, 'restAfterRetract'), 1)],
    ['rest_time_before_lift', '0'],
    ['rest_time_after_lift', '0'],
    ['anti_aliasing', '1'],
    ['anti_aliasing_level', '4'],
  ];
  const width = Math.max(...pairs.map(([k]) => k.length));
  const content = [
    `# ${GENERATOR} ${GENERATOR_VERSION}`,
    `# ${profile.resin.brand} ${profile.name} on ${profile.group.name} | rev ${profile.revision.version}`,
    `# layer ${fmt(need(v, 'layerHeight'), 2)} mm | ambient ${computed.state.ambient ?? '25'} C | hash ${contentHash(profile, computed)}`,
    '# units: time = s, distance = mm, speed = mm/min',
    ...pairs.map(([k, val]) => `${k.padEnd(width)} = ${val}`),
    '',
  ].join('\n');

  return {
    filename: `${baseName(profile, computed)}.cfg`,
    mime: 'text/plain',
    format: 'cfg',
    content,
  };
}

// ───────────────────────────── 4 · Plotter / CNC settings sheet (JSON) ─────────────────────────────

export function synthPlotterJson(profile: PlotterProfile, computed: ComputedProfile): SynthesizedFile {
  const v = computed.values;
  const proc = profile.process;
  const settings =
    proc.kind === 'knife'
      ? {
          process: 'drag-knife',
          blade: proc.blade,
          cut_type: proc.cutType,
          mat: proc.mat,
          downforce_gf: need(v, 'force'),
          speed_mm_s: need(v, 'toolSpeed'),
          blade_depth_mm: need(v, 'bladeDepth'),
          blade_setting: need(v, 'bladeSetting'),
          passes: need(v, 'passes'),
        }
      : {
          process: 'cnc-router',
          bit: proc.bit,
          bit_diameter_mm: need(v, 'bitDiameter'),
          flutes: proc.flutes,
          spindle_rpm: need(v, 'spindleRpm'),
          feed_mm_min: need(v, 'feedRate'),
          plunge_mm_min: need(v, 'plungeRate'),
          depth_per_pass_mm: need(v, 'depthPerPass'),
          stepover_pct: need(v, 'stepover'),
          chip_load_mm: need(v, 'chipLoad'),
        };
  const sheet = {
    type: 'cutting-settings',
    generator: `${GENERATOR} ${GENERATOR_VERSION}`,
    name: `FabVault ${profile.group.name} ${profile.name}`,
    revision: profile.revision.version,
    hash: contentHash(profile, computed),
    machine: profile.group.name,
    material: { name: profile.name, class: profile.materialClass, thickness_mm: profile.thicknessMm },
    conditions: { tool: computed.state.tool ?? 'new', detail: computed.state.detail ?? 'standard' },
    settings,
  };
  return {
    filename: `${baseName(profile, computed)}.json`,
    mime: 'application/json',
    format: 'json',
    content: `${JSON.stringify(sheet, null, 2)}\n`,
  };
}

// ───────────────────────────── 5 · 100 × 100 mm true-scale calibration card (SVG) ─────────────────────────────

export interface MatrixAxis {
  label: string;
  unit: string;
  decimals: number;
  values: number[];
}

/** `count` evenly spaced steps across [min, max], always containing the endpoints. */
function steps(min: number, max: number, count: number, decimals: number): number[] {
  if (max <= min) return Array.from({ length: count }, () => round(min, decimals));
  return Array.from({ length: count }, (_, i) => round(min + ((max - min) * i) / (count - 1), decimals));
}

/** Axes of the test matrix, centred on the active compensated values. */
export function calibrationAxes(profile: VaultProfile, computed: ComputedProfile): { x: MatrixAxis; y: MatrixAxis } {
  const g = (key: ParamKey) => computed.gauges.find((gauge) => gauge.key === key);
  const axis = (key: ParamKey, label: string, pad: number): MatrixAxis => {
    const gauge = g(key);
    const value = need(computed.values, key);
    const decimals = gauge?.decimals ?? 0;
    const lo = gauge ? gauge.safeMin : value * (1 - pad);
    const hi = gauge ? gauge.safeMax : value * (1 + pad);
    return { label, unit: gauge?.unit ?? '', decimals, values: steps(Math.min(lo, value), Math.max(hi, value), 5, decimals) };
  };
  switch (profile.vertical) {
    case 'filament': return { x: axis('nozzleTemp', 'NOZZLE', 0.05), y: axis('maxFlow', 'FLOW', 0.3) };
    case 'laser': return { x: axis('cutSpeed', 'SPEED', 0.3), y: axis('cutPower', 'POWER', 0.2) };
    case 'resin': return { x: axis('normalExposure', 'EXPOSURE', 0.25), y: axis('liftSpeed', 'LIFT', 0.3) };
    case 'plotter':
      return profile.process.kind === 'knife'
        ? { x: axis('toolSpeed', 'SPEED', 0.3), y: axis('force', 'FORCE', 0.2) }
        : { x: axis('feedRate', 'FEED', 0.3), y: axis('spindleRpm', 'RPM', 0.2) };
  }
}

const n2 = (value: number): string => fmt(value, 2);

/**
 * Printable / cuttable test card. Physical size is fixed at 100 mm × 100 mm with a
 * 1 user-unit = 1 mm viewBox, so it imports at true scale into LightBurn, slicers and
 * vector editors. Stroke colours follow the laser convention: black = cut, blue = score.
 */
export function synthCalibrationSvg(profile: VaultProfile, computed: ComputedProfile): SynthesizedFile {
  const { x, y } = calibrationAxes(profile, computed);
  const cell = 13;
  const gap = 2;
  const originX = 18;
  const originY = 17;
  const span = cell * 5 + gap * 4;
  const title = `${profile.group.name} ${profile.name}`.toUpperCase().slice(0, 44);
  const out: string[] = [];

  out.push(
    '<?xml version="1.0" encoding="UTF-8"?>',
    '<svg xmlns="http://www.w3.org/2000/svg" width="100mm" height="100mm" viewBox="0 0 100 100" font-family="Arial, Helvetica, sans-serif">',
    `  <title>${escapeXml(`${profile.group.name} ${profile.name} calibration card, 100 mm x 100 mm`)}</title>`,
    `  <desc>${escapeXml(`${GENERATOR} ${GENERATOR_VERSION} | rev ${profile.revision.version} | hash ${contentHash(profile, computed)} | 1 unit = 1 mm`)}</desc>`,
    '  <g id="cut" fill="none" stroke="#000000" stroke-width="0.1">',
    '    <rect x="0.05" y="0.05" width="99.9" height="99.9" rx="2"/>',
  );
  for (let row = 0; row < 5; row++) {
    for (let col = 0; col < 5; col++) {
      out.push(
        `    <rect x="${n2(originX + col * (cell + gap))}" y="${n2(originY + row * (cell + gap))}" width="${cell}" height="${cell}"/>`,
      );
    }
  }
  out.push('  </g>', '  <g id="score" fill="none" stroke="#0000FF" stroke-width="0.1">');
  // 0–80 mm verification ruler: measure it after printing/cutting to confirm 1:1 scale.
  const ry = 93;
  out.push(`    <line x1="10" y1="${ry}" x2="90" y2="${ry}"/>`);
  for (let mm = 0; mm <= 80; mm++) {
    const len = mm % 10 === 0 ? 3 : mm % 5 === 0 ? 2 : 1;
    out.push(`    <line x1="${10 + mm}" y1="${ry}" x2="${10 + mm}" y2="${n2(ry - len)}"/>`);
  }
  // The centre cell is the recommended setting: mark it with corner ticks.
  const cx = originX + 2 * (cell + gap);
  const cy = originY + 2 * (cell + gap);
  out.push(
    `    <path d="M${n2(cx - 1)} ${n2(cy + 2)}V${n2(cy - 1)}H${n2(cx + 2)} M${n2(cx + cell - 2)} ${n2(cy - 1)}H${n2(cx + cell + 1)}V${n2(cy + 2)} M${n2(cx + cell + 1)} ${n2(cy + cell - 2)}V${n2(cy + cell + 1)}H${n2(cx + cell - 2)} M${n2(cx + 2)} ${n2(cy + cell + 1)}H${n2(cx - 1)}V${n2(cy + cell - 2)}"/>`,
    '  </g>',
    '  <g id="engrave" fill="#0000FF" stroke="none">',
    `    <text x="5" y="7" font-size="3" font-weight="700">${escapeXml(title)}</text>`,
    `    <text x="5" y="11" font-size="2">${escapeXml(`REV ${profile.revision.version}  ·  ${x.label} × ${y.label} MATRIX  ·  100 × 100 MM  ·  1:1`)}</text>`,
    `    <text x="${n2(originX + span / 2)}" y="15.4" font-size="2" text-anchor="middle">${escapeXml(`${x.label} (${x.unit}) →`)}</text>`,
    `    <text x="4.6" y="${n2(originY + span / 2)}" font-size="2" text-anchor="middle" transform="rotate(-90 4.6 ${n2(originY + span / 2)})">${escapeXml(`← ${y.label} (${y.unit})`)}</text>`,
  );
  x.values.forEach((value, col) => {
    out.push(
      `    <text x="${n2(originX + col * (cell + gap) + cell / 2)}" y="${n2(originY + span + 3.2)}" font-size="2.2" text-anchor="middle">${fmt(value, x.decimals)}</text>`,
    );
  });
  y.values.forEach((value, row) => {
    out.push(
      `    <text x="${n2(originX - 1.4)}" y="${n2(originY + row * (cell + gap) + cell / 2 + 0.8)}" font-size="2.2" text-anchor="end">${fmt(value, y.decimals)}</text>`,
    );
  });
  for (let mm = 0; mm <= 80; mm += 10) {
    out.push(`    <text x="${10 + mm}" y="97" font-size="1.8" text-anchor="middle">${mm}</text>`);
  }
  out.push(`    <text x="95" y="97" font-size="1.8" text-anchor="end">mm</text>`, '  </g>', '</svg>', '');

  return {
    filename: `${baseName(profile, computed)}-calibration-card-100mm.svg`,
    mime: 'image/svg+xml',
    format: 'svg',
    content: out.join('\n'),
  };
}

// ───────────────────────────── Dispatch + browser download ─────────────────────────────

export function synthPrimaryFile(profile: VaultProfile, computed: ComputedProfile): SynthesizedFile {
  switch (profile.vertical) {
    case 'filament': return synthFilamentJson(profile, computed);
    case 'laser': return synthLightBurnClb(profile, computed);
    case 'resin': return synthResinCfg(profile, computed);
    case 'plotter': return synthPlotterJson(profile, computed);
  }
}

/** Browser only: builds the Blob and triggers a download without any network request. */
export function downloadFile(file: SynthesizedFile): void {
  const blob = new Blob([file.content], { type: `${file.mime};charset=utf-8` });
  const url = URL.createObjectURL(blob);
  const anchor = document.createElement('a');
  anchor.href = url;
  anchor.download = file.filename;
  anchor.rel = 'noopener';
  document.body.appendChild(anchor);
  anchor.click();
  anchor.remove();
  window.setTimeout(() => URL.revokeObjectURL(url), 1500);
}
