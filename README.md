CSCI 561: Foundations of Artificial Intelligence-USC-2017 Fall

* HW1 [Search(BFS + DFS + SA)](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework1.cpp)

* HW2 [Game playing](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework2.cpp)

* HW3 [FOL using Resolution](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/homework3.java)

**Optimization method: Given a query, how to efficiently find all candidate sentences that could unify with query?**

I maintained a **predicatemap** HashMap which save all mapping from **predicate(key)** to **sentence index(value)** so that I could locate sentences faster using advantage of Hash.

![resolution process](https://github.com/ddwwjj/CSCI561-Artificial-Intelligence/blob/master/Resolution_Process.PNG)
