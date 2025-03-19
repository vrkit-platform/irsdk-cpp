#!/usr/bin/env node
// noinspection JSCheckFunctionSignatures,JSUnresolvedReference
// noinspection JSCheckFunctionSignatures

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
  const releaseInfoOutput = await $`gh release list --json "name,isDraft,tagName" -q '[.[] | select(.name == "${pkgVersion}")]'`,
    releaseInfoJsonStr = releaseInfoOutput.stdout,
    releaseInfoJson = JSON.parse(releaseInfoJsonStr)
  
  const isDraft = releaseInfoJson?.[0]?.isDraft === true
  if (isDraft) {
    echo`Updating Release Draft Info for ${versionTag}`
    await $`gh release edit ${versionTag} --draft --target=develop`
  } else {
    echo`Creating Release Draft Info for ${versionTag}`
    await $`gh release create ${versionTag} --draft --target=develop`
  }
}


updateReleaseDraft()
  .catch(err => {
    log.error(`Error occurred: ${err?.message}`, err)
    fatalError(err.message)
  })

