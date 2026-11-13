import { defineConfig } from 'astro/config';
import tailwind from '@astrojs/tailwind';
import sitemap from '@astrojs/sitemap';

// Canonical production origin. Override per environment with SITE_URL
// (Cloudflare Pages → Settings → Environment variables).
const site = process.env.SITE_URL ?? 'https://fabvault.pages.dev';

export default defineConfig({
  site,
  output: 'static',
  trailingSlash: 'always',
  build: { format: 'directory', inlineStylesheets: 'always' },
  compressHTML: true,
  integrations: [
    tailwind({ applyBaseStyles: false }),
    sitemap({
      // "/" is a duplicate of "/en/" (canonicalised there), 404 is not indexable.
      filter: (page) => new URL(page).pathname !== '/' && !page.includes('/404'),
      changefreq: 'weekly',
      priority: 0.7,
    }),
  ],
  vite: { build: { cssCodeSplit: false, assetsInlineLimit: 0 } },
});
