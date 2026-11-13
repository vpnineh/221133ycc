/**
 * FabVault — unified, strictly typed data contract.
 *
 * One schema covers all four maker verticals, the compensation engine output,
 * the i18n dictionary, E-E-A-T authorship and monetisation. Everything that
 * renders a page, a gauge, a JSON-LD node or a downloadable file reads from
 * the types in this module.
 */

// ───────────────────────────── Locales ─────────────────────────────

export const LOCALES = ['en', 'de', 'es', 'fr', 'ja', 'pt', 'it', 'nl'] as const;
export type Locale = (typeof LOCALES)[number];

/** Free text that ships in English and may be translated per locale later. */
export type LocalizedText = { en: string } & Partial<Record<Exclude<Locale, 'en'>, string>>;

// ───────────────────────────── Verticals ─────────────────────────────

export const VERTICAL_IDS = ['filament', 'laser', 'resin', 'plotter'] as const;
export type VerticalId = (typeof VERTICAL_IDS)[number];

export type FileFormat = 'json' | 'clb' | 'cfg' | 'svg';

export interface VerticalDefinition {
  id: VerticalId;
  /** URL segment, e.g. `3d-filament-profiles`. */
  route: string;
  /** Format of the primary synthesised preset file. */
  primaryFormat: Exclude<FileFormat, 'svg'>;
  /** Applications the preset is meant to be imported into. */
  software: readonly string[];
}

// ───────────────────────────── Shared building blocks ─────────────────────────────

/** A recommended value inside a safe operating window. */
export interface Range {
  min: number;
  rec: number;
  max: number;
}

export type Tone = 'heat' | 'flow' | 'neutral';

export type VerificationStatus = 'lab-verified' | 'community-validated' | 'engineering-baseline';

export type CalibrationMethod =
  | 'flow-temp-tower'
  | 'power-speed-matrix'
  | 'exposure-matrix'
  | 'force-speed-matrix'
  | 'feed-speed-matrix';

export interface Revision {
  /** Semantic profile revision, e.g. `2.4`. */
  version: string;
  /** ISO date (YYYY-MM-DD) of the last parameter change. */
  date: string;
  method: CalibrationMethod;
}

export interface Verification {
  status: VerificationStatus;
  reviewerId: string;
  /** ISO date of the physical verification run. Required when status is `lab-verified`. */
  verifiedOn?: string;
  /** Hardware the verification run was performed on. Required when status is `lab-verified`. */
  hardware?: string;
}

export type BrandProgramId = 'bambulab' | 'polymaker' | 'xtool' | 'elegoo' | 'sirayatech';

export interface AffiliateHook {
  /** Search phrase sent to the visitor's Amazon marketplace. */
  amazonQuery: string;
  /** Direct brand programme, when the manufacturer sells the product itself. */
  brand?: BrandProgramId;
  /** Path on the brand store, e.g. `/products/pla-basic-filament`. */
  brandPath?: string;
}

export interface ProfileGroup {
  /** URL segment: brand for filament, machine for the other verticals. */
  slug: string;
  name: string;
}

interface ProfileBase {
  /** Stable unique id: `<vertical>/<group>/<slug>`. */
  id: string;
  group: ProfileGroup;
  slug: string;
  /** Product or material name shown in headings, e.g. `PLA Basic`. */
  name: string;
  revision: Revision;
  verification: Verification;
  /** First-hand bench note from the reviewer. Rendered verbatim. */
  labNote: LocalizedText;
  affiliate: AffiliateHook;
  /** ISO date the profile was first published. */
  published: string;
}

// ───────────────────────────── 1 · Filament ─────────────────────────────

export type FilamentFamily = 'PLA' | 'PETG' | 'ABS' | 'ASA' | 'TPU' | 'PA' | 'PA-CF' | 'PC';
export type BedPlate = 'textured-pei' | 'smooth-pei' | 'cool-plate' | 'engineering-plate';
export type ChamberRequirement = 'required' | 'recommended' | 'none' | 'avoid';
export type SlicerId = 'orcaslicer' | 'bambu-studio' | 'prusaslicer' | 'cura';
export type MoistureIndex = 1 | 2 | 3 | 4 | 5;

