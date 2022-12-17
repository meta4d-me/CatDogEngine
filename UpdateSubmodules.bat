REM if new submodule added, it can init it from remote.
git submodule init

REM grab latest commits from server
git submodule update --recursive --remote

REM above command will set current branch to detached HEAD. set back to master/main.
REM docking is a hack for imgui, need a if condition?
git submodule foreach "git checkout docking || git checkout master || git checkout main"

REM now do pull to fast-forward to latest commit
REM docking is a hack for imgui, need a if condition?
git submodule foreach "git pull origin docking || git pull origin master || git pull origin main"

pause