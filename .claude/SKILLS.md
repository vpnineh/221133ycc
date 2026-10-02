# Installed skills

159 skills, provisioned into every session on this repo by the
SessionStart hooks in `.claude/settings.json`. Regenerate with
`bash .claude/scripts/list-skills.sh`.

Account-level skills (uploaded at claude.ai, so they reach every chat
rather than only Claude Code) are listed separately at the end.

## ibelick/ui-skills

- **baseline-ui** — Quickly deslop UI code by fixing spacing, hierarchy, typography, and small layout issues.
- **create-design-md** — Create or update a DESIGN.md from an existing product repository or public website.
- **fixing-accessibility** — Audit and fix HTML accessibility issues including ARIA labels, keyboard navigation, focus management, color contrast, and form errors.
- **fixing-metadata** —  Audit and fix HTML metadata including page titles, meta descriptions, canonical URLs, Open Graph tags, Twitter cards, favicons, JSON-LD structured data, and robots directives.
- **fixing-motion-performance** — Audit and fix animation performance issues including layout thrashing, compositor properties, scroll-linked motion, and blur effects.
- **improve-ui** — Audit an existing product surface against its own design evidence, identify verified UI problems, and write self-contained implementation plans for another agent.
- **ui-skills-root** — Use before UI-related work to select the smallest useful UI Skills context through the ui-skills CLI.

## vercel-labs/agent-skills

- **deploy-to-vercel** — Deploy applications and websites to Vercel.
- **vercel-cli-with-tokens** — Deploy and manage projects on Vercel using token-based authentication.
- **vercel-composition-patterns** —  React composition patterns that scale.
- **vercel-optimize** — Use for Vercel cost and performance optimization on deployed projects, especially Next.js, SvelteKit, Nuxt, and limited Astro apps.
- **vercel-react-best-practices** — React and Next.js performance optimization guidelines from Vercel Engineering.
- **vercel-react-native-skills** —  React Native and Expo best practices for building performant mobile apps.
- **vercel-react-view-transitions** — Guide for implementing smooth, native-feeling animations using React's View Transition API (`<ViewTransition>` component, `addTransitionType`, and CSS view transition pseudo-elements).
- **web-design-guidelines** — Review UI code for Web Interface Guidelines compliance.
- **writing-guidelines** — Review docs/prose for Writing Guidelines compliance.

## pbakaus/impeccable

- **impeccable** — Use when the user wants to design, redesign, shape, critique, audit, polish, clarify, distill, harden, optimize, adapt, animate, colorize, extract, or otherwise improve a frontend interface.

## nextlevelbuilder/ui-ux-pro-max-skill

- **banner-design** — Design banners for social media, ads, website heroes, creative assets, and print.
- **brand** — Brand voice, visual identity, messaging frameworks, asset management, brand consistency.
- **design** — Comprehensive design skill: brand identity, design tokens, UI styling, logo generation (55 styles, Gemini, Atlas Cloud, or MuAPI AI), corporate identity program (50 deliverables, CIP mockups), HTML pr
- **design-system** — Token architecture, component specifications, and slide generation.
- **slides** — Create strategic HTML presentations with Chart.js, design tokens, responsive layouts, copywriting formulas, and contextual slide strategies.
- **ui-styling** — Create beautiful, accessible user interfaces with shadcn/ui components (built on Radix UI + Tailwind), Tailwind CSS utility-first styling, and canvas-based visual designs.
- **ui-ux-pro-max** — UI/UX design intelligence for web, mobile, and desktop.

## upstash/context7

- **context7-cli** — Use the ctx7 CLI to fetch library documentation, manage AI coding skills, and configure Context7 MCP.
- **context7-mcp** — This skill should be used when the user asks about libraries, frameworks, API references, or needs code examples.
- **find-docs** —  Retrieves up-to-date documentation, API references, and code examples for any developer technology.

## anthropics/skills

- **frontend-design** — Guidance for distinctive, intentional visual design when building new UI or reshaping an existing one.

## coreyhaines31/marketingskills

