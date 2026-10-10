## What and why

<!-- One or two sentences. Link the issue if there is one. -->

## How it was checked

- [ ] Built and ran `ctest` locally (`cmake --build build --config Release && (cd build && ctest -C Release)`)
- [ ] Docs updated if behaviour, counts or CI changed (STATUS.md, README.md, docs/)

## CI cost

Open this PR as a **draft** while you are still pushing commits; CI does not run on drafts and starts when it is marked ready. Add the `full-ci` label only if iOS or Windows should run on a change that does not touch build files.
