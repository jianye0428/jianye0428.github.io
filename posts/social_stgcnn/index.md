# Social_STGCNN 论文解读



`paper link:` https://arxiv.org/abs/2002.11927?from=leiphonecolumn_paperreview0323

## 网络结构

特点: Social STGCNN不同于其他方法只是聚合各种学习的行人状态，而是对行人交互做图建模。其中提出一种kernel function把行人社交交互嵌入一个adjacency matrix。

> 代码显示，图建模一般在数据前处理完成。

### Model Description

两部分：时空图卷积神经网络ST-GCNN、时间外推器TXP-CNN。

ST-GCNN对行人轨迹的图表示进行时空卷积操作以提取特征。这些特征是观察到的行人轨迹历史的紧凑表示。
TXP-CNN将这些特征作为输入，并预测所有行人作为一个整体的未来轨迹。我们使用时间外推器的名字是因为TXP-CNN期望通过卷积运算外推未来的轨迹。

![ModelDescription](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-08_Social_STGCNN/ModelDescription.png)

给定T帧，构造表示 $G=(V,A)$ 的时空图. 然后，$G$ 通过时空图卷积神经网络(ST-GCNNs)转发，创建一个时空嵌入。 之后，TXP-CNNs 预测了未来的轨迹。 $P$ 是行人位置的维数，$N$ 是行人的数目，$T$ 是时间步长, $\hat{P}$是来自ST-GCNN的嵌入的维数.


(1) <font color=red>Graph Representation of Pedestrian Trajectories</font>

我们首先构造一组空间图 $G_t$，表示每个时间步长 $t$ 在场景中行人的相对位置，$G_t = (V_t, E_t)$ 。 $V_t$是图 $G_t$ 的顶点集，观察到的位置 $(x^i_t，y^i_t)$ 是顶点 $v^i_t$ 的属性; $E_t$ 是边集，如果顶点 $v^i_t$ 和顶点 $v^j_t$ 相连 $e^{ij}_t = 1$ ，否则 $=0$。

为了建模两个节点之间相互影响的强度，我们附加了一个值$a^{ij}_t$, 它是由每个$ e^{ij}_t$ 的某种核函数计算得到。$a^{ij}_t$ 被组织为带权邻接矩阵$A_t$。

**$a^{ij}_{sim,t}$是要在邻接矩阵$A_t$中使用的内核函数。** 定义为:

$$\begin{equation}
a^{ij}_{sim,t}=
\left
\{
\begin{aligned}
1/||v^i_t - v^j_t||_2 , ||v^i_t - v^j_t||_1\neq0 \\
0, Otherwise
\end{aligned}
\right.
\end{equation}$$

(2) <font color=red>Graph Convolution Neural Network</font>

对于在二维网格地图或特征地图上定义的卷积运算，定义如下:

$$z^{(l+1)} = \sigma(\sum_{h=1}^{k}\sum_{\omega=1}^{k}(p(z^{(l)},h, \omega) \cdot \boldsymbol{W}^{(l)}(h, \omega))$$

其中，$k$是内核大小，$p(.)$ 是采样函数，其聚集以$z$为中心的邻居的信息， $\sigma$ 是激活函数。${l}$表示神经网络层。

图卷积定义如下:

$$v^{i(l+1)} =\sigma (\frac{1}{\Omega}\sum_{v^{j(l)}\in B(v^{j(l)})}p(v^{i(l)}, v^{j(l)}) \cdot \boldsymbol{W}(v^{i(l)}, v^{j(l)}))$$

其中$\frac{1}{\Omega}$ 是正则化项，$B(v^i) =  \{ v^j|d(v^i,v^j)≤D \}$是顶点的邻居集，而$d(v^i,v^j)$表示连接$v^i$和$v^j$的最短距离， $\Omega$是邻居集的基数。

(3) <font color=red>Spatio-Temporal Graph Convolution Neural Network(ST-GCNNs)</font>

通过定义一个新的图G，其属性是$G_t$属性的集合，ST-GCNN将**空间图卷积**扩展到**时空图卷积**。 $G$结合了行人轨迹的时空信息。值得注意的是，$G_1，…，G_T$的拓扑结构是相同的，而当t变化时，不同的属性被分配给$v^i_t$。

因此，我们将$G$定义为$(V,E)$，其中$V=\{v_i|i\in \{ 1，…，N \}\}$ 和 $E=\{e_{ij}|i，j，\{1，…，N\}\}$。 顶点$v_i$在G中的属性是$v^i_t$的集合，$∀t∈\{0，…，T\}$。 另外， 加权邻接矩阵A对应于$G$ 是$\{ A_1，…，A_T\}$的集合。 我们将ST-GCNN产生的嵌入表示为 $\overline{V}$.

(4) <font color=red>Time-Extrapolator Convolution Neural Network (TXP-CNN)</font>

ST-GCNN的功能是从输入图中**提取时空节点嵌入**。然而，我们的目标是预测行人未来的进一步位置。
TXP-CNN直接作用于图嵌入 $\overline{V}$ 的时间维度，并将其扩展为预测的必要条件。 由于TXP-CNN依赖于特征空间的卷积运算，因此与递归单元相比，它的参数较小。需要注意的一个特性是， TXP-CNN层不是置换不变的，因为在TXP-CNN之前，图嵌入的变化会导致不同的结果。Other than this, if the order of pedestrians is permutated starting from the input to Social-STGCNN then the predictions are invariant.


### model(Social STGCNN) Implementation

1. Adjacency Matrix Normalization

$$ A_t = \Lambda_t^{-\frac{1}{2}}\hat{A}\Lambda_t^{-\frac{1}{2}}$$

where $\hat{A_t} = A_t + I$ and $\Lambda_t$ is the diagonal node degree matric of $\hat{A_t}$. We use $\hat{A}$ and $\Lambda$ to denote the stack of $\hat{A_t}$ and $\Lambda_t$ repectively.

The normalization of adjacency is essential for the graph CNN to work properly.


2. STGCNN Network Mechanism

$$f(V^{l}, A) = \sigma(\Lambda_t^{-\frac{1}{2}}\hat{A}\Lambda_t^{-\frac{1}{2}}V^{(l)}W^{(l)})$$

where, $V^{(l)}$ denotes the stack of $V^{(l)}_t$, and $W^{(l)}$ denotes the trainable parameters.

## Data Processing 数据处理以及图构建

obs_traj - <font color=red>*前8帧观察轨迹(绝对坐标)*</font>
pred_traj_gt - <font color=red>*后12帧预测轨迹(ground truth)(绝对坐标)*</font>
obs_traj_rel - *<font color=red>前8帧观察轨迹(相对坐标)*</font>
pred_traj_gt_rel - *<font color=red>后12帧预测轨迹(ground truth)(相对坐标)*</font>
non_linear_ped - *<font color=red>非线性轨迹 (剔除)*</font>
loss_mask
V_obs - *<font color=red>graph nodes*</font>
A_obs - *<font color=red>graph Adjacency Matrix*</font>
V_tr - *<font color=red>预测轨迹 graph nodes*</font>
A_tr - *<font color=red>预测轨迹 graph Adjacency Matrix*</font>


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/social_stgcnn/  

