# Security Policy

### In short: This is a color library, not an arena for your war games.

## Supported Versions

Security updates are applied only to the development head. Reports must reproduce the issue on the current development head.

## Basic rules

LittleCMS is used deep in the Linux dependency tree. Real security issues should be addressed. 

**Submit reports through [the contact page](https://www.littlecms.com/contact). GitHub security advisories are deleted without review because of the high volume of spam received.**

Keep in mind that misusing the API does not by itself make it a vulnerability. It may just be you doing things wrong. C itself is unsafe. For example, it is not possible to reliably check whether a passed pointer is valid.

## Reporting a Vulnerability

A report should include:

- A proof of concept (PoC), if available, including a suitable input file that demonstrates the issue.
- A short explanation of the issue and how to reproduce it.
- A proposed patch, if you have one, with a short explanation of the solution.

AI-generated security reports without a suitable input file PoC will be deleted immediately. AI-generated text walls are generally disregarded.

## Please

- **Please avoid public advisories, as doing that, hints how to use the flaw for malicious use**.

- Please don’t make noise to gain popularity, this can result in bad karma to you. Any CVE without reliable proof will be promptly disputed.

- Please don’t use untrusted patches from 3rd parties. We had an incident years ago with so called “security experts” trying to add a back door by using a crafted patch.

Credits to vulnerability busters will be given on each release. AI-generated reports will be credited exclusively to the AI


