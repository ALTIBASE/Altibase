#!/bin/sh
# $1 : tag name (ex) 5_5_1_0_0
# $2 : tags name (ex) 5_5_1_0_8
# $3 : Target file to save. -- $(WORK_DIR)/APatch/pkg_patch_XX

branch_name=`svn info | grep URL |awk -F'altidev4' '{print $2}'`

echo "
select issue_type ||'-'|| issue_number ||' '||(select synopsis
		       from t_bug_master bug_master
		       where pk IN (issue_number)) synopsis 
from t_subversion_issue_relation
where branchname = '$branch_name'
and revision > (select copyrevision from t_subversion_changepath where path like '%$1%')
and revision <= (select copyrevision from t_subversion_changepath where path like '%$2%')
and issue_type = 'BUG'
order by issue_type, issue_number, revision;" > tmp_checkBugInfo.sql


isql -u mis -p mis -s support.altibase.in -port 8801 -silent -f tmp_checkBugInfo.sql -o bugList.tmp

echo "
============ Fixed Bug Info. ============" >> $3


# Cut our quary and result count
sed '1,11d' bugList.tmp | sed '$d' | awk -F'      ' '{print $1}' >> $3

# remove TMP files
rm -rf tmp_checkBugInfo.sql bugList.tmp

