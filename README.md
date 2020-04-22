# dwm-status

C implementation of my dwm status script (example output: `"load:0.01 | Wed 22 Apr 2020 16:14"`)

Runs in a loop and updates the root window's name every minute (precise to 1 ms).

Reacts to signals:
 - SIGHUP: update status now and reset the waiting timer (have this sent when waking from suspend state):

`kill -HUP $(pidof dwm_status)`
 - SIGTERM/SIGINT: exit cleanly

Using it reduced the minimum load on my ThinkPad from 0.18 to 0.00
