/** @type {import('tailwindcss').Config} */
export default {
  content: ['./src/**/*.{astro,html,ts,tsx,md,mdx}'],
  theme: {
    screens: { xs: '400px', sm: '640px', md: '768px', lg: '1024px', xl: '1280px', '2xl': '1536px', '3xl': '2200px' },
    extend: {
      colors: {
        // Surfaces
        ink: '#0B0F17',
        panel: '#111827',
        well: '#0D131E',
        line: '#223046',
        'line-strong': '#3A4C68',
        // Text
        paper: '#E6EDF7',
        mute: '#9FB0C6',
        // Semantic accents: cyan = motion & flow, amber = heat & energy.
        flow: { DEFAULT: '#38BDF8', dim: '#0E4A68' },
        heat: { DEFAULT: '#F59E0B', dim: '#5C3B06' },
        ok: '#4ADE80',
        risk: '#F87171',
      },
      fontFamily: {
        sans: ['"Archivo Variable"', 'Archivo Fallback', 'system-ui', 'sans-serif'],
        mono: ['"JetBrains Mono Variable"', 'ui-monospace', 'SFMono-Regular', 'Menlo', 'monospace'],
      },
      borderRadius: { DEFAULT: '2px', sm: '2px', md: '3px', lg: '4px' },
      maxWidth: { sheet: '1280px' },
      transitionTimingFunction: { machine: 'cubic-bezier(0.22, 1, 0.36, 1)' },
    },
  },
  plugins: [],
};
