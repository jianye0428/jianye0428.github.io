# VectorNet 论文解读



`ref link`:
[1] https://blog.csdn.net/qq_41897558/article/details/120087113
[2] https://zhuanlan.zhihu.com/p/355131328
`ref code`:
[1]https://github.com/xk-huang/yet-another-vectornet
[2]https://github.com/DQSSSSS/VectorNet
## Novel Highlights

 (1) 使用矢量化的高精地图以及障碍物的历史轨迹，从而避免有损渲染以及ConvNet编码(计算开销比较大)。

 (2) 设计子图网络以及全局图网络，建模低阶以及高阶交互

 (3) auxiliary task 提高网络性能

![VectorNet Overview](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-20_VectorNet/Overview_VectorNet.png#pic_center)


## VecotorNet 网络介绍

### 轨迹和地图的向量表示 Representing trajectories and HD maps

lane可以表示为splines，人行道可以表示为一个很多个点组成的polygon，stop sign标记可以表示为单一个点。 对于agent来说，他们的轨迹也是一种splines。 这些元素都可以向量表示。

- 对于地图的特征：选择一个start point和朝向，等间距均匀采样关键点，并于相邻的关键点相连为向量
- 对于agent轨迹，按照0.1s sample关键点，并将它们连接成向量。

通过向量化的过程，可以得到折线polylines，这个polylines和轨迹、地图标注之间是一一对应的。如果给定的时空间隔足够小，得到的这些折线就与原始地图和轨迹十分接近。

我们将属于折线 $P_j$​ 的每一个向量$v_i$看出图中的一个节点，节点特征如下:

$$v_i = [d_i^s, d_i^e, a_i, j]$$

- 其中前两个vector分别是vector的start point和end point的坐标，可以是(x,y)或者(x,y,z)三维的形式
- 第三个向量则是attribute属性的特征，比如object的类型，轨迹的时间戳，道路的特征，道路限速等
- 最后一个是障碍物id，表示 $v_i$ ​属于 $P_j$

### Polyline 子图构建
对于一个Polyline P, 它的节点有 $\{v_1,v_2,...,v_p\}$， 可以定义一个子图网络：

$$v_i^{l+1} = \varphi_{rel}(g_{enc}(v_i^{(l)}), \varphi({g_{enc}(v_j^{(l)})}))$$

- $v_i^{(l)}$​ 代表第i个节点第L层的节点特征。

- $g_{enc}(\cdot)$代表节点的变换，实践中采用MLP来实现。

- $\varphi_{agg}(\cdot)$代表特征聚合，用来从相邻的节点来获取信息，实践中采用的是max_pooling。

- $\varphi_{rel}(\cdot)$代表vi和周围节点的关系，实践中采用的是concate的操作。

![Subgraph](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-20_VectorNet/Subgraph.png#pic_center)

最后经过多层的堆叠，来获取整个Polyline级别的特征：

$$P = \varphi_{agg}(v_i^{L_p})$$

这里， $φ_{agg}(⋅)$也是max pooling操作.

### 全局图的高阶交互 Global graph for high-order interactions

经过上面的子图进行低阶模型建模后，现在有了polyline级别节点的特征$\{p_1,p_2,...,p_P\}$.

为了建立高阶的交互，需要建立一个global的交互图，详见论文图2的第3个子图。

$$P_i^{l+1} = GNN(p^l_i, A)$$

- $p_i^l$​代表polyline节点的集合

- A代表邻接矩阵，实践中采用全链接

- $GNN(⋅)$代表一层的GNN网络，实践中采用的是self attention layer：
  $$GNN(P) = softmax(P_Q P_K^T)P_V$$

    其中，P是node的feature matrix， $P_Q$,$P_k$,$P_v$ ​则是它的线性投影。

经过了全局的网络之后，就生成了节点的特征$P^{L_t}_i$，其中Lt是全局GNN网络的层数。然后将$P^{(L_t)}_i$放入decoder进行轨迹的生成:

$$v_i^{future} = \varphi_{traj}(P_i^{L_t})$$

论文中，decoder $φ_{traj}(⋅)$ 使用的是MLP，当然也可以用MultiPath中anchor-based的方法或者variational RNNs 来进行多模态轨迹预测。

### 辅助任务训练 auxiliary graph completion task

为了让全局交互图能更好地捕捉不同轨迹和地图元素之间的交互信息，论文还提出了一个辅助的任务：在训练过程中，随机mask掉一些节点的特征，然后尝试去还原被掩盖的节点特征:

$$\hat{P}_i = \varphi_{node}(P_i^{L_t})$$

这里节点的decoder $φ_{node}(⋅)$ 也是一个MLP，只在训练的时候使用,在inference过程中不使用。

### 损失函数 Loss Function

多任务训练目标， multi-task training task:

$$\mathcal{L} = \mathcal{L_{traj}} + \alpha \mathcal{L_{node}}$$


- $L_{traj}​$: negative Gaussian log-likelihood loss

- $L_{node}$​: 是预测的节点和被掩盖节点的huber损失函数

其中，
negative Gaussian Log Likelihood 损失函数为:

$$L(x, y) = -\log P(y) = - \log P(y|\mu(x), \sum(x))$$

where,

$$p(y) = p(y∣μ,Σ)=1(2π)n/2∣Σ∣1/2exp−12(y−μ)⊤Σ−1(y−μ)$$

Huber 损失函数为:

$$ L(Y|f(x))= \begin{cases} \frac{1}{2} (Y-f(x))^2, & |Y-f(x)|<= \delta \\\\ \delta |Y-f(x)| - \frac{1}{2}\delta^2, & |Y-f(x)| > \delta \end{cases} $$

## 整理

**VectorNet数据处理部分:**

- 对actor的处理:

  - 输入: 取轨迹点，每两个轨迹点构建vector, 形式为(x1, x2, y1, y2), 其他特征(object type, timestamp, track_id)

- 对lane node的处理:

   - 输入: 针对lane segment 的点，求polyline，原则上求lane segment的左右边界的点的向量(x_start, x_end, y_start, y_end, turn_direction, traffic_control, is_intersection, lane_id)

**网络部分:**

- 构建subgraphnet: 针对每一个polyline，通过mlp和maxpool构构建subgraphnet

- 构建globalgraphnet: 以每个polyline作为graph node，构建全局图网络，采用全链接，通过自注意力机制$GNN(P) = softmax(P_Q, P_K)^T(P_V)$

**轨迹生成:**

将全局网络的节点特征，通过mlp进行轨迹生成。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/vectornet/  

