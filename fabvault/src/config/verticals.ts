import type { VerticalDefinition, VerticalId } from '@/types/vault';

export const VERTICALS: Record<VerticalId, VerticalDefinition> = {
  filament: {
    id: 'filament',
    route: '3d-filament-profiles',
    primaryFormat: 'json',
    software: ['OrcaSlicer', 'Bambu Studio', 'PrusaSlicer', 'Cura'],
  },
  laser: {
    id: 'laser',
    route: 'laser-material-library',
    primaryFormat: 'clb',
    software: ['LightBurn'],
  },
  resin: {
    id: 'resin',
    route: 'resin-exposure-profiles',
    primaryFormat: 'cfg',
    software: ['Chitubox', 'Lychee Slicer'],
  },
  plotter: {
    id: 'plotter',
    route: 'cutting-plotter-settings',
    primaryFormat: 'json',
    software: ['Cricut Design Space', 'Silhouette Studio', 'Candle / gSender'],
  },
};

export const VERTICAL_LIST: readonly VerticalDefinition[] = Object.values(VERTICALS);

export function verticalByRoute(route: string): VerticalDefinition | undefined {
  return VERTICAL_LIST.find((v) => v.route === route);
}
