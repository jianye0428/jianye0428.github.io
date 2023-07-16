# LaneGCN 论文解读



`paper link:` https://arxiv.org/abs/2007.13732
`PPT:` https://www.cs.toronto.edu/~byang/slides/LaneGCN.pdf

## Architechture

**<font color=red>Lane Graph + Actor Map:</font>**

- construct lane graph from vectorized map data to preserve the map structure and can avoid information loss 构建矢量化地图信息，避免地图信息丢失

- LaneGCN:

  - extends **graph convolutions with multiple adjacency matrices** and along-lane dilation

    - to capture complex topology and long range dependencies of the lane graph.

  - exploit a **fusion network** consisting of four types of interactions: `actor-to-lane`, `lane-to-actor`, `actor-to-actor`, `lane-to-lane`.

    - present both actors and lanes as nodes in the graph and use a 1D CNN and LaneGCN to extract the features for the actor and lane nodes respectively, and then exploit spatial attention and another LaneGCN to model four types of interactions.



![NN Architecture](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-13_LaneGCN/NN_Architecture.png#pic_center)


**<font color=red>Difference between VectorNet and LaneGCN:</font>**

- <u>VecotrNet</u> uses vanilla graph networks with undirected full connections; <u>LaneGCN</u> uses connected lane graph folllowing the map topology and propose task specific multi-type and dilated graph operators.
- VectorNet uses polyline-level nodes for interactions; LaneGCN uses polyline segments as map nodes to capture higher resolution.

## Lane Graph Representations for Motion Forecasting

![Model_Layout](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-13_LaneGCN/Model_Layout.png#pic_center)

### <font color=red>ActorNet</font>: Extracting Traffic Participant Representations

Each Trajctory is represented as a sequence of displacement $\{ \bigtriangleup{p_{-(T-1)},...,\bigtriangleup{p_{-1}}, \bigtriangleup{p_0}}\}$, where $\bigtriangleup{p_t}$ is the 2D displacement from time step $t-1$ to t, and T is the trajectory size.

For trajectories with sizes smaller than $T$ , we pad them with zeros. We add a binary $1 × T$ mask to indicate if the element at each step is padded or not and concatenate it with the trajectory tensor, resulting in an input tensor of size $3 × T$.

1D CNN is used to process the trajectory input for its effectiveness in extracting multi-scale features
and efficiency in parallel computing. The output of ActorNet is a temporal feature map, whose element at $t = 0$ is used as the actor feature. The network has 3 groups/scales of 1D convolutions.

Each group consists of 2 residual blocks, with the stride of the first block as 2. We then use a Feature Pyramid Network (FPN) to fuse the
multi-scale features, and apply another residual block to obtain the output tensor. For all layers, the convolution kernel size is 3 and the number of output channels is 128. Layer normalization and the Rectified Linear Unit (ReLU) are used after each convolution.

![ActorNet](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-13_LaneGCN/ActorNet.png#pic_center)


### <font color=red>MapNet</font>: Extracting Structured Map Representation

General Architecture:

  - part 1: building a lane graph from vectorized map data;
  - part 2: applying our novel LaneGCN to the lane graph to output the map features.



**Map Data:**

In this paper, we adopt a simple form of vectorized map data as our representation of HD maps. Specifically, the map data is represented as a set of lanes and their connectivity. Each lane contains a centerline, i.e., a sequence of 2D BEV points, which are arranged following the lane direction (see Fig. 3, top). For any two lanes which are directly reachable, 4 types of connections are given: `predecessor`, `successor`, `left neighbour` and `right neighbour`.

**Lane Graph Construction:**

first define a lane node as the straight line segment formed by any two consecutive points (grey circles in Fig. 3) of the centerline. The location of a lane node is the averaged coordinates of its two end points. Following the connections between lane centerlines, we also derive 4 connectivity types for the lane nodes, i.e., `predecessor`, `successor`, `left neighbour` and `right neighbour`.

We denote the lane nodes with $V ∈ \mathbb R^{N ×2}$ , where $N$ is the number of lane nodes and the $i$-th row of $V$ is the BEV coordinates of the $i$-th node. We represent the connectivity with 4 adjacency matrices ${\lbrace A_i \rbrace}_{i \in {pre,suc,left,right}}$ , with $A_i \in \mathbb R^{N ×N}$.

We denote $A_{i,jk}$, as the element in the $j$-th row and $k$-th column of $A_i$. Then $A_{i,jk} = 1$ if node $k$ is an $i$-type neighbor of node $j$.

**LaneConv Operator:**

<font color=green>*Node Feature:*</font>
Each lane node corresponds to a straight line segment of a centerline. To encode all the lane node information, we need to take into account both the shape (size and orientation) and the location (the coordinates of the center) of the corresponding line segment. We parameterize the node feature as follows,

$$x_i = MLP_{shape} (v_{i}^{end} - v_{i}^{start}) + MLP_{loc}(v_i) $$

where $MLP$ indicates a multi-layer perceptron and the two subscripts refer to shape and location, respectively. $v_i$ is the location of the i-th lane node, i.e., the center between two end points, $v_i^{start}$ and $v_i^{end}$ are the BEV coordinates of the node $i’s$ starting and ending points, and $x_i$ is the $i$-th row of the node feature matrix $X$, denoting the input feature of the $i$-th lane node.

<font color=green>*LaneConv:* </font>
To aggregate the topology information of the lane graph at a larger scale, we design the following LaneConv operator:

$$Y = XW_0 + \sum_{i\in\{pre, suc, left, right\}}A_iXW_i,\tag{2}$$

where $A_i$ and $W_i$ are the adjacency and the weight matrices corresponding to the $i$-th connection type respectively. Since we order the lane nodes from the start to the end of the lane, $A_{suc}$ and $A_{pre}$ are matrices obtained by shifting the identity matrix (diagnal 1) one step towards upper right (non-zero superdiagonal) and lower left (non-zero subdiagonal). $A_{suc}$ and $A_{pre}$ can propagate information from the forward and backward neighbours whereas $A_{left}$ and $A_{right}$ allow information to flow from the cross-lane neighbours. It is not hard to see that our LaneConv builds on top of the general graph convolution and encodes more geometric (e.g., connection type/direction) information. As shown in our experiments this improves over the vanilla graph convolution.


<font color=green>*Dilated LaneConv:*</font>

Functionality: The model needs to capture the long range dependency along the lane direction for accurate prediction.

the k-dilation LaneConv operator is defined as follows:

$$Y = XW_0 + A_{pre}^k XW_{pre,k} + A_{suc}^k X W_{suc,k} \tag{3}$$

where $A_{pre}^k$ is the $k$-th matrix power of $A_{pre}$. This allows us to directly propagate information along the lane for $k$ steps, with $k$ a hyperparameter. Since $A_{pre}^k$ is highly sparse, one can efficiently compute it using sparse matrix multiplication. Note that the dilated LaneConv is only used for predecessor and successor, as the long range dependency is mostly along the lane direction.


<font color=green>*LaneGCN:*</font>

With Eq.(2) and Eq.(3), we get a multi-scale LaneConv operator with C dilation size as follows:

$$Y = XW_0 + \sum_{i\in \lbrace left, right \rbrace} A_i X W_i + \sum_{c=1}^C (A_{pre}^{k_c}XW_{pre, k_c} + A_{suc}^{k_c}XW_{suc, k_c})， \tag{4}$$

where $k_c$ is the $c$-th dilation size. We denote $LaneConv(k_1 , · · · , k_C)$ this multi-scale layer.

![LaneGCN Architecture](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-13_LaneGCN/LaneGCN_Architecture.png)


### <font color=red>Fusion Net</font>

Four types fusion modules:

 - A2L: introduces real-time traffic information to lane nodes, such as blockage or usage of the lanes.
 - L2L: updates lane node features by propagating the traffic information over the lane graph. -> LaneGCN
 - L2A: fuses updated map features with real-time traffic information back to the actors.
 - A2A: handles the interactions between actors and produces the output actor features, which are then used by the prediction header for motion forecasting.

We implement L2L using another LaneGCN, which has the same architecture as the one used in our MapNet (see Section 3.2). In the following we describe the other three modules in detail. We exploit a spatial attention layer for A2L, L2A and A2A. The attention layer applies to each of the three modules in the same way. Taking A2L as an example, given an actor node i, we aggregate the features from its context lane nodes j as follows:

$$y_i = x_i W_0 + \sum_j \phi (concat(x_i, \Delta_{i,j}, x_j)W_1)W_2, \tag{5}$$

with $x_i$ the feature of the $i$-th node, $W$ a weight matrix, $\phi$ the compositon of layer notmalization and RelU, and $\Delta_{ij} = MLP(v_j - v_i)$, where $v$ denotes the node location.


### <font color=red>Prediction Header</font>

Take after-fusion actor features as input, a multi-modal prediction header outputs the final motion forecasting. For each actor, it predicts $K$ possible future trajectories and their confidence scores.

The header has two branches, a regression branch to predict
the trajectory of each mode and a classification branch to predict the confidence score of each mode.

For the m-th actor, we apply a residual block and a linear layer in the
regression branch to regress the K sequences of BEV coordinates:

$$O_{m,reg} = \lbrace (p_{m,1}^k, p_{m,2}^k, ..., p_{m,T}^k) \rbrace _{k\in[0,K-1]}$$

where $p_{m,i}^k$ is the predicted $m$-th actor's BEV coordinates of the $k$-th mode at the $i$-th time step. For the classification branch, we apply an MLP to $p^k_{m,T} − p_{m,0}$ to get $K$ distance embeddings. We then concatenate each distance embedding with the actor feature, apply a residual block and a linear layer to output $K$ confidence scores, $O_{m,cls} = (c_{m,0}, c_{m,1}, ..., c_{m,K−1})$.

### <font color=red>Learning</font>

use the sum of classification and regreesion losses to train the model:

$$ L = L_{cls} + \alpha L_{reg},$$

where $\alpha = 1.0$.

For classification, we use the max-margin loss:

$$L_{cls} = \frac{1}{M(K-1)}\sum_{m=1}^M \sum_{k \neq \hat{k}} \max(0, c_{m,k} + \epsilon - c_{m, \hat{k}}) \tag{6}$$

where $\epsilon$ is the margin and $M$ is the total number of actors. For regression, we apply the smooth $l1$ loss on all predicted time steps:

$$L_{reg} = \frac{1}{MT} \sum_{m=1}^M \sum_{t=1}^T reg(p_{m,y}^{\hat{k}} - p_{m,t}^*) \tag{7}$$

where $p_t^*$ is the ground truth BEV coordinates at time step $t$, $reg(x) = \sum\limits_i d(x_i)$, $x_i$ is the $i$-th element of $x$, and $d(x_i)$ is the smooth $\ell1$ loss defined as:

$$d(x_i) = \begin{cases}
   0.5x_i^2 &\text{if} ||x|| < 1, \\
   ||x_i|| - 0.5 & \text{otherwise,}
\end{cases} \tag{8}$$

where $||x_i||$ denotes the $\ell1$ norm of $x_i$.


### <font color=red> Neural Network Layout</font>

![LaneGCN Architecture](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-13_LaneGCN/NN_Layout.png)


### <font color=red>Data Process And Network Construction</font>

> 以官方的2645.csv数据集为例子

**agent node:**

- `data['city']:`城市名称
- `data['trajs'] = [agt_traj] + ctx_trajs:`轨迹点，(agent + context vehicles)
- `data['steps'] = [agt_step] + ctx_steps:`在原始数据中的位置
- `data['feats'] = feats:` (13 X 20 X 3) 前20预测轨迹 + 一维是否存在点
- `data['ctrs'] = ctrs:` (13 X 2) 中心点
- `data['orig'] = orig:` AGENT 当前点坐标
- `data['theta'] = theta:` AGENT 偏转角
- `data['rot'] = rot:` (2 X 2) 旋转矩阵
- `data['gt_preds'] = gt_preds:`(13 X 30 X 2) 后30帧真实轨迹
- `data['has_preds'] = has_preds:` (13 X 30) 标识后30帧轨迹是否存在


**lane node:**

- `graph['ctrs'] = np.concatenate(ctrs, 0):` lane node的中心点坐标
- `graph['num_nodes'] = num_nodes:` lane node的数量
- `graph['feats'] = np.concatenate(feats, 0):` lane node 方向向量
- `graph['turn'] = np.concatenate(turn, 0):` lane node 转向标识
- `graph['control'] = np.concatenate(control, 0):` lane node 的 has_traffic_control 标识
- `graph['intersect'] = np.concatenate(intersect, 0):` lane node 的 is_intersection 标识
- `graph['pre'] = [pre]:` pre['u'] 和 pre['v'], v 是 u 的pre， 这里表述的是lane node之间的关系
- `graph['suc'] = [suc]:` suc['u'] 和 suc['v'], v 是 u 的suc， 这里表述的是lane node之间的关系
- `graph['lane_idcs'] = lane_idcs:` lane node index
  - ```python
    0 0 0 ... 0
    1 1 1 ... 1
        ...
    83 83 83 ... 83
    ```
- `graph['pre_pairs'] = pre_pairs:` pair 表述的是lane之间的关系
- `graph['suc_pairs'] = suc_pairs:` pair 表述的是lane之间的关系
- `graph['left_pairs'] = left_pairs:` pair 表述的是lane之间的关系
- `graph['right_pairs'] = right_pairs:` pair 表述的是lane之间的关系
  - 对于`pre['u']`和`pre['v']`, v 是 u 的 pre
  - 对于`suc['u']`和`suc['v']`, v 是 u 的 suc
  - 对于`left['u']`和`left['v']`, v 是 u 的 left
  - 对于`right['u']`和`right['v']`, v 是 u 的 right

**Net结构**

 - **ActorNet**
  `input:` M x 3 x 20
  `output:` M x 128 x 20

  解释:


 - **MapNet**: 把 v 按照 u 加到center上
  `input:` N x 4
  `output:` N x 128

  - **A2M**
  `input:` N x 128
  `output:` N x 128

  - **M2M**
  `input:` N x 128
  `output:` N x 128

  - **M2A**
  `input:` N x 128
  `output:` M x 128

  - **A2A**
  `input:` N x 128
  `output:` N x 128

  - **Prediction Header:**
  `input` M x 128
     - MLP Regression
     - MLP Classification

ref link: https://zhuanlan.zhihu.com/p/447129428

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/lanegcn/  