export interface FilamentProfile extends ProfileBase {
  vertical: 'filament';
  family: FilamentFamily;
  diameterMm: 1.75 | 2.85;
  densityGcm3: number;
  nozzleTempC: Range;
  bedTempC: Range;
  bedPlate: BedPlate;
  /** Verified melt capacity on a 0.4 mm standard-flow brass hotend, mm³/s. */
  maxVolumetricSpeed: number;
  retraction: { lengthMm: number; speedMmS: number };
  fanPct: { min: number; max: number };
  pressureAdvance: number;
  flowRatio: number;
  chamber: { requirement: ChamberRequirement; tempC: number };
  moistureIndex: MoistureIndex;
  drying: { tempC: number; hours: number };
  abrasive: boolean;
  /** Hardware ceiling of the reference machine class — the gauge full-scale values. */
  limits: { nozzleMaxC: number; bedMaxC: number; flowMax: number };
  slicers: readonly SlicerId[];
}

// ───────────────────────────── 2 · Laser ─────────────────────────────

export type LaserType = 'diode' | 'co2' | 'fiber';
export type HazardLevel = 1 | 2 | 3 | 4 | 5;

export interface LaserOperation {
  /** Percent of the source's optical power. */
  powerPct: Range;
  speedMmMin: Range;
  passes: number;
}

export interface LaserProfile extends ProfileBase {
  vertical: 'laser';
  laser: { type: LaserType; opticalPowerW: number; wavelengthNm: number; maxSpeedMmMin: number };
  materialClass: 'wood' | 'acrylic' | 'leather' | 'metal' | 'paper' | 'stone';
  thicknessMm: number;
  cut: LaserOperation;
  engrave: LaserOperation & { intervalMm: number };
  kerfMm: number;
  focusOffsetMm: number;
  airAssistRecommended: boolean;
  masking: boolean;
  hazards: { fume: HazardLevel; flame: HazardLevel };
}

// ───────────────────────────── 3 · Resin ─────────────────────────────

export type ResinType = 'standard' | 'abs-like' | 'tough' | 'water-washable' | 'flexible';

export interface ResinProfile extends ProfileBase {
  vertical: 'resin';
  printer: { xyResolutionUm: number; maxLiftSpeedMmMin: number };
  resin: {
    brand: string;
    type: ResinType;
    viscosityCps: number;
    densityGcm3: number;
    /** Working-curve penetration depth Dp, used to rescale exposure across layer heights. */
    penetrationDepthMm: number;
  };
  baseLayerHeightMm: number;
  normalExposureS: Range;
  bottomExposureS: Range;
  bottomLayers: number;
  transitionLayers: number;
  lift: { distanceMm: number; speedMmMin: Range };
  retractSpeedMmMin: number;
  lightOffDelayS: number;
  restAfterRetractS: number;
  optimalTempC: { min: number; max: number };
  post: { washMin: number; cureMin: number };
  slicers: readonly ('chitubox' | 'lychee')[];
}

// ───────────────────────────── 4 · Plotter & desktop CNC ─────────────────────────────

export type CuttingMat = 'light-grip' | 'standard-grip' | 'strong-grip' | 'none';

export interface KnifeProcess {
  kind: 'knife';
  blade: string;
  /** Downforce in gram-force. */
  forceG: Range;
  speedMmS: Range;
  bladeDepthMm: number;
  /** Number printed on the blade housing / software dial. */
  bladeSetting: number;
  passes: number;
  mat: CuttingMat;
  cutType: 'kiss-cut' | 'through-cut';
  limits: { forceMaxG: number; speedMaxMmS: number };
}

export interface CncProcess {
  kind: 'cnc';
  bit: string;
  bitDiameterMm: number;
  flutes: number;
  spindleRpm: Range;
  feedMmMin: Range;
  plungeMmMin: number;
  depthPerPassMm: number;
  stepoverPct: number;
  limits: { rpmMax: number; feedMaxMmMin: number };
}

export interface PlotterProfile extends ProfileBase {
  vertical: 'plotter';
  materialClass: 'vinyl' | 'htv' | 'cardstock' | 'wood' | 'plastic' | 'fabric';
  thicknessMm: number;
  process: KnifeProcess | CncProcess;
}

export type VaultProfile = FilamentProfile | LaserProfile | ResinProfile | PlotterProfile;
export type ProfileOf<V extends VerticalId> = Extract<VaultProfile, { vertical: V }>;

// ───────────────────────────── Compensation engine ─────────────────────────────

