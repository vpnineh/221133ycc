# Installed skills

146 skills, provisioned into every session on this repo by the
SessionStart hooks in `.claude/settings.json`. Regenerate with
`bash .claude/scripts/list-skills.sh`.

Account-level skills (uploaded at claude.ai, so they reach every chat
rather than only Claude Code) are listed separately at the end.

## ibelick/ui-skills

- **baseline-ui** — Quickly deslop UI code by fixing spacing, hierarchy, typography, and small layout issues.
- **component-canvas** — Build a full-screen, interactive canvas to explore existing UI components and their variants with pan and zoom.
- **create-canvas** — Build a freeform canvas to explore, compare, and iterate on anything.
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

- **impeccable** — Use when the user wants to design, redesign, shape, critique, audit, polish, simplify, harden, animate, or otherwise improve a frontend interface: websites, landing pages, dashboards, product UI, comp

## nextlevelbuilder/ui-ux-pro-max-skill

- **banner-design** — Design banners for social media, ads, website heroes, creative assets, and print.
- **brand** — Brand voice, visual identity, messaging frameworks, asset management, brand consistency.
- **design** — Comprehensive design skill: brand identity, design tokens, UI styling, logo generation (55 styles, Gemini, Atlas Cloud, or MuAPI AI), corporate identity program (50 deliverables, CIP mockups), HTML pr
- **design-system** — Token architecture, component specifications, and slide generation.
- **slides** — Create strategic HTML presentations with Chart.js, design tokens, responsive layouts, copywriting formulas, and contextual slide strategies.
- **ui-styling** — Create beautiful, accessible user interfaces with shadcn/ui components (built on Radix UI + Tailwind), Tailwind CSS utility-first styling, and canvas-based visual designs.
- **ui-ux-pro-max** — UI/UX design intelligence for web, mobile, and desktop.

## upstash/context7

- **context7-cli** — Use the ctx7 CLI to fetch library documentation and configure Context7 MCP.
- **context7-mcp** — This skill should be used when the user asks about libraries, frameworks, API references, or needs code examples.
- **find-docs** —  Retrieves up-to-date documentation, API references, and code examples for any developer technology.

## anthropics/skills

- **frontend-design** — Guidance for distinctive, intentional visual design when building new UI or reshaping an existing one.

## coreyhaines31/marketingskills

