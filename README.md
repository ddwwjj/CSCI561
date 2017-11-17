CSCI 561: Foundations of Artificial Intelligence-USC-2017 Fall

* HW1 [Search(BFS + DFS + SA)](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework1.cpp)

**Optimization method: **

Given a nursery with some tree and lizards in it and given a new lizard location, how to efficiently calculate the conflicts. I first created four segmented 2D array based on the original nursery(0 or more trees, 0 lizards) and the policy. Locations that belong to the same segment area should not have more than one lizard.

![segmentation](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/segmentation.PNG)





* HW2 [Game playing](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework2.cpp)

* HW3 [FOL using Resolution](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework3.java)

**Optimization method:**

Given a query, how to efficiently find all candidate sentences that could unify with query?

I maintained a **predicatemap** HashMap which save all mapping from **predicate(key)** to **sentence index(value)** so that I could locate sentences faster using advantage of Hash.

![resolution process](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/Resolution_Process.PNG)
