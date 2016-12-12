#!/bin/bash

echo "Starte alle Testprogramme..."

#cd $1/tests

if [ -d $1/build/cov ]; then
  rm -R $1/build/cov
fi

mkdir $1/build/cov

for file in $1/build/tests/test_* ; do 
	echo "Starte: $file"
	$file
	echo "Fertig."
done

echo "Alle gefundenen Testdateien wurden ausgeführt"


lcov --no-external --directory $1 --capture --output-file $1/build/tests/app.info
genhtml --output-directory $1/build/cov $1/build/tests/app.info

