#!/bin/bash

echo "Starte alle Testprogramme..."

cd $1/tests

if [ -d $1/cov ]; then
  rm -R $1/cov
fi

mkdir $1/cov

for file in ./test_* ; do 
	echo "Starte: $file"
	$file
	echo "Fertig."
done

echo "Alle gefundenen Testdateien wurden ausgeführt"

cd ..
cd ..

lcov --no-external --directory ./ --capture --output-file $1/tests/app.info
genhtml --output-directory $1/cov $1/tests/app.info

