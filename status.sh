#!/bin/zsh
SEPARATOR="$(echo "\u239C")"

update_status() {
    LOAD="load:$(uptime | cut -d, -f3 | cut -d: -f2 | tr -d ' ')"
    DATE="$(date '+%a %d %b %H:%M')"
    STATUS="$LOAD $SEPARATOR $DATE"

	echo "$STATUS"
    xsetroot -name "$STATUS"
}

if [ -n "$1" ]; then
	update_status
	exit 0
fi

update_status
SLEEPSECONDS="$(($(date -d "next minute" '+%s') / 60 * 60 - $(date -d "now" +%s) + 0.05))"
#$HOME/bin/dwm_sleep $SLEEPSECONDS
