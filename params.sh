p=(131 179 233 277 293)
m=(21845 32767 43691 55831 65537 92837 90751 104353)
echo "Testing Security Params"
for i in ${!p[@]}; do
	for j in ${!m[@]}; do
		echo "--------"${p[$i]} ${m[$j]}"--------"
		./bin/Panther 0 ${p[$i]} ${m[$j]} > ./data/params/logs/run-${p[$i]}-${m[$j]}.log
	done	
done
echo "Done all"