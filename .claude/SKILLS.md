# Installed skills

32 skills, provisioned into every session on this repo by the
SessionStart hooks in `.claude/settings.json`. Regenerate with
`bash .claude/scripts/list-skills.sh`.

Account-level skills (uploaded at claude.ai, so they reach every chat
rather than only Claude Code) are listed separately at the end.

## Other

claude-seo (its own hook), graphify, and anything installed by hand.

- **graphify** — Use for any question about a codebase, its architecture, file relationships, or project content — especially when graphify-out/ exists, where the question should be treated as a graphify query first
- **seo** — Comprehensive SEO analysis for any website or business type.
- **seo-ahrefs** — Ahrefs API analyst (extension).
- **seo-audit** — Full website SEO audit with parallel subagent delegation.
- **seo-backlinks** — Backlink profile analysis: referring domains, anchor text distribution, toxic link detection, competitor gap analysis.
- **seo-bing** — Bing Webmaster Tools + IndexNow extension.
- **seo-cluster** —  SERP-based semantic topic clustering for content architecture planning.
- **seo-competitor-pages** —  Generate SEO-optimized competitor comparison and alternatives pages.
- **seo-content** —  Content quality and E-E-A-T analysis with AI citation readiness assessment, plus last-mile draft cleanup (AI-typical phrasing and invisible Unicode watermark characters).
- **seo-content-brief** —  Generate competitive SEO content briefs with per-section word counts, competitor scoring, keyword density guidance, and page-type templates.
- **seo-dataforseo** —  Live SEO data via DataForSEO MCP server: SERP analysis, keyword research (volume, difficulty, intent, trends), backlink profiles, on-page analysis, competitor and content analysis, business listings,
- **seo-drift** —  SEO drift monitoring: capture baselines of SEO-critical elements, detect changes, and track regressions over time.
- **seo-ecommerce** —  E-commerce SEO analysis: Google Shopping visibility, Amazon marketplace intelligence, product schema validation, competitor pricing analysis, and marketplace keyword gaps.
- **seo-firecrawl** —  Full-site crawling, scraping, and site mapping via Firecrawl MCP.
- **seo-flow** —  FLOW framework integration: evidence-led SEO using the Find → Leverage → Optimize → Win loop.
- **seo-geo** —  Optimize content for AI Overviews (formerly SGE), ChatGPT web search, Perplexity, and other AI-powered search experiences.
- **seo-google** —  Google SEO APIs: Search Console (Search Analytics, URL Inspection, Sitemaps), PageSpeed Insights v5, CrUX field data with 25-week history, Indexing API v3, and GA4 organic traffic.
- **seo-hreflang** —  Hreflang and international SEO audit, validation, and generation.
- **seo-image-gen** — AI image generation for SEO assets: OG/social preview images, blog hero images, schema images, product photography, infographics.
- **seo-images** —  Image optimization analysis for SEO and performance.
- **seo-local** —  Local SEO analysis covering Google Business Profile optimization, NAP consistency, citation health, review signals, local schema markup, location page quality, multi-location SEO, and industry-specif
- **seo-maps** —  Maps intelligence for local SEO: geo-grid rank tracking, GBP profile auditing via API, review intelligence across Google/Tripadvisor/Trustpilot, cross-platform NAP verification, competitor radius map
- **seo-page** —  Deep single-page SEO analysis covering on-page elements, content quality, technical meta tags, schema, images, and performance.
- **seo-plan** —  Strategic SEO planning for new or existing websites.
- **seo-profound** — Profound LLM citation tracker (extension).
- **seo-programmatic** —  Programmatic SEO planning and analysis for pages generated at scale from data sources.
- **seo-schema** —  Detect, validate, and generate Schema.org structured data.
- **seo-seranking** — SE Ranking AI visibility analyst (extension).
- **seo-sitemap** —  Analyze existing XML sitemaps or generate new ones with industry templates.
- **seo-sxo** —  Search Experience Optimization: reads Google SERPs backwards to detect page-type mismatches, derives user stories from search intent signals, and scores pages from multiple persona perspectives.
- **seo-technical** —  Technical SEO audit across 9 categories: crawlability, indexability, security, URL structure, mobile, Core Web Vitals, structured data, JavaScript rendering, and IndexNow protocol.
- **seo-unlighthouse** — Multi-page Lighthouse audit via the MIT-licensed Unlighthouse CLI.

## Account level (every chat, not just Claude Code)

Uploaded as skills at claude.ai, so they load in plain chats too. Not managed
by these hooks.

- **typesafe-ai** — building with TypeSafe's System One models
- **graphify** — codebase knowledge graph
- **karpathy-guidelines** — behavioural guardrails against common LLM coding mistakes
- **caveman** — ultra-compressed output mode
- **caveman-review** — one-line-per-finding code review
