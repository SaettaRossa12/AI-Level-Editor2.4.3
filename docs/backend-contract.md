# Backend Contract

This mod should call a backend for every paid or abuse-sensitive action.

## Product

- Price: 5 EUR.
- The server stores purchase entitlement by user account or license key.
- The client never stores the AI provider key.
- The mod setting `purchase-url` should point to the Gumroad page where users pay you.
- The mod setting `license-key` is only a placeholder until a backend verifies real purchases.
- The mod setting `owner-gd-username` identifies the creator account that receives unlimited natural prompts.
- The configured owner account is `saettarossa2503`.
- The support email is `uwhahshe@gmail.com`.
- Gumroad payments should be configured on the seller account `Fra.grasso77`.

## Natural Prompt Limit

- Allow 3 natural-language generations per calendar month.
- The server computes the month window in UTC.
- The client can display remaining uses, but the server response is authoritative.

## Temporary Ban

- If the server detects limit tampering, patched integrity fields, replayed usage tokens, or modified signed config, apply a 14-day ban.
- During the ban, script mode can be allowed or blocked depending on product policy. Natural prompt mode should be blocked.

## Suggested Endpoints

### `POST /v1/session/check`

Request:

```json
{
  "licenseKey": "user-provided-key",
  "modVersion": "v0.1.0",
  "gdVersion": "2.2074",
  "integrity": {
    "clientBuild": "winza.levelscribe-ai/v0.1.0",
    "settingsHash": "hex",
    "binaryHash": "hex-or-empty-if-unavailable"
  }
}
```

Response:

```json
{
  "entitled": true,
  "naturalPromptsRemaining": 3,
  "bannedUntil": null,
  "serverTime": "2026-04-30T12:00:00Z"
}
```

### `POST /v1/generate/script`

Takes structured script text and returns normalized object commands.

### `POST /v1/generate/natural`

Consumes 1 monthly natural prompt if successful. Returns normalized object commands.

## Anti-Cheat Position

Client-side anti-cheat is only a signal. It can detect obvious local changes, but cannot reliably punish users by itself. Real enforcement must be server-side and based on signed server state.
