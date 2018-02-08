#!/bin/sh

is -silent -f db_usage.sql -o ../action_logs/db_usage.log.`date +%Y%m%d_%H%M%S`
