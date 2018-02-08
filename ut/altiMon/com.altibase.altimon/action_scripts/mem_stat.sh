#!/bin/sh

is -silent -f mem_stat.sql -o ../action_logs/mem_stat.log.`date +%Y%m%d_%H%M%S`
