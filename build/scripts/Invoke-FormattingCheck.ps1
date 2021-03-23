
#.SYNOPSIS
# Checks for code formatting errors. Will throw exception if any are found.
function Invoke-CheckBadCodeFormatting() {
    Import-Module ./tools/OpenConsole.psm1
    Invoke-CodeFormat

    # returns a non-zero exit code if there are any diffs in the tracked files in the repo
    git diff-index --quiet HEAD --
    if ($lastExitCode -eq 1) {
        # Write the list of files that need updating to the log
        $changedFiles = git diff-index --name-only HEAD
        Write-Error $changedFiles
        throw "code formatting bad, run Invoke-CodeFormat on branch"
    }
}

Invoke-CheckBadCodeFormatting