export type ParamKey =
  // filament
  | 'nozzleTemp' | 'nozzleTempInitial' | 'bedTemp' | 'maxFlow' | 'lineWidth' | 'layerHeight'
  | 'maxSpeed' | 'retractLength' | 'retractSpeed' | 'fanMin' | 'fanMax' | 'pressureAdvance'
  | 'flowRatio' | 'chamberTemp' | 'dryTemp' | 'dryHours' | 'density'
  // laser
  | 'opticalPower' | 'thickness' | 'cutPower' | 'cutSpeed' | 'cutPasses' | 'engravePower'
  | 'engraveSpeed' | 'engraveInterval' | 'lineEnergy' | 'kerf' | 'focusOffset'
  // resin
  | 'normalExposure' | 'bottomExposure' | 'bottomLayers' | 'transitionLayers' | 'liftDistance'
  | 'liftSpeed' | 'zRetractSpeed' | 'lightOffDelay' | 'restAfterRetract' | 'printRate'
  | 'washTime' | 'cureTime' | 'xyResolution'
  // plotter / cnc
  | 'force' | 'toolSpeed' | 'bladeDepth' | 'bladeSetting' | 'passes'
  | 'spindleRpm' | 'feedRate' | 'plungeRate' | 'depthPerPass' | 'stepover' | 'chipLoad' | 'bitDiameter';

export type ConfigGroupId =
  | 'nozzle' | 'hotend' | 'chamber'
  | 'power' | 'air'
  | 'layer' | 'ambient'
  | 'tool' | 'detail';

/** Keys of the translatable option labels (numeric options carry a literal label instead). */
export type ConfigOptionLabelKey =
  | 'brass' | 'hardened' | 'highflow' | 'open' | 'enclosed' | 'on' | 'off'
  | 'toolNew' | 'toolUsed' | 'toolWorn' | 'detailBold' | 'detailStandard' | 'detailIntricate';

export interface ConfigOption {
  value: string;
  /** Literal, locale-neutral label such as `0.4 mm`. */
  label?: string;
  /** Dictionary key for word labels. */
  labelKey?: ConfigOptionLabelKey;
}

export interface ConfigGroup {
  id: ConfigGroupId;
  options: readonly ConfigOption[];
  defaultValue: string;
}

export type ConfigState = Partial<Record<ConfigGroupId, string>>;

export type NoteKey =
  | 'nozzleSmall' | 'nozzleLarge' | 'hardenedSteel' | 'highFlow' | 'abrasiveOnBrass'
  | 'enclosedLowTg' | 'openHighTemp'
  | 'laserUnderpowered' | 'laserSpeedCapped' | 'airOffChar' | 'airOffFlame'
  | 'thinLayers' | 'thickLayers' | 'coldResin' | 'warmResin'
  | 'toolWorn' | 'detailIntricate' | 'detailBold';

export interface ComputedRow {
  key: ParamKey;
  value: number;
  /** Value with every toggle at its default — used to render deltas. */
  base: number;
  unit: string;
  decimals: number;
  tone: Tone;
}

export interface GaugeModel {
  key: ParamKey;
  kind: 'radial' | 'bar';
  value: number;
  safeMin: number;
  safeMax: number;
  scaleMin: number;
  scaleMax: number;
  unit: string;
  decimals: number;
  tone: Tone;
}

export interface ComputedProfile {
  state: Required<ConfigState> | ConfigState;
  rows: ComputedRow[];
  gauges: GaugeModel[];
  notes: NoteKey[];
  /** Flat lookup of every computed parameter. */
  values: Partial<Record<ParamKey, number>>;
}

export interface SynthesizedFile {
  filename: string;
  mime: string;
  format: FileFormat;
  content: string;
}

// ───────────────────────────── Defect diagnostics ─────────────────────────────

export type DefectId =
  | 'stringing' | 'warping' | 'underExtrusion'
  | 'kerfCharring' | 'incompleteCut' | 'engraveBanding'
  | 'delamination' | 'overcure' | 'supportFailure'
  | 'tearing' | 'notCutThrough' | 'materialLift';

export type DefectActionKey =
  | 'dryFilament' | 'raiseDraftShield' | 'cleanPlate' | 'checkNozzleClog'
  | 'useMasking' | 'refocus' | 'cleanLens' | 'tightenBelts'
  | 'warmResin' | 'levelPlate' | 'addSupports' | 'antiAlias'
  | 'replaceBlade' | 'cleanMat' | 'burnish' | 'climbCut';

