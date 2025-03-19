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

echo`irsdkcpp version v${pkgVersion} - Releasing`

async function checkReleaseDraftValid() {
  echo`Checking Github Draft Release v${pkgVersion} exists`
  const releaseInfoOutput = await $`gh release list --json "name,isDraft,tagName" -q '[.[] | select(.name == "${pkgVersion}")]'`,
    releaseInfoJsonStr = releaseInfoOutput.stdout,
    releaseInfoJson = JSON.parse(releaseInfoJsonStr)
  
  echo`Release Info for ${versionTag}:\n${releaseInfoJsonStr}`
  
  assert(
    releaseInfoJson?.[0]?.isDraft === true,
    `Release is not marked as a draft (${versionTag})`)
}

async function releaseDraft() {
  echo`Updating Github Release v${pkgVersion} to production channel`
  await $`gh release edit ${versionTag} --draft=false --latest`
}

async function pushMaster() {
  try {
    // Push the changes to master (with --force due to rebase)
    await gitExec`git push --force`
  } catch (error) {
    fatalError(`Error during push: ${error.message}`)
  }
}

async function releaseVersion() {
  await checkReleaseDraftValid()
  await pushMaster()
  await releaseDraft()
}

releaseVersion()
  .catch(err => {
    log.error(`Error occurred: ${err?.message}`, err)
    fatalError(err.message)
  })