- **ab-testing** — When the user wants to plan, design, or implement an A/B test or experiment, or build a growth experimentation program.
- **ad-creative** — When the user wants to generate, iterate, or scale ad creative — headlines, descriptions, primary text, or full ad variations — for any paid advertising platform.
- **ads** — When the user wants help with paid advertising campaigns on Google Ads, Meta (Facebook/Instagram), LinkedIn, Twitter/X, or other ad platforms.
- **ai-seo** — When the user wants to optimize content for AI search engines, get cited by LLMs, or appear in AI-generated answers.
- **analytics** — When the user wants to set up, improve, or audit analytics tracking and measurement.
- **aso** — When the user wants to audit or optimize an App Store or Google Play listing.
- **attribution** — When the user wants to figure out which marketing actually drives conversions and revenue, choose or interpret an attribution model, or reconcile conflicting numbers across tools.
- **churn-prevention** — When the user wants to reduce churn, build cancellation flows, set up save offers, recover failed payments, or implement retention strategies.
- **co-marketing** — When the user wants to find co-marketing partners, plan joint campaigns, or brainstorm partnership opportunities.
- **cold-email** — Write B2B cold emails and follow-up sequences that get replies.
- **community-marketing** — Build and leverage online communities to drive product growth and brand loyalty.
- **competitor-profiling** — When the user wants to research, profile, or analyze competitors from their URLs.
- **competitors** — When the user wants to create competitor comparison or alternative pages for SEO and buyer-facing use.
- **content-strategy** — When the user wants to plan a content strategy, decide what content to create, or figure out what topics to cover.
- **copy-editing** — When the user wants to edit, review, or improve existing marketing copy, or refresh outdated content.
- **copywriting** — When the user wants to write, rewrite, or improve marketing copy for any page — including homepage, landing pages, pricing pages, feature pages, about pages, or product pages.
- **cro** — When the user wants to optimize, improve, or increase conversions on any marketing page or form — including homepage, landing pages, pricing pages, feature pages, lead capture forms, or contact form
- **customer-research** — When the user wants to conduct, analyze, or synthesize customer research.
- **directory-submissions** — When the user wants to submit their product to startup, SaaS, AI, agent, MCP, no-code, or review directories for backlinks, domain rating, and discovery.
- **emails** — When the user wants to create or optimize an email sequence, drip campaign, automated email flow, or lifecycle email program.
- **events** — When the user wants to plan, run, sponsor, speak at, or get pipeline from events — webinars, conferences, trade shows, meetups, dinners, workshops, virtual summits, or user conferences.
- **free-tools** — When the user wants to plan, evaluate, or build a free tool for marketing purposes — lead generation, SEO value, or brand awareness.
- **image** — When the user wants to create, generate, edit, or optimize images for marketing — blog heroes, social graphics, product mockups, profile banners, listing visuals, or brand assets.
- **influencer-marketing** — When the user wants to run influencer, creator, or ambassador partnerships to promote their product — finding and vetting partners, structuring deals, briefing creators, disclosure compliance, and m
- **launch** — When the user wants to plan a product launch, feature announcement, or release strategy.
- **lead-magnets** — When the user wants to create, plan, or optimize a lead magnet for email capture or lead generation.
- **marketing-council** — When the user wants multiple expert perspectives on a marketing question — a simulated board of advisors staffed by legendary marketers (Seth Godin, David Ogilvy, Eugene Schwartz, April Dunford, Ror
- **marketing-ideas** — When the user needs marketing ideas, inspiration, or strategies for their SaaS or software product.
- **marketing-loops** — When the user wants to set up a recurring, self-running marketing workflow — a repeatable loop an AI agent runs on a cadence (weekly, daily, on a trigger) rather than a one-off task.
- **marketing-plan** — When the user needs a comprehensive marketing plan for a client, a company they advise, or their own product.
- **marketing-psychology** — When the user wants to apply psychological principles, mental models, or behavioral science to marketing.
- **marketing-seo-audit** — When the user wants to audit, review, or diagnose SEO issues on their site.
- **offers** — When the user wants to design, construct, or improve an offer — the thing they actually sell — including value framing, bonus stacking, guarantee design, scarcity/urgency, naming, and payment stru
- **onboarding** — When the user wants to optimize post-signup onboarding, user activation, first-run experience, or time-to-value.
- **paywalls** — When the user wants to create or optimize in-app paywalls, upgrade screens, upsell modals, or feature gates.
- **popups** — When the user wants to create or optimize popups, modals, overlays, slide-ins, or banners for conversion purposes.
- **pricing** — When the user wants help with pricing decisions, packaging, or monetization strategy.
- **product-marketing** — When the user wants to create or update their product marketing context document.
- **programmatic-seo** — When the user wants to create SEO-driven pages at scale using templates and data.
- **prospecting** — When the user wants to find, qualify, and build a list of prospects to reach out to — across B2B SaaS, general B2B, or local small businesses.
- **public-relations** — When the user wants help with public relations, earned media, press coverage, journalist outreach, or media strategy (not pull requests).
- **referrals** — When the user wants to create, optimize, or analyze a referral program, affiliate program, or word-of-mouth strategy.
- **revops** — When the user wants help with revenue operations, lead lifecycle management, or marketing-to-sales handoff processes.
- **sales-enablement** — When the user wants to create sales collateral, pitch decks, one-pagers, objection handling docs, or demo scripts.
- **schema** — When the user wants to add, fix, or optimize schema markup and structured data on their site.
- **signup** — When the user wants to optimize signup, registration, account creation, or trial activation flows.
- **site-architecture** — When the user wants to plan, map, or restructure their website's page hierarchy, navigation, URL structure, or internal linking.
- **sms** — When the user wants to plan, build, or optimize SMS or MMS marketing — including welcome flows, abandoned cart texts, post-purchase, win-back, promotional sends, or transactional/auth SMS.
- **social** — When the user wants help creating, scheduling, or optimizing social media content for LinkedIn, Twitter/X, Instagram, TikTok, Facebook, or other platforms, or wants to do social listening and engageme
- **video** — When the user wants to create, generate, or produce video content using AI tools or programmatic frameworks.

## JuliusBrussee/caveman

- **cavecrew** —  When to delegate to `cavecrew-investigator` (locate code), `cavecrew-builder` (1-2 file edit) or `cavecrew-reviewer` (diff review) instead of working inline or using `Explore`.
- **caveman-commit** —  Write a Conventional Commits message compressed to intent only.
- **caveman-compress** —  Compress a memory file such as CLAUDE.md or a todo list into caveman format to save input tokens, keeping a readable backup.
- **caveman-discover** —  Find and label every LLM workflow in the repository so Caveman Cloud groups spend by workflow instead of one bucket.
- **caveman-evidence-review** —  Read-only review of Caveman Cloud evidence: cost, Cave Score, workflows, traces, latency, errors, routing, savings.
- **caveman-explore** — Read-only repository explorer for cold-start orientation, broad cross-file localization, or when a direct search failed.
- **caveman-help** —  Quick-reference card for caveman modes, skills and commands.
- **caveman-learn** — Act on a Caveman learn report - review the ranked token sinks, apply cost-lowering fixes with per-edit consent, and report what those fixes returned.
- **caveman-manage** —  Inspect Caveman Cloud's experiment lifecycle and block unsafe execution.
- **caveman-optimize** —  Turn a Caveman optimization observation into an operator-chosen candidate with a paired baseline evaluation.
- **caveman-setup** —  Wire a repository through the Caveman Cloud gateway so every LLM request is measured, with no behavior change.
- **caveman-stats** —  Show recorded output and cache-read token usage and mode attribution for the current Claude Code session, or locate the host's native usage report.
- **investigate-first** — Diagnose ambiguous failures before editing.
- **lean-build** — Build feature work with high overbuilding risk.
- **migration** — Implement reversible compatibility-safe transitions.
- **safe-refactor** — Restructure code while preserving behavior.
- **surgical-patch** — Fix bugs and small behavior changes at the narrowest responsible layer.
- **verify-and-stop** — Prove existing work meets acceptance conditions without expanding scope.

## emilkowalski/skills

- **animate** — Build an animation from scratch, making the decisions in the order that determines whether it feels right — should it animate at all, what purpose, which tool, which properties, which curve and dura
- **animate-expo** — Build animations in React Native and Expo, making the decisions in the order that determines whether they feel right — should it animate, which thread it runs on, which properties, spring or timing,
- **animation-vocabulary** — Reverse-lookup glossary that turns a vague description of a web animation or motion effect into its exact term ("the bouncy thing when a popover opens" → Pop in; "the iOS rubber-band scroll" → Rub
- **apple-design** — Apple's approach to interface design and fluid, physical motion, translated for the web.
- **ask-sonner** — Guide to Sonner, the React toast library — install and wire up the Toaster, pick the right toast() call, promise and loading toasts, updating, dismissing and persisting toasts, styling, theming and
- **emil-design-eng** — This skill encodes Emil Kowalski's philosophy on UI polish, component design, animation decisions, and the invisible details that make software feel great.
- **find-animation-opportunities** — Search a codebase or UI for places that don't animate but should, and reject everything that shouldn't.
- **improve-animations** — Survey a codebase's animation and motion code as a senior motion advisor, then produce a prioritized audit and self-contained implementation plans for other agents (or cheaper models) to execute.
- **mobile-native** — Make a web app feel native on a phone — the small CSS and meta-tag fixes that separate "a website in a browser" from something that feels installed.
- **pick-ui-library** — Pick the right library for a given frontend task from a curated, opinionated list — numbers, OTP inputs, charts, command menus, virtualization, drag and drop, toasts, state, styling, and more.
- **prototype** — Build multiple genuinely different versions of a UI piece you describe, rendered behind a visual picker so you can flip through them live and promote the one that feels right.
- **review-animations** — Reviews animation and motion code against a high craft bar derived from Emil Kowalski's design engineering philosophy.
- **write-swift** — How to write modern Swift well — modeling with value types, Swift 6 data-race safety and approachable concurrency (@concurrent, main-actor-by-default, actors, task groups), protocols and generics (s

## aaron-he-zhu/aaron-marketing-skills

- **competitor-analysis** — 'Use when the user asks to "analyze competitors" or "竞品分析"; benchmarks competitor keywords, content, backlinks, AI citations, and traffic share into strengths, weaknesses, and an action plan.
- **content-gap-analysis** — 'Use when the user asks to "find content gaps", "竞品写了什么", or "还应该写什么"; builds a competitor-relative coverage map of missing topics, keyword gaps, and editorial-calendar opportu
- **content-quality-auditor** — 'Use when auditing content quality, E-E-A-T, or publish readiness; runs a typed 80-item CORE-EEAT profile with evidence coverage, veto checks, and a fix plan.
- **content-writer** — 'Use when the user asks to "write SEO content", "draft a blog post / landing page", "update outdated content", or "fix traffic/ranking decay"; two modes — new drafts pages with keywords, headers, sn
- **domain-authority-auditor** — 'Use when auditing domain authority, trust, or citation credibility; runs a peer-relative 40-item CITE profile with evidence coverage and verified manipulation/penalty veto checks.
- **geo-content-optimizer** — 'Use when the user asks to "optimize for AI citations"; improves citation readiness for ChatGPT, Perplexity, AI Overviews, Gemini, and Claude.
- **keyword-research** — 'Use when the user asks to "find keywords", "挖词", or "搜什么词"; prioritizes search volume, keyword difficulty, intent, and topic clusters from provided or connected data.
- **offsite-signal-analyzer** — 'Use when the user asks to "analyze backlinks", "analyze my off-site signals", or "track AI traffic / ChatGPT / Perplexity referrals"; profiles referring domains, anchor-text mix, toxic links, and dis
- **on-page-seo-checker** — 'Use when the user asks to "audit on-page SEO" or "diagnose why a single page dropped"; scores titles, meta, header structure, keyword placement, links, and images with prioritized fixes.
- **page-play-builder** — 'Use when the user asks to "build programmatic SEO pages", "generate pages at scale", "rank on a high-authority third-party site", "borrow domain authority", "build a vs / alternative page", "do local
- **performance-monitor** — 'Use when the user asks to "generate an SEO report", "出月报", "set SEO alerts", or "排名掉了提醒我"; two modes — report builds multi-metric traffic/ranking/authority/content dashboards, a
- **rank-tracker** — 'Use when the user asks to "track rankings" or "查排名"; measures keyword and SERP-position deltas over time from provided exports or connected tools, including AI-response checks.
- **serp-analysis** — 'Use when the user asks to "analyze the SERP" or "SERP分析"; maps SERP features, layout, ranking factors, search intent, AI Overviews, and snippet opportunities for a query.
- **serp-markup-builder** — 'Use when the user asks to "optimize meta tags", "write title tags / meta descriptions", "add Open Graph or Twitter cards", or "generate schema / JSON-LD" for FAQ, HowTo, Article, Product, or LocalBus
- **site-structure-optimizer** — 'Use when the user asks to "plan my site structure", "design the page hierarchy / navigation / URL taxonomy", "fix internal linking", or "find orphan pages"; runs two modes — architecture (hierarchy
- **technical-seo-checker** — 'Use when the user asks to "check technical SEO"; audits crawlability, indexing, Core Web Vitals, robots.txt, sitemaps, canonicals, redirects, and migrations.

## Other

claude-seo (its own hook), graphify, and anything installed by hand.

- **graphify** — Use for any question about a codebase, its architecture, file relationships, or project content — especially when graphify-out/ exists, where the question should be treated as a graphify query first
- **seo** — Comprehensive SEO analysis for any website or business type.
- **seo-agentic** —  Audit and fix agent readiness: the Lighthouse Agentic Browsing fraction, accessibility tree for agents, robots.txt and Content-Signal for AI agents, WAF treatment of agent traffic, llms.txt, Markdown
- **seo-ahrefs** — Ahrefs API analyst (extension).
- **seo-audit** — Run a full-site SEO audit and return a scored, prioritized report.
- **seo-backlinks** — Analyze a site's backlink profile, anchors, toxic signals, competitors, gaps, and disavow candidates.
- **seo-bing** — Bing Webmaster Tools + IndexNow extension.
- **seo-cluster** —  Cluster keywords by SERP overlap and design hub-and-spoke content architecture with internal links.
- **seo-competitor-pages** —  Generate SEO-optimized competitor comparison and alternatives pages.
- **seo-content** —  Evaluate page content for usefulness, E-E-A-T, readability, thinness, and AI citation readiness, plus last-mile draft cleanup (AI-typical phrasing and invisible Unicode watermark characters).
- **seo-content-brief** —  Generate competitive SEO content briefs with per-section word counts, competitor scoring, keyword density guidance, and page-type templates.
- **seo-dataforseo** —  Live SEO data via DataForSEO MCP server: SERP analysis, keyword research (volume, difficulty, intent, trends), backlink profiles, on-page analysis, competitor and content analysis, business listings,
- **seo-drift** —  SEO drift monitoring: capture baselines of SEO-critical elements, detect changes, and track regressions over time.
- **seo-ecommerce** —  Analyze ecommerce SEO across product pages, product schema, Shopping visibility, marketplace signals, and keyword gaps.
- **seo-firecrawl** —  Full-site crawling, scraping, and site mapping via Firecrawl MCP.
- **seo-flow** —  FLOW framework integration: evidence-led SEO using the Find → Leverage → Optimize → Win loop.
- **seo-geo** —  Audit and improve content for AI Overviews and answer engines, including citability, entity clarity, crawler access, brand signals, and passage structure.
- **seo-google** —  Google SEO APIs: Search Console (Search Analytics, URL Inspection, Sitemaps), PageSpeed Insights v5, CrUX field data with 25-week history, Indexing API v3, and GA4 organic traffic.
- **seo-hreflang** —  Hreflang and international SEO audit, validation, and generation.
- **seo-image-gen** — AI image generation for SEO assets: OG/social preview images, blog hero images, schema images, product photography, infographics.
- **seo-images** —  Image optimization analysis for SEO and performance.
- **seo-local** —  Audit local SEO, including Google Business Profile, NAP consistency, citations, reviews, local schema, location pages, and multi-location structure.
- **seo-maps** —  Maps intelligence for local SEO: geo-grid rank tracking, GBP profile auditing via API, review intelligence across Google/Tripadvisor/Trustpilot, cross-platform NAP verification, competitor radius map
- **seo-matomo** — Matomo Reporting API extension.
- **seo-page** —  Analyze one supplied URL across on-page, content, technical metadata, schema, images, and performance.
- **seo-plan** —  Strategic SEO planning for new or existing websites.
- **seo-profound** — Profound LLM citation tracker (extension).
- **seo-programmatic** —  Programmatic SEO planning and analysis for pages generated at scale from data sources.
- **seo-schema** —  Detect, validate, or generate Schema.org JSON-LD for a supplied page or entity.
- **seo-seranking** — SE Ranking AI visibility analyst (extension).
- **seo-sitemap** —  Analyze existing XML sitemaps or generate new ones with industry templates.
- **seo-sxo** —  Diagnose search-experience and intent mismatches using SERP page types, user stories, and persona scoring.
- **seo-technical** —  Audit technical SEO across crawlability, indexability, security, URLs, mobile, Core Web Vitals, rendering, structured data, and IndexNow.
- **seo-unlighthouse** — Multi-page Lighthouse audit via the MIT-licensed Unlighthouse CLI.

## Account level (every chat, not just Claude Code)

Uploaded as skills at claude.ai, so they load in plain chats too. Not managed
by these hooks.

- **typesafe-ai** — building with TypeSafe's System One models
- **graphify** — codebase knowledge graph
- **karpathy-guidelines** — behavioural guardrails against common LLM coding mistakes
- **caveman** — ultra-compressed output mode
- **caveman-review** — one-line-per-finding code review
