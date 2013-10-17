# tests_dir=tests/
if [ $1 != "" ]
then
	tests_dir=$1
else
	tests_dir=tests/nasze/
fi

for i in `seq 1 30`
do
	if [ -e ${tests_dir}$i.in ]
	then
		./pociagi < ${tests_dir}$i.in 1>./${tests_dir}tmp.out 2>./${tests_dir}tmp.err
		diff -s -Bb ${tests_dir}$i.out ./${tests_dir}tmp.out || break
		diff -s -Bb ${tests_dir}$i.err ./${tests_dir}tmp.err || break
	else
		echo "No input for test $i"
	fi
done
