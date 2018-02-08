#!/bin/sh

is -silent -f cpu_act.sql -o ../action_logs/cpu_act.log.`date +%Y%m%d_%H%M%S`
