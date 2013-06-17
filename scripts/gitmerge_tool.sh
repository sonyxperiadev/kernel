#!/bin/bash
# Purpose of this script would be
#  1) Generate a list of commit ids that needs merge from source branch.
#  2) Cherry pick each commit id from the list to target branch.
#  3) Stop in case of conflicts, and continue when conflicts are resolved.
#


SOURCE_BRANCH="android-2.6.35"
TAREGT_BRANCH="android-3.0"
COMMIT_ID=""
NUM_SKIP=0
GEN_SKIP=0
TMP_FILE=".merge_commit"

set -- `getopt -n$0 -u -a --longoptions="commitid: numskip: genskip:: continue:: version:: help::" "h" "$@"`

while [ $# -gt 0 ]; do
  case "$1" in
	--commitid)
		COMMIT_ID=$2
		shift 2
		;;
	--numskip)
		NUM_SKIP=$2
		shift 2
		;;
	--genskip)
		GEN_SKIP=1
		shift 1
		;;
	--continue)
		NUM_SKIP=1
		shift 1
		;;
	--version)
		echo "$0 script version is $VERSION"
		exit;;
	--help)
		echo ""
		echo "Broadcom merge tool utility (ver. $VERSION). Broadcom, Inc. (C) 2011."
		echo ""
		echo "Examples:"
		echo "script/gitmerge_tool.sh --commitid <commitid>"
		exit;;
    --)
		break
		;;
    -*)
		shift 1
		;;
    *)
		shift 1
		break
		;;
  esac
done


if [ -f $TMP_FILE ]; then
	COMMIT_ID=`head -n 1 $TMP_FILE`
else
	GEN_SKIP=0
fi

if [ "$COMMIT_ID" != "" ]; then
	if [ "$GEN_SKIP" == 0 ]; then
		echo "git checkout $SOURCE_BRANCH"
		git checkout $SOURCE_BRANCH
		echo "git rev-list --reverse $COMMIT_ID..HEAD > $TMP_FILE"
		git rev-list --reverse $COMMIT_ID..HEAD > $TMP_FILE
		echo "git checkout $TAREGT_BRANCH"
		git checkout $TAREGT_BRANCH
	fi
else
	echo "Commit id not provided"
	exit 1
fi

if [ $NUM_SKIP -gt 0 ]; then
	sed -i 1,"$NUM_SKIP"d $TMP_FILE
fi

for I in `cat $TMP_FILE`
do
	echo "git cherry-pick $I"
	git cherry-pick $I
	if [ "$?" != 0 ]; then
		echo "Error: Resolve the conflict, commit the changes and run the same script with --continue option"
		echo "git status"
		git status
		exit 1
	fi

	sed -i 1d $TMP_FILE
done	
