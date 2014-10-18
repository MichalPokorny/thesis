#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/errno.h>

#include "hash_array/hash_array.h"
#include "hash_hashtable/hash_hashtable.h"
#include "hash_bplustree/hash_bplustree.h"

#include "log/log.h"

#include "performance/random_read.h"

int test_mmap();

void run_performance_tests() {
	time_random_reads(&hash_array, 1000, 10000);

	time_random_reads(&hash_array, 10000, 100000);

	time_random_reads(&hash_hashtable, 10000, 200000);
	time_random_reads(&hash_hashtable, 20000, 200000);
	time_random_reads(&hash_hashtable, 50000, 200000);
	time_random_reads(&hash_hashtable, 100000, 200000);
	time_random_reads(&hash_hashtable, 200000, 200000);
	time_random_reads(&hash_hashtable, 1000000, 2000000);
	time_random_reads(&hash_hashtable, 2000000, 10000000);
	time_random_reads(&hash_hashtable, 5000000, 10000000);
	// time_random_reads(&hash_hashtable, 10000000, 20000000);
	// time_random_reads(&hash_hashtable, 20000000, 100000000);
	// time_random_reads(&hash_hashtable, 50000000, 100000000);

	time_random_reads(&hash_bplustree, 10000, 200000);
	time_random_reads(&hash_bplustree, 20000, 200000);
	time_random_reads(&hash_bplustree, 50000, 200000);
	time_random_reads(&hash_bplustree, 100000, 200000);
	time_random_reads(&hash_bplustree, 200000, 200000);
	time_random_reads(&hash_bplustree, 1000000, 2000000);
	time_random_reads(&hash_bplustree, 2000000, 10000000);
	time_random_reads(&hash_bplustree, 5000000, 10000000);
	time_random_reads(&hash_bplustree, 10000000, 10000000);
	time_random_reads(&hash_bplustree, 20000000, 10000000);
	time_random_reads(&hash_bplustree, 50000000, 10000000);

//	if (test_mmap()) return 1;
}

int main(int argc, char** argv) {
	(void) argc; (void) argv;

	run_performance_tests();
}

int test_mmap() {
	// 4 GB
	// const size_t size = 1024 * 1024 * 1024;
	const size_t size = 4L * 1024L * 1024L * 1024L;
	void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (memory == MAP_FAILED) {
		log_error("mmap returned MAP_FAILED");
		switch (errno) {
			case EINVAL: log_error("EINVAL"); break;
			case EACCES: log_error("EACCES"); break;
			case ENOMEM: log_error("ENOMEM"); break;
			case ENODEV: log_error("ENODEV"); break;
			case ENOEXEC: log_error("ENOEXEC"); break;
		}
		return 1;
	}

	log_info("mmap'd ptr %p", memory);

	for (int i = 0; i < 5; i++) {
		for (size_t i = 0; i < size; i++) {
			((char*) memory)[i] = i;
		}
	}

	if (munmap(memory, size) != 0) {
		log_error("munmap failed");
		return 1;
	}
	return 0;
}
