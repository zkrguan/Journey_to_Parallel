# False sharing

Naive is a perfect example of false sharing.

Padded version used the hardware optimization knowledge to fill the array around the actual data. So "sloshing around the cache line" will not happen.

Sync is the version we use the most in open MP to write once a thread by using critical.