#!/bin/sh

is -silent -f logfile_act.sql -o ../action_logs/logfile_act.log.`date +%Y%m%d_%H%M%S`
