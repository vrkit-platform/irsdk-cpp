import { $ } from "zx"

const log = console

export async function isGitDirty() {
  try {
    // Run the Git status command to check for uncommitted changes
    const { stdout } = await $`git status --porcelain`
    
    // If there's any output, the repository is dirty
    if (stdout.trim() !== "") {
      log.info("Repository is dirty")
      return true
    } else {
      log.info("Repository is clean")
      return false
    }
  } catch (err) {
    log.error("Error checking Git status:", err.message)
    return false
  }
}
