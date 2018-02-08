$(filter-out msg,$(DIRS)): msg

altiProfile audit utm checkServer iloader3 isql altiWrap: util

audit checkServer: iloader3

iloader3 isql sample altiWrap: libedit

package: altiWrap

#audit checkServer iloader3 sample unittest: lib util
