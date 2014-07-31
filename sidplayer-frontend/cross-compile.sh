export GOARCH="arm"
# defaults to GOARM 6, which equals RPi
#export GOARM="5"
export GOOS="linux"
go build -o ../build/sidplayer-frontend/sidplayer-frontend