export interface DefectAdjustment {
  key: ParamKey;
  from: number;
  to: number;
  unit: string;
  decimals: number;
}

export interface DefectDiagnosis {
  id: DefectId;
  adjustments: DefectAdjustment[];
  actions: DefectActionKey[];
}

// ───────────────────────────── E-E-A-T ─────────────────────────────

export interface AuthorProfile {
  id: string;
  /** `person` emits schema.org/Person, `organization` emits an editorial-team Organization. */
  kind: 'person' | 'organization';
  name: string;
  jobTitle: LocalizedText;
  bio: LocalizedText;
  credentials: readonly LocalizedText[];
  expertise: readonly string[];
  links: { github?: string; orcid?: string; linkedin?: string; website?: string };
}

// ───────────────────────────── Monetisation ─────────────────────────────

export type AmazonMarketplaceId = 'US' | 'DE' | 'ES' | 'FR' | 'JP' | 'IT' | 'NL' | 'BR';

export interface AmazonMarketplace {
  id: AmazonMarketplaceId;
  domain: string;
  /** Associates tracking id, empty when not configured. */
  tag: string;
  /** ISO 3166-1 alpha-2 regions served by this storefront. */
  regions: readonly string[];
}

export interface BrandProgram {
  id: BrandProgramId;
  name: string;
  storeUrl: string;
  /** Query-string fragment appended to outbound links, empty when not configured. */
  params: string;
}

export interface AdSlotDefinition {
  id: 'inline' | 'leaderboard';
  slot: string;
  /** Reserved height classes so the slot can never shift layout. */
  reserveClass: string;
  format: 'rectangle' | 'horizontal';
}

export interface MonetizationConfig {
  amazon: Record<AmazonMarketplaceId, AmazonMarketplace>;
  localeMarketplace: Record<Locale, AmazonMarketplaceId>;
  brands: Record<BrandProgramId, BrandProgram>;
  adsense: { client: string; slots: Record<AdSlotDefinition['id'], AdSlotDefinition> };
  /** `rel` applied to every paid outbound link. */
  sponsoredRel: string;
}

// ───────────────────────────── i18n dictionary ─────────────────────────────

export interface QA {
  q: string;
  a: string;
}

export interface VerticalCopy {
  /** Plural hub name, e.g. `3D Printer Filament Profiles`. */
  name: string;
  /** Short label for nav and tables, e.g. `Filament`. */
  short: string;
  /** One-sentence description of the hub. */
  description: string;
  /** What a "group" is in this vertical: `Brand` or `Machine`. */
  groupNoun: string;
  /** Human label of the preset file, e.g. `OrcaSlicer / Bambu Studio preset`. */
  fileLabel: string;
  /** Profile page H1 template. Vars: {group} {name}. */
  h1: string;
  /** <title> template. Vars: {group} {name}. */
  metaTitle: string;
  /** Meta description template. Vars depend on vertical, see `content.ts`. */
  metaDescription: string;
  /** ≈45-word direct answer. Vars depend on vertical, see `content.ts`. */
  summary: string;
  /** HowTo name. Vars: {group} {name}. */
  howToName: string;
  /** Ordered import steps. Vars: {file}. */
  howToSteps: readonly string[];
  /** Four engineering FAQs. Vars depend on vertical, see `content.ts`. */
  faqs: readonly [QA, QA, QA, QA];
}

