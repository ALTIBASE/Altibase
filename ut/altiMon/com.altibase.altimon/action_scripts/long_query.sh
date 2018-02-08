#!/bin/sh

is -silent -f long_query.sql -o ../action_logs/long_query.log.`date +%Y%m%d_%H%M%S`
