#!/bin/bash
./app_sign decrypt ticket.key `defaults read com.anscamobile.ratatouille Resource` | awk '{printf("hash of mac: %s\ntimestamp: %s\nsubType: %s\nlic type: %s  --not used\nplatform type: %s  --not used\nsub-active: %s  --not used\nuser-id: %s\nemail: %s\n",$1,$2,$3,$4,$5,$6,$7,$8)}' -