export interface TranslationDictionary {
  meta: { locale: Locale; languageName: string; ogLocale: string };
  site: {
    name: string;
    tagline: string;
    description: string;
    skipToContent: string;
  };
  nav: {
    primaryLabel: string;
    home: string;
    methodology: string;
    search: string;
    language: string;
    breadcrumb: string;
  };
  verticals: Record<VerticalId, VerticalCopy>;
  home: {
    metaTitle: string;
    eyebrow: string;
    title: string;
    lede: string;
    ctaBrowse: string;
    ctaMethod: string;
    cardCaption: string;
    legendHeat: string;
    legendFlow: string;
    indexTitle: string;
    indexLede: string;
    colVertical: string;
    colFormat: string;
    colImportsInto: string;
    colProfiles: string;
    latestTitle: string;
    latestLede: string;
    colProfile: string;
    colKeySettings: string;
    colRevision: string;
    methodTitle: string;
    methodLede: string;
    methodSteps: readonly [{ title: string; body: string }, { title: string; body: string }, { title: string; body: string }];
  };
  hub: {
    /** Vars: {vertical}. */
    metaTitle: string;
    /** Vars: {group} {vertical}. */
    groupTitle: string;
    /** Vars: {count} {group}. */
    groupLede: string;
    /** Vars: {count}. */
    countLabel: string;
    openProfile: string;
    allGroups: string;
  };
  profile: {
    titleBlock: {
      label: string;
      group: string;
      material: string;
      revision: string;
      calibrated: string;
      format: string;
      status: string;
      hash: string;
    };
    summaryHeading: string;
    summaryAria: string;
    /** Vars: {ext}. */
    summaryToolNote: string;
    specCaption: string;
    colParameter: string;
    colValue: string;
    colWindow: string;
    workbenchTitle: string;
    workbenchLede: string;
    configTitle: string;
    configLegend: string;
    resultsTitle: string;
    resultsCaption: string;
    colBaseline: string;
    colCompensated: string;
    notesTitle: string;
    noNotes: string;
    reset: string;
    relatedTitle: string;
    labNoteTitle: string;
    safetyTitle: string;
  };
  params: Record<ParamKey, string>;
  config: {
    groups: Record<ConfigGroupId, string>;
    options: Record<ConfigOptionLabelKey, string>;
  };
  notes: Record<NoteKey, string>;
  gauges: {
    title: string;
    safeWindow: string;
    hardwareLimit: string;
    current: string;
    /** Vars: {label} {value} {unit} {min} {max} {limit}. */
    aria: string;
    moistureTitle: string;
    moistureLevels: readonly [string, string, string, string, string];
    /** Vars: {temp} {hours}. */
    dryingHint: string;
    enclosureTitle: string;
    enclosure: Record<ChamberRequirement, string>;
    fumeTitle: string;
    flameTitle: string;
    hazardLevels: readonly [string, string, string, string, string];
    maskingOn: string;
    maskingOff: string;
    resinTempTitle: string;
    resinPpe: string;
    matTitle: string;
    mats: Record<CuttingMat, string>;
    cutTypes: Record<KnifeProcess['cutType'], string>;
    dustTitle: string;
    dustHint: string;
  };
  preview: {
    title: string;
    tabPreset: string;
    tabCard: string;
    tabsLabel: string;
    /** Vars: {file}. */
    download: string;
    downloadCard: string;
    copy: string;
    copied: string;
    generatedNote: string;
    cardAlt: string;
    cardNote: string;
    codeLabel: string;
  };
  defects: {
    title: string;
    lede: string;
    tabsLabel: string;
    cause: string;
    adjustments: string;
    actions: string;
    items: Record<DefectId, { name: string; symptom: string; cause: string }>;
    actionLabels: Record<DefectActionKey, string>;
  };
  howTo: { title: string; stepLabel: string };
  faq: { title: string };
  eeat: {
    label: string;
    status: Record<VerificationStatus, { badge: string; detail: string }>;
    reviewedBy: string;
    methods: Record<CalibrationMethod, string>;
    /** Vars: {version}. */
    revision: string;
    calibrated: string;
    verifiedOn: string;
    hardware: string;
    hashLabel: string;
    methodologyLink: string;
    profileLink: string;
  };
  monetization: {
    title: string;
    disclosure: string;
    /** Vars: {store}. */
    amazonCta: string;
    /** Vars: {brand}. */
    brandCta: string;
    /** Vars: {query}. */
    amazonHint: string;
    brandHint: string;
    adLabel: string;
  };
  search: {
    open: string;
    title: string;
    placeholder: string;
    inputLabel: string;
    close: string;
    empty: string;
    hint: string;
    loading: string;
    error: string;
    /** Vars: {count}. */
    results: string;
  };
  footer: {
    about: string;
    disclaimer: string;
    affiliate: string;
    explore: string;
    rights: string;
    languages: string;
  };
  methodology: {
    metaTitle: string;
    metaDescription: string;
    title: string;
    lede: string;
    sections: readonly { id: string; heading: string; paragraphs: readonly string[] }[];
    statusHeading: string;
    statusLede: string;
  };
  author: {
    /** Vars: {name}. */
    metaTitle: string;
    credentials: string;
    expertise: string;
    links: string;
    reviewed: string;
    kindLabel: Record<AuthorProfile['kind'], string>;
  };
  notFound: { title: string; body: string; cta: string };
}
