btree.pdf: generated by experiments/btree-dot at 327f53c, with overrides:
		#define LEAF_MAX_KEYS 5
		#define LEAF_MIN_KEYS 2
		#define INTERNAL_MAX_KEYS 5
		#define INTERNAL_MIN_KEYS 2
veb-drilldown-speed.pdf: generated by experiments/veb_performance at e5961b3

cloud/*: generated at b032ead via:
	bin/experiments/cloud -a dict_splay -s 100000 \
		--min_year=1997 --max_year=1998 --dump_averages

cuckoo/results.pdf: generated at d6df442c via:
	bin/experiments/cuckoo-cube
	cd experiments/cuckoo-cube
	gnuplot plot_results.gnuplot
	pdflatex results.tex

performance/: exported by export_images.sh, plotted by plot_results.py

ksplay-noalloc-insert.png, ksplay-noalloc-find-50.png:
	generated by no-alloc version of K-splay
