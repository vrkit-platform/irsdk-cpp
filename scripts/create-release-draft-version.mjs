#!/usr/bin/env node
// noinspection JSCheckFunctionSignatures,JSUnresolvedReference
// noinspection JSCheckFunctionSignatures
import assert from "node:assert/strict"
import { $, cd, echo, fs as Fsx, path as Path } from "zx"
import { fatalError } from "./common/process-helpers.mjs"
import { rootDir } from "./common/workflow-global.mjs"

const log = console

cd(rootDir)

const pkgFile = Path.join(rootDir, "package.json"),
  pkgDirs = rootDir,
  pkgJson = Fsx.readJSONSync(pkgFile),
  pkgVersion = pkgJson.version,
  versionTag = `v${pkgVersion}`

const gitExec = $({
  cwd: rootDir
})

echo`irsdkcpp version v${pkgVersion} - Create/Update release draft`

async function updateReleaseDraft() {
  echo`Checking Github Draft Release v${pkgVersion} exists`
  const releaseInfoOutput = await $`gh release list --json "name,isDraft,tagName" -q '[.[] | select(.name == "${versionTag}")]'`,
    releaseInfoJsonStr = releaseInfoOutput.stdout,
    releaseInfoJson = JSON.parse(releaseInfoJsonStr)
  
  const
    relFound = (releaseInfoJson?.length ?? 0) > 0,
    isDraft = releaseInfoJson?.[0]?.isDraft === true,
    relArgs = `${versionTag} --title ${versionTag} --draft --target=develop`.split(" ")
  
  assert(!relFound || isDraft, `Only draft releases can be updated`)
  if (isDraft) {
    echo`Updating Release Draft Info for ${versionTag}`
    await $`gh release edit ${relArgs} --tag ${versionTag}`
  } else {
    echo`Creating Release Draft Info for ${versionTag}`
    await $`gh release create ${relArgs} --generate-notes`
  }
}


updateReleaseDraft()
  .catch(err => {
    log.error(`Error occurred: ${err?.message}`, err)
    fatalError(err.message)
  })

