/**
 * Defect diagnostics — turns a profile's baseline into concrete, material-specific
 * corrections ("220 → 215 °C") for the most common failure modes of its process.
 */
import type {
  DefectAdjustment, DefectDiagnosis, ParamKey, VaultProfile,
} from '@/types/vault';
import { computeProfile } from './compensation';
import { clamp, round } from './format';

export function diagnoseDefects(profile: VaultProfile): DefectDiagnosis[] {
  const computed = computeProfile(profile);
  const rows = new Map(computed.rows.map((r) => [r.key, r]));

  const adjust = (key: ParamKey, next: (from: number) => number): DefectAdjustment | null => {
    const row = rows.get(key);
    if (!row) return null;
    const to = round(next(row.value), row.decimals);
    if (to === row.value) return null;
    return { key, from: row.value, to, unit: row.unit, decimals: row.decimals };
  };
  const list = (...items: Array<DefectAdjustment | null>): DefectAdjustment[] =>
    items.filter((i): i is DefectAdjustment => i !== null);

  switch (profile.vertical) {
    case 'filament': {
      const p = profile;
      const wet = p.moistureIndex >= 3;
      const shrinks = p.chamber.requirement === 'required' || p.chamber.requirement === 'recommended';
      return [
        {
          id: 'stringing',
          adjustments: list(
            adjust('nozzleTemp', (v) => clamp(v - (p.family === 'PETG' ? 10 : 5), p.nozzleTempC.min, p.nozzleTempC.max)),
            adjust('retractLength', (v) => v + (p.family === 'TPU' ? 0.4 : 0.2)),
            adjust('retractSpeed', (v) => Math.min(v + 5, 60)),
          ),
          actions: wet ? ['dryFilament'] : [],
        },
        {
          id: 'warping',
          adjustments: list(
            adjust('bedTemp', (v) => clamp(v + 5, 0, p.limits.bedMaxC)),
            adjust('fanMax', (v) => Math.round(v * (shrinks ? 0.5 : 0.8))),
            adjust('nozzleTempInitial', (v) => clamp(v + 5, p.nozzleTempC.min, p.limits.nozzleMaxC)),
          ),
          actions: shrinks ? ['raiseDraftShield', 'cleanPlate'] : ['cleanPlate'],
        },
        {
          id: 'underExtrusion',
          adjustments: list(
            adjust('nozzleTemp', (v) => clamp(v + 10, p.nozzleTempC.min, Math.min(p.nozzleTempC.max + 5, p.limits.nozzleMaxC))),
            adjust('maxFlow', (v) => v * 0.8),
            adjust('flowRatio', (v) => v + 0.02),
          ),
          actions: wet ? ['checkNozzleClog', 'dryFilament'] : ['checkNozzleClog'],
        },
      ];
    }
    case 'laser': {
      const p = profile;
      return [
        {
          id: 'kerfCharring',
          adjustments: list(
            adjust('cutSpeed', (v) => Math.min(v * 1.15, p.laser.maxSpeedMmMin)),
            adjust('cutPower', (v) => Math.max(10, v - 10)),
            adjust('cutPasses', (v) => v + 1),
          ),
          actions: p.masking ? ['useMasking', 'cleanLens'] : ['cleanLens'],
        },
        {
          id: 'incompleteCut',
          adjustments: list(
            adjust('cutSpeed', (v) => v * 0.8),
            adjust('cutPower', (v) => Math.min(100, v + 5)),
            adjust('focusOffset', () => -round(p.thicknessMm / 2, 1)),
          ),
          actions: ['refocus', 'cleanLens'],
        },
        {
          id: 'engraveBanding',
          adjustments: list(
            adjust('engraveSpeed', (v) => v * 0.8),
            adjust('engraveInterval', (v) => Math.max(0.05, v - 0.02)),
            adjust('engravePower', (v) => Math.max(5, v - 5)),
          ),
          actions: ['tightenBelts'],
        },
      ];
    }
    case 'resin': {
      const p = profile;
      return [
        {
          id: 'delamination',
          adjustments: list(
            adjust('normalExposure', (v) => v * 1.15),
            adjust('liftSpeed', (v) => Math.max(20, v * 0.75)),
            adjust('lightOffDelay', (v) => v + 0.5),
          ),
          actions: ['warmResin'],
        },
        {
          id: 'overcure',
          adjustments: list(
            adjust('normalExposure', (v) => v * 0.88),
            adjust('bottomExposure', (v) => v * 0.8),
            adjust('transitionLayers', (v) => v + 2),
          ),
          actions: ['antiAlias'],
        },
        {
          id: 'supportFailure',
          adjustments: list(
            adjust('bottomExposure', (v) => Math.min(v * 1.25, p.bottomExposureS.max * 1.3)),
            adjust('bottomLayers', (v) => v + 2),
            adjust('liftSpeed', (v) => Math.max(20, v * 0.7)),
          ),
          actions: ['levelPlate', 'addSupports'],
        },
      ];
    }
    case 'plotter': {
      const proc = profile.process;
      if (proc.kind === 'knife') {
        return [
          {
            id: 'tearing',
            adjustments: list(
              adjust('toolSpeed', (v) => Math.max(5, v * 0.6)),
              adjust('force', (v) => Math.max(proc.forceG.min, v * 0.9)),
            ),
            actions: ['replaceBlade'],
          },
          {
            id: 'notCutThrough',
            adjustments: list(
              adjust('force', (v) => Math.min(proc.limits.forceMaxG, v * 1.15)),
              adjust('passes', (v) => v + 1),
              adjust('bladeSetting', (v) => v + 1),
            ),
            actions: ['replaceBlade'],
          },
          {
            id: 'materialLift',
            adjustments: list(adjust('toolSpeed', (v) => Math.max(5, v * 0.75))),
            actions: ['cleanMat', 'burnish'],
          },
        ];
      }
      return [
        {
          id: 'tearing',
          adjustments: list(
            adjust('feedRate', (v) => v * 0.8),
            adjust('depthPerPass', (v) => v * 0.7),
          ),
          actions: ['replaceBlade', 'climbCut'],
        },
        {
          id: 'notCutThrough',
          adjustments: list(
            adjust('depthPerPass', (v) => v * 0.8),
            adjust('plungeRate', (v) => v * 0.75),
          ),
          actions: ['levelPlate'],
        },
        {
          id: 'materialLift',
          adjustments: list(
            adjust('feedRate', (v) => v * 0.85),
            adjust('stepover', (v) => Math.max(10, v - 10)),
          ),
          actions: ['burnish', 'climbCut'],
        },
      ];
    }
  }
}
