dim=(9)
echo "Running Tests on One Column Recovery, Two Col Recovery"
for i in ${!dim[@]}; do
	for (( c1 = 0; c1 < dim[i]-1; c1++ )); do
		for (( c2 = c1+1; c2 < dim[i]; c2++ )); do
			echo "--------" ${dim[$i]} $c1 $c2 "--------"
			./bin/testPanther ${dim[$i]} 2 $c1 $c2 > ./data/tests/logs/tests-${dim[$i]}-$c1-$c2.log
		done
	done
done
echo "Done all"