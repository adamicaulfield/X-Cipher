# Basic tests
dim=(5 7 11 13)
mults=(12 9 5 4)
echo "Running Tests on Encode, Extract Tag, Verify, Rotate"
for i in ${!dim[@]}; do
	for ((iter = 0; iter < 25; iter++)); do
		echo "--------" $iter ${dim[$i]} "--------"
		./bin/Panther 1 ${dim[$i]} > ./data/timing/logs/run-simpleVerify-1-${dim[$i]}-$iter.log
	done
done
echo "Done all"

# # Recovery Tests
# echo "Running Tests on One Column Recovery, Two Col Recovery"
# for i in ${!dim[@]}; do
# 	for (( c1 = 0; c1 < dim[i]-1; c1++ )); do
# 		for (( c2 = c1+1; c2 < dim[i]; c2++ )); do
# 			for iter in {1..2..1}; do
# 				echo "--------" $iter ${dim[$i]} $c1 $c2 "--------"
# 				./bin/Panther 2 ${dim[$i]} $c1 $c2 > ./data/timing/logs/run-2-${dim[$i]}-$iter.log
# 			done
# 		done
# 	done
# done
# echo "Done all"

# # PSI Tests
# n_S=(50 100 200 500)
# div=(8 4 2)
# echo "Running Test for PSI"
# for i in ${!n_S[@]}; do
# 	for j in ${!div[@]}; do
# 		for iter in {1..2..1}; do
# 			let "n_R = ${n_S[$i]} / ${div[$j]} "
# 			echo "-------- iter = " $iter "n_S =" ${n_S[$i]} "n_R =" $n_R "--------"
# 			./bin/Panther 4 ${n_S[i]} $n_R > ./data/timing/logs/run-${n_S[i]}-$n_R-$iter.log
# 		done
# 	done
# done

# # Matrix Ops Tests
# d=(3 4 5 6)
# echo "Running Test for Matrix Ops"
# for i in ${!d[@]}; do
# 		for iter in {1..10..1}; do
# 			echo "-------- iter = " $iter "d =" ${d[$i]} "--------"
# 			./bin/Panther 5 ${d[i]} > ./data/timing/logs/run-${d[i]}-$iter.log
# 		done
# done