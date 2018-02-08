#!/bin/sh

is -silent -f mem_act.sql -o ../action_logs/mem_act.log.`date +%Y%m%d_%H%M%S`
