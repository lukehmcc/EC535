savedcmd_mytimer.mod := printf '%s\n'   mytimer.o | awk '!x[$$0]++ { print("./"$$0) }' > mytimer.mod
