#!/usr/bin/env bash
# Summarise compiler warnings from a build log: unique project warnings (JUCE, submodules and
# fetched dependencies excluded) go to the job summary and, for the first 50, to inline
# annotations. Never fails the job. Usage: summarize-warnings.sh <build.log> <label>
set -uo pipefail

log="${1:-build.log}"
label="${2:-build}"
summary="${GITHUB_STEP_SUMMARY:-/dev/null}"

if [ ! -s "$log" ]; then
  echo "No build log at $log; nothing to summarise."
  exit 0
fi

# gcc/clang: path:line:col: warning: msg      MSVC: path(line,col): warning C4244: msg [project]
# Normalise backslashes, strip the checkout prefix, drop third-party paths, de-duplicate.
tr '\\' '/' < "$log" \
  | grep -E ':[0-9]+(:[0-9]+)?: warning:|\([0-9]+(,[0-9]+)?\): warning C[0-9]+' \
  | sed -E 's# \[[A-Za-z]:/[^]]*\]$##' \
  | grep -vE '/(JUCE|art|_deps|build[^/]*)/|/usr/(include|lib)/|\.sdk/|libtool:|lto-wrapper' \
  | sed -E 's#^.*/GenerativeMIDI/GenerativeMIDI/##' \
  | sed -E 's#^([^:(]+)\(([0-9]+)(,[0-9]+)?\): warning (C[0-9]+): (.*)$#\1:\2: warning: \5 [\4]#' \
  | sed -E 's#^([^:]+):([0-9]+)(:[0-9]+)?: warning: (.*)$#\1\t\2\t\4#' \
  | sort -u > /tmp/warnings.tsv

count=$(wc -l < /tmp/warnings.tsv | tr -d ' ')

{
  echo "### Compiler warnings: ${label}"
  if [ "$count" -eq 0 ]; then
    echo "None in project sources."
  else
    echo "${count} unique warning(s) in project sources (third-party code excluded)."
    echo
    echo "| File | Line | Warning |"
    echo "|---|---|---|"
    head -n 100 /tmp/warnings.tsv | awk -F'\t' '{ gsub(/\|/, "\\|", $3); printf "| `%s` | %s | %s |\n", $1, $2, $3 }'
    [ "$count" -gt 100 ] && echo && echo "(first 100 of ${count} shown)"
  fi
} >> "$summary"

head -n 50 /tmp/warnings.tsv | awk -F'\t' '{ printf "::warning file=%s,line=%s,title=Compiler warning::%s\n", $1, $2, $3 }'
echo "${label}: ${count} unique project warning(s)."
exit 0
