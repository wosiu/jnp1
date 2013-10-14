for i in `seq 1 100`
do
	./pociagi < tests/$i.in 1>./tests/tmp.out 2>./tests/tmp.err
	diff -s -Bb tests/$i.out ./tests/tmp.out || break
	diff -s -Bb tests/$i.err ./tests/tmp.err || break
done
