REM grab latest commits from server
git submodule update --recursive --remote

REM above command will set current branch to detached HEAD. set back to master.
git submodule foreach "git checkout master || git checkout main"

REM now do pull to fast-forward to latest commit
git submodule foreach "git pull origin master || git pull origin main"