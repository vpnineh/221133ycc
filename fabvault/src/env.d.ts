/// <reference path="../.astro/types.d.ts" />
/// <reference types="astro/client" />

interface ImportMetaEnv {
  readonly PUBLIC_AMAZON_TAG_US?: string;
  readonly PUBLIC_AMAZON_TAG_DE?: string;
  readonly PUBLIC_AMAZON_TAG_ES?: string;
  readonly PUBLIC_AMAZON_TAG_FR?: string;
  readonly PUBLIC_AMAZON_TAG_JP?: string;
  readonly PUBLIC_AMAZON_TAG_IT?: string;
  readonly PUBLIC_AMAZON_TAG_NL?: string;
  readonly PUBLIC_AMAZON_TAG_BR?: string;
  readonly PUBLIC_AFF_BAMBULAB?: string;
  readonly PUBLIC_AFF_POLYMAKER?: string;
  readonly PUBLIC_AFF_XTOOL?: string;
  readonly PUBLIC_AFF_ELEGOO?: string;
  readonly PUBLIC_AFF_SIRAYATECH?: string;
  readonly PUBLIC_ADSENSE_CLIENT?: string;
  readonly PUBLIC_ADSENSE_SLOT_INLINE?: string;
  readonly PUBLIC_ADSENSE_SLOT_LEADERBOARD?: string;
}
interface ImportMeta {
  readonly env: ImportMetaEnv;
}
