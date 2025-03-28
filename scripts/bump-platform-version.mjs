#!/usr/bin/env node
// noinspection JSCheckFunctionSignatures,JSUnresolvedReference

import assert from "node:assert/strict"
import * as semver from "semver"
import { $, argv, cd, echo, path as Path, fs as Fsx } from "zx"
import { isGitDirty } from "./common/git-helpers.mjs"
import { rootDir } from "./common/workflow-global.mjs"

const log = console

cd(rootDir)

assert(!await isGitDirty(), "Git repo is dirty, can not bump version")

const versionInc = argv["inc"] ?? "patch",
  pkgFile = Path.join(rootDir, "package.json"),
  pkgJson = Fsx.readJSONSync(pkgFile),
  { version: currentPkgVersion } = pkgJson,
  vcpkgFile = Path.join(rootDir, "vcpkg.json"),
  vcpkgJson = Fsx.readJSONSync(vcpkgFile),
  currentVCPKGVersion = vcpkgJson.version,
  cmakeVersionFile = Path.join(rootDir, "version.txt"),
  currentCMakeVersion = Fsx.readFileSync(cmakeVersionFile, "utf-8")

assert(currentCMakeVersion === currentPkgVersion, `CMake Version ${currentCMakeVersion} does not match pkg version ${currentPkgVersion}`)

const
  newVersion = semver.inc(currentPkgVersion, versionInc)

echo`Updating version (${currentPkgVersion} -> ${newVersion}) for irsdkcpp`

async function updatePackageJson() {
  echo`Processing ${pkgFile} (${pkgJson.version} -> ${newVersion})`

  pkgJson.version = newVersion

  Fsx.writeJSONSync(pkgFile, pkgJson, {
    spaces: 2
  })
  
  await $`git add ${Path.relative(rootDir, pkgFile)}`
}

async function updateCMakeVersion() {
  echo`Updating CMake version (${currentCMakeVersion} -> ${newVersion})`
  
  Fsx.writeFileSync(cmakeVersionFile, newVersion)
  
  await $`git add ${Path.relative(rootDir, cmakeVersionFile)}`
}

async function updateVCPKGVersion() {
  echo`Updating VCPKG version (${currentVCPKGVersion} -> ${newVersion})`
  
  vcpkgJson.version = newVersion
  Fsx.writeJSONSync(vcpkgFile, vcpkgJson, {
    spaces: 2
  })
  
  await $`git add ${Path.relative(rootDir, vcpkgFile)}`
}

await updatePackageJson()
await updateCMakeVersion()
await updateVCPKGVersion()

await $`git commit -m "bumped version ${currentPkgVersion} -> ${newVersion}"`
