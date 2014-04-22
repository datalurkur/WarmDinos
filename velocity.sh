gnuplot -e "set term png; set output \"test.png\"; set grid ytics; plot \"movement.dat\" using 1:3 title 'v' with points, \"movement.dat\" using 1:4 title 'a' with points"
