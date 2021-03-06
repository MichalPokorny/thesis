set term pdf
set output "graph-random.pdf"
set key inside
set xlabel 'Van Emde Boas tree height'
set ylabel 'ns'
set ytics
plot "results-random.csv" u 1:($3/$2) w lines title 'Time per lookup'

set output "veb-drilldown-speed.pdf"
plot "results-drilldown.csv" u 1:($3/$2) w lines title 'Root-to-leaf traversal, recursive', \
	"results-hyperdrilldown.csv" u 1:($3/$2) w lines title 'Root-to-leaf traversal, with precomputed level data'
