# KnapSack Problem 背包问题


### KnapSack 背包问题
#### Definiton 定义

<u>背包问题</u>是一种组合优化的NP完全问题:有N个物品和容量为W的背包，每个物品都有自己的体积w和价值v，求拿哪些物品可以使得背包所装下的物品的总价值最大。如果限定每种物品只能选择0个或者1个，则称问题为<u>0-1背包问题</u>;如果不限定每种物品的数量，则问题称为<u>无界背包问题和或者完全背包问题</u>。

#### 0-1 背包问题

以 0-1 背包问题为例。我们可以定义一个二维数组 `dp` 存储最大价值，其中<u> `dp[i][j]` 表示前 `i` 件物品体积不超过 `j` 的情况下能达到的最大价值</u>。在我们遍历到第 `i` 件物品时，在当前背包总容量为 `j` 的情况下，如果我们不将物品 `i` 放入背包，那么 `dp[i][j]
= dp[i-1][j]`，即前 `i` 个物品的最大价值等于只取前 `i-1` 个物品时的最大价值；如果我们将物品 `i` 放入背包，假设第 `i` 件物品体积为 `w`，价值为 `v`，那么我们得到 `dp[i][j] = dp[i-1][j-w] + v`。我们只需在遍历过程中对这两种情况取最大值即可，总时间复杂度和空间复杂度都为 `O(NW)`。

```c++
int knapsack(vector<int>& weights, vector<int>& values, int N, int W) {
    vector<vector<int>> dp(N+1, vector<int> (W+1, 0));
    for (int i = 1; i <=N; ++i) {
        int w = weight[i-1], v = values[i-1];
        for (int j = 1; j <= W; ++j) {
            if (j >= w) {
                dp[i][j] = max(dp[i-1][j], dp[i-1][j-w] + v);
            } else {
                dp[i][j] = dp[i-1][j];
            }
        }
    }
    return dp[N][W];
}
```

**空间压缩:**

```c++
int knapsack(vector<int>& weights, vector<int>& values, int N, int W) {
    vector<int> dp(W+1, 0);
    for (int i = 1; i <= N; ++i) {
        int w = weights[i-1], v = values[i-1];
        for (j = W; j >= w; ++j) {
            dp[j] = max(dp[j], dp[j-w] + v);
        }
    }
    return dp[W];
}
```

#### 完全背包问题

<u>完全背包问题</u>中，一个物品可以拿多次。对于拿多个物品的情况，我们只需考虑 `dp[2][3]` 即可，即 `dp[2][5] = max(dp[1][5], dp[2][3] + 3)`。这样，我们
就得到了完全背包问题的状态转移方程：`dp[i][j] = max(dp[i-1][j], dp[i][j-w] + v)`，其与 `0-1背包问题`的差别仅仅是把状态转移方程中的第二个 `i-1` 变成了 `i`。

```c++
int knapsack(vector<int>& weights, vector<int>& values, int N, int W) {
    vector<vector<int>> dp(N+1, vector<int>(W+1, 0));
    for (int i = 1; i <= N; ++i) {
        int w = weights[i-1], v = values[i-1];
        for (int j = 1; j <= W; ++j) {
            if (j >= w) {
                dp[i][j] = max(dp[i-1][j], dp[i][j-w] + v);
            } else {
                dp[i][j] = dp[i-1][j];
            }
        }
    }
    return dp[N][W];
}
```

**空间压缩:**

```c++
int knapsack(vector<int>& weights, vector<int>& values, int N, int W) {
    vector<int> dp(W+1, 0);
    for (int i = 1; i <= N; ++i) {
        int w = weights[i-1], v = values[i-1];
        for (int j = w; j <= W; ++j) {
            dp[j] = max(dp[j], dp[j-w] + v);
        }
    }
    return dp[W];
}
```

ref:</br>
`dp:` https://juejin.cn/post/6844903993429196813</br>
`knapsack problem:` https://blog.csdn.net/qq_38410730/article/details/81667885</br>
`完全背包问题:` https://www.cnblogs.com/darkerg/p/15464987.html</br>


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/knapsack/  

