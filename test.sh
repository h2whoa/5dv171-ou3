#!/bin/bash

tmp_file="/tmp/test2G"

# drop_caches
# Tells the kernel to drop all cache pages used for storing VFS-related data.
#
# This is something you normally shouldn't do since it reduces performance
# by *a lot* but it is required in order to provide somewhat reliable test
# results with the iotest utility. For data preservation reasons, sync all
# data to disk before dropping the caches and wait 3 seconds.
drop_caches()
{
	sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches' && sleep 3
}

# Warn the user about the root privileges before running, let them bail out
# in case they don't feel right about this. This script is not meant to be
# executed for shits and giggles, it's mostly just available for reviewing
# purposes only.

echo "WARNING! You are about to run a test script that will generated a 2 GiB temporary file and requires root privileges in order to clear the kernel page cache."
echo "In other words, not a good idea unless if you like losing free space, wasting valuable time and using a painfully slow system afterwards."

echo "Would you like to abort the script? (Y/N)"
read user_input

if [ "$user_input" == "Y" ]; then
	echo "Stopping script..."
	exit 0
fi

# Create the temporary test file but *only* if it doesn't exist already.
# No point in rewriting something that already exists.

if [ ! -e $tmp_file ]; then
	echo "Generating 2 GiB test file at $TMP_PATH..."
	dd if=/dev/zero of=$tmp_file bs=1024 count=2M
fi

# Now, iterate through the exponential values and run each test accordingly.

for n in {0..6}; do
	threads=$(awk "BEGIN{print 2^${n}}")
	echo "${threads} threads..."

	for x in {12..24}; do
		blk_size=$(awk "BEGIN{print 2^${x}}")
		echo "${blk_size} bytes per block..."

		for rep in {1..3}; do
			drop_caches
			./iotest -n ${threads} -t seq -B 100 -b ${blk_size} $tmp_file >> "logs/seq_${blk_size}_${threads}.log"

			drop_caches
			./iotest -n ${threads} -t rand -B 100 -b ${blk_size} $tmp_file >> "logs/rand_${blk_size}_${threads}.log"
		done
	done
done
