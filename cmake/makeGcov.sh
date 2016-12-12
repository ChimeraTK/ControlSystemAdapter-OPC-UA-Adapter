#!/bin/bash

echo "Starte alle Testprogramme..."

#cd $1/tests

if [ -d $1/cov ]; then
  rm -R $1/cov
fi

mkdir $1/cov

for file in $1/tests/test_* ; do 
	echo "Starte: $file"
	$file
	echo "Fertig."
done

echo "Alle gefundenen Testdateien wurden ausgeführt"


lcov --no-external --directory $1 --capture --output-file $1/tests/app.info
genhtml --output-directory $1/cov $1/tests/app.info