- **ab-testing** — When the user wants to plan, run, or read A/B tests, or asks a judgement call on one: is the result real, can we stop, which version won, is this worth testing at all: sample size, duration, significa
- **ad-creative** — When the user wants to write or judge ad creative, or asks which ad, hook, or angle to run or kill: headlines, primary text, and variations for Google, Meta, LinkedIn, and TikTok, fresh angles when cr
- **ads** — When the user wants to plan, run, or fix paid ads on Google, Meta, and LinkedIn, or asks a judgement call on them: are ads worth it, which channel or budget split, why CPA rose, kill or keep a campaig
- **ai-seo** — When the user wants to get cited and recommended in AI answers (ChatGPT, Perplexity, AI Overviews), or asks why AI cites a competitor, which content gets cited, or whether llms.txt or a page format is
- **analytics** — When the user wants to set up or audit analytics, or asks whether a metric or number is calculated right, what to track, or how to name events: tracking plans, GA4, GTM, Mixpanel, PostHog, UTMs.
- **aso** — When the user wants to audit or optimize an App Store or Google Play listing, or asks which title, keywords, or screenshots to pick or why downloads are low: subtitle, description, ratings, and compar
- **attribution** — When the user wants to work out which marketing drives revenue, or asks whether a channel is really working, which numbers to trust, or how to credit podcasts and dark social: attribution models, plat
- **churn-prevention** — When the user wants to reduce churn, or asks why customers cancel, whether to offer a discount or pause: cancellation flows, exit surveys, save offers, dunning and failed-payment recovery.
- **co-marketing** — When the user wants co-marketing partners or joint campaigns, or asks whether a partner fits, a size mismatch matters, or how to split work and leads: integration partners, cross-promotion, co-branded
- **cold-email** — When the user wants to write or run B2B cold outbound, or asks whether an opener, reason to reach out, or email works or why nobody replies: cold emails, follow-ups, sending domains, deliverability, L
- **community-marketing** — When the user wants to build or grow an online community, or asks whether a community is worth it, where to host it, or why members stay quiet: Discord, Slack, forums, participation, community-led gro
- **competitor-profiling** — When the user wants to research competitors into dated profiles, or asks what a rival charges, claims, or is doing, or whether a conclusion about them is fair: positioning, pricing, features, reviews,
- **competitors** — When the user wants to create or audit comparison and alternative pages ('X vs Y', 'X alternative', 'best X alternatives'), or asks what a public page may say about a competitor: page formats, fair cl
- **content-strategy** — When the user wants to plan content, or asks which topic or piece to prioritize, what to write about, or why posts bring no customers: topic clusters, editorial calendars, roadmaps tied to customers a
- **conversion-tracking** — When the user wants to set up, audit, or fix conversion tracking to Google Ads, Meta, GA4, LinkedIn, and TikTok, or asks why conversions are missing or doubled: GTM, pixels, CAPI, Enhanced Conversions
- **copy-editing** — When the user wants to edit, tighten, proofread, or refresh existing copy, or asks whether to keep, cut, or change a line in it, without rewriting from scratch.
- **copywriting** — When the user wants to write or rewrite page copy (homepage, landing, pricing, feature), or asks which headline, promise, or message to lead with.
- **cro** — When the user wants more conversions from a page or form, or asks why a page isn't converting, what to change first, or whether to add or remove a field or element: landing and pricing pages, lead for
- **customer-research** — When the user wants to run or synthesize customer research, or asks what customer quotes, reviews, or feedback really mean, what is safe to conclude, and what to ask next: interviews, surveys, voice o
- **directory-submissions** — When the user wants to list a product in startup, SaaS, AI, MCP, and review directories, or asks which directories are worth it or whether listings are working: Product Hunt, G2, Capterra, Alternative
- **emails** — When the user wants to create or improve automated email sequences, or asks what to send, when, or whether an email or flow is working: welcome, onboarding, nurture, and win-back flows.
- **events** — When the user wants to plan, run, sponsor, or speak at events, or asks whether an event or booth is worth the money: webinars, conferences, booths, meetups, show-up rates, and follow-up into pipeline.
- **free-tools** — When the user wants to plan or build a free tool as marketing, or asks whether a tool idea is worth building or which one to pick: calculators, graders, generators, and other interactive tools for lea
- **image** — When the user wants to create, generate, edit, or optimize marketing images, or asks which approach to take (generate, design, or photograph): blog heroes, social graphics, product mockups, banners, O
- **influencer-marketing** — When the user wants creator, influencer, or ambassador partnerships, or asks which creators fit, what to pay, which rights to buy, or whether a sponsorship worked: vetting, contracts, briefs, UGC, new
- **launch** — When the user wants to plan a product or feature launch, or asks whether it's ready, when to announce, or how to get noticed: Product Hunt, beta and early access, waitlists, release go-to-market, laun
- **lead-magnets** — When the user wants to plan or improve a lead magnet, or asks what to give away for emails or how much to ask in return (gating, form fields): checklists, templates, ebooks, gated guides, content upgr
- **link-building** — When the user wants backlinks and mentions that lift rankings and AI citations, or asks how to get sites to link or whether a link tactic is safe: link gaps, unlinked mentions, broken links, guest pos
- **marketing-council** — When the user wants several expert takes on a marketing decision, or asks what a famous marketer would say about it: a simulated board (Seth Godin, David Ogilvy, April Dunford, Byron Sharp, and more),
- **marketing-ideas** — When the user wants marketing ideas or growth tactics for a software product, or asks what to try to get attention or the next customers with little time or budget, or after the obvious channels.
- **marketing-loops** — When the user wants a recurring marketing job an AI agent runs on a cadence or trigger, or asks what a weekly or daily automated check should do, remember, and when it should stop: SEO and ads checks,
- **marketing-plan** — When the user wants a full marketing or go-to-market plan for a client, an advised company, or their own product, or asks how to sequence or prioritize several initiatives with a fixed team and budget
- **marketing-psychology** — When the user wants to apply psychology to marketing, or asks why buyers behave or value things as they do, or whether a persuasion tactic is honest: biases, mental models, anchoring, social proof, sc
- **marketing-seo-audit** — When the user wants to audit or diagnose SEO, or asks why traffic dropped, a page won't rank, or whether a setup like canonicals, hreflang, or redirects is right: technical SEO, indexing, Google updat
- **offers** — When the user wants to design or improve the offer itself, or asks why it isn't selling or how to add urgency honestly: value stacking, bonuses, guarantees, scarcity, payment plans, upsells.
- **onboarding** — When the user wants better onboarding and activation, or asks why new users don't stick or what the first session should do: first-run experience, aha moments, checklists, empty states, time to value.
- **paywalls** — When the user wants to create or improve in-app paywalls and upgrade moments, or asks when or how hard to ask free users to pay (after they've seen value): upgrade screens, upsell modals, feature gate
- **popups** — When the user wants to create or improve popups, modals, slide-ins, sticky bars, and banners, or asks whether one will help or annoy and when to show it: exit intent, email capture, scroll and time tr
- **pricing** — When the user wants to set pricing and packaging, or asks what to charge, whether to raise prices, or how to move existing customers: tiers, value metrics, per-seat vs usage, freemium and trials, will
- **product-marketing** — When the user wants to create or update the product marketing context doc (.agents/product-marketing.md), or asks what belongs in it or whether positioning or ICP is clear enough: product, ICP, positi
- **programmatic-seo** — When the user wants SEO pages at scale from templates and data, or asks whether a page set will rank or look thin: location, integration, comparison, directory, and '[X] for [Y]' pages, each useful ra
- **prospecting** — When the user wants to find and qualify prospects for outbound, or asks who to target first, which signal to prioritize, or where to spot buyers with the problem: buying signals, enrichment, verificat
- **public-relations** — When the user wants press and media coverage, or asks what to say to a reporter, whether a story is newsworthy, or how to handle a crisis: pitches, press releases, media lists, HARO, op-eds, podcast g
- **referrals** — When the user wants referral and affiliate programs or word of mouth, or asks what to reward, how much to pay, or which loop to start with: refer-a-friend incentives, commissions and payouts, partner
- **reverse-engineer-marketing** — When the user wants to know how another company grew and what to copy, or asks what a competitor did and when: a dated log of their moves against their traffic from public data, archives, job posts, r
- **revops** — When the user wants to design revenue operations, or asks when a lead is ready for sales or why sales and marketing disagree on lead quality: lifecycle stages, MQL and SQL, scoring, routing, handoff,
- **sales-enablement** — When the user wants sales collateral, or asks what to give a champion or rep, or how to answer an objection or make the case: decks, one-pagers, objection handling, demo scripts, talk tracks, battle c
- **schema** — When the user wants to add, fix, or validate schema markup (JSON-LD), or asks which type fits a page or whether some markup is allowed: product, review, recipe, FAQ, rich result eligibility (FAQ no lo
- **signup** — When the user wants better signup, registration, and trial start flows, or asks which fields or steps to cut or whether to require a card: SSO and drop-off before an account exists.
- **site-architecture** — When the user wants to plan or restructure a website, or asks which pages they need or where something belongs: hierarchy, navigation, URL structure, internal linking, website builders and migrations.
- **sms** — When the user wants SMS, MMS, or WhatsApp marketing, or asks whether texting is worth it or a message is allowed: abandoned cart, win-back, and promo texts, WhatsApp templates and windows, TCPA and A2
- **social** — When the user wants organic social content for LinkedIn, X, Instagram, TikTok, or Facebook, or asks what to post, which format to favor, or from which account: posts, threads, carousels, short-form vi
- **video** — When the user wants to create video with AI tools or code, or asks which approach to use (screen recording, AI avatar, generated, or templated) or how to make videos repeatably: product demos, AI gene

## JuliusBrussee/caveman

- **cavecrew** —  When to delegate to `cavecrew-investigator` (locate code), `cavecrew-builder` (1-2 file edit) or `cavecrew-reviewer` (diff review) instead of working inline or using `Explore`.
- **caveman-commit** —  Write a Conventional Commits message compressed to intent only.
- **caveman-explore** — Read-only repository explorer for cold-start orientation, broad cross-file localization, or when a direct search failed.
- **caveman-help** —  Quick-reference card for the three caveman skills and their commands.
- **investigate-first** — Diagnose ambiguous failures before editing.
- **lean-build** — Build feature work with high overbuilding risk.
- **migration** — Implement reversible compatibility-safe transitions.
- **safe-refactor** — Restructure code while preserving behavior.
- **surgical-patch** — Fix bugs and small behavior changes at the narrowest responsible layer.
- **verify-and-stop** — Prove existing work meets acceptance conditions without expanding scope.

## emilkowalski/skills

- **animate** — Build an animation from scratch, making the decisions in the order that determines whether it feels right — should it animate at all, what purpose, which tool, which properties, which curve and durati
- **animate-expo** — Build animations in React Native and Expo, making the decisions in the order that determines whether they feel right — should it animate, which thread it runs on, which properties, spring or timing, h
- **animation-performance** — Keep web animations inside the frame budget — animate composite-only properties, keep motion off the main thread when the page is busy, and find the cause when an animation drops frames.
- **animation-vocabulary** — Reverse-lookup glossary that turns a vague description of a web animation or motion effect into its exact term ("the bouncy thing when a popover opens" → Pop in; "the iOS rubber-band scroll" → Rubber-
- **apple-design** — Apple's approach to interface design and fluid, physical motion, translated for the web.
- **ask-sonner** — Guide to Sonner, the React toast library — install and wire up the Toaster, pick the right toast() call, promise and loading toasts, updating, dismissing and persisting toasts, styling, theming and ic
- **break-ui** — Try to break a piece of UI by feeding it worst-case data — long names, unbreakable emails, one-letter names, missing fields, huge counts, zero items, long labels, non-Latin text, emoji, extreme number
- **choreograph** — Coordinate several elements that animate together — decide what moves, in what order, how much they overlap, and what stays still — so a group reads as one intentional motion instead of a pile of sepa
- **emil-design-eng** — Emil Kowalski's design-engineering philosophy for UI polish, component design, animation decisions, and the invisible details that make software feel great.
- **find-animation-opportunities** — Search a codebase or UI for places that don't animate but should, and reject everything that shouldn't.
- **gestures** — Build drag, swipe, and sheet interactions that track the finger 1:1 and stay interruptible.
- **improve-animations** — Survey a codebase's animation and motion code as a senior motion advisor, then produce a prioritized audit and self-contained implementation plans for other agents (or cheaper models) to execute.
- **learn-taste** — Turn a session's design corrections into rules, with their why, in DESIGN.md, a project skill, or a lint check.
- **mobile-native** — Make a web app feel native on a phone — the small CSS and meta-tag fixes that separate "a website in a browser" from something that feels installed.
- **pick-ui-library** — Pick the library for a frontend task from a curated list — primitives, toasts, drawers, command menus, OTP, numbers, charts, virtualization, drag and drop, state, styling.
- **prototype** — Build multiple genuinely different versions of a UI piece you describe, rendered behind a visual picker so you can flip through them live and promote the one that feels right.
- **remove-animations** — Find the animations an interface doesn't need and remove them — motion on keyboard actions, high-frequency surfaces, functional data, and decoration with no purpose — then clean up what they leave beh
- **review-animations** — Reviews animation and motion code against a high craft bar derived from Emil Kowalski's design engineering philosophy.
- **write-swift** — How to write modern Swift well — modeling with value types, Swift 6 data-race safety and approachable concurrency (@concurrent, main-actor-by-default, actors, task groups), protocols and generics (som

## Other

claude-seo (its own hook), graphify, and anything installed by hand.

- **graphify** — Use for any question about a codebase, its architecture, file relationships, or project content — especially when graphify-out/ exists, where the question should be treated as a graphify query first.
- **seo** — Comprehensive SEO analysis for any website or business type.
- **seo-agentic** —  Audit and fix agent readiness: the Lighthouse Agentic Browsing fraction, accessibility tree for agents, robots.txt and Content-Signal for AI agents, WAF treatment of agent traffic, llms.txt, Markdown
- **seo-ahrefs** — Ahrefs API analyst (extension).
- **seo-audit** — Run a full-site SEO audit and return a scored, prioritized report.
- **seo-backlinks** — Analyze a site's backlink profile, anchors, toxic signals, competitors, gaps, and disavow candidates.
- **seo-bing** — Bing Webmaster Tools + IndexNow extension.
- **seo-cluster** —  Cluster keywords by SERP overlap and design hub-and-spoke content architecture with internal links.
- **seo-competitor-pages** —  Generate SEO-optimized competitor comparison and alternatives pages.
- **seo-content** —  Evaluate page content for usefulness, E-E-A-T, readability, thinness, and AI citation readiness, plus last-mile draft cleanup (AI-typical phrasing and invisible Unicode watermark characters).
- **seo-content-brief** —  Generate competitive SEO content briefs with per-section word counts, competitor scoring, keyword placement guidance, and page-type templates.
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
