#!/bin/sh

# This script scan all pdf files in a directory given in parameter
# return the results in another file given as second parameter
# the stats are stored in the stat.txt file

# check parameters

# VARIABLES
DIR=$1
RES_FILE=$2
EXE=/home/ulrich/PDF/GIT/uhurupdf/module7/uhuruPDFAnalyzer

for f in $DIR/* ; do
	echo "Processing $f ...";
	$EXE "$f" >> $RES_FILE
	#mv "$f" $DIR/Treated/
done


# Stats coef
more $RES_FILE | grep -e 'Coef =' | sort | uniq -c > stats.txt


exit 0;
