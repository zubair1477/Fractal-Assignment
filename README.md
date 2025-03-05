Zubair Rashaad
1002051693

*Evaluation Report*

Purpose: The purpose of this experiment is to implement and test the effect that multi-threading has on the execution time of this Mandlebrot generator. We used different numbers of threads(1,2,3,4,5,10,50) to see how the parallelism affacted the performance.

Problem Question: Does increasing the number of threads consistently lead to faster execution time?

Hypothesis: Yes, it will lead to faster execution time.

Original output: mandel: x=0.000000 y=0.000000 scale=4.000000 max=1000 outfile=mandel.bmp
![alt text](image-1.png)

Output after modifications: x=0.000000 y=0.000000 scale=4.000000 max=1000 outfile=mandel.bmp
![alt text](image-2.png)

Test Results:
![alt text](<Screenshot 2025-03-04 at 11.26.52 PM-1.png>)

We can see that after adding more than 1 thread, the execution time dropped significantly. This was because the workload per thread was reduced.

For Configuration A, the execution time kept decreasing up to about 4 threads, then the improvements slowed down.
For Configuration B, the execution time flattened a little earlier, around 2-3 threads.

In my case, adding too many threads didn't significantly improve the performance, which may have been because of my CPU core count or because of threading overhead.

Optimal Thread count:
Configuration A: 4 threads
Configuration B: 2 threads
-Adding more threads cancels out the benefits as the improvement slowed down.

Shape:
In configuration A, the work is more evenly distributed so multi threading was more effective.
In configuration B, there might have been some thread imbalance, causing some threads to do more work while others finished early.

Command Line Arguments:
mandel -x -.5 -y .5 -s 1 -m 2000 -n X
mandel -x 0.2869325 -y 0.0142905 -s .000001 -W 1024 -H 1024 -m 1000 -n X

*replace X with the number of threads