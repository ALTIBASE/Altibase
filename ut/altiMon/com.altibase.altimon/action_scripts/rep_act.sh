#!/bin/sh

is -silent -f rep_act.sql -o ../action_logs/rep_act.log.`date +%Y%m%d_%H%M%S`
