# Social_NCE 论文解读



`paper link:` https://arxiv.org/abs/2012.11717
`论文解读参考:`
    [1] https://zhuanlan.zhihu.com/p/434650863
    [2] https://www.gushiciku.cn/pl/amod

## Issue to solve and its Solution

Due to the ill-distributed training Data, it's <u><font color=red>difficult to capture the notion of the "negative" examples</font></u> like collision.

**Solution:**

Modeling the negative samples through self-supervision:
- <font color=red>a social contrastive loss</font>: regularizes the extracted motion representation by discerning the ground-truth positive events from synthetic negative ones;
- <font color=red>Construct negative samples</font> based on prior knowledge of rare but dangerous circumstances.
  > a social sampling strategy (informed): construct the positive event from the ground-truth location of the primary agent and the negative events from the regions of other neighbors. given that one location cannot be occupied by multiple agents at the same time.


## Method: <font color=red>*Contrastive Learning + Social NCE*</font>

### Contrastive Representation Learning

- Functionality:

  - `Representation Learning:` to learn a parametric function that maps the raw data into a feature space to extract abstract and useful information for downstream tasks.

  - `NCE(Noise Contrastive Estimation):` to train encoder

   $$\mathcal{L_{NCE}} = -\log \frac{\exp(sim(q,k^+)/\tau)}{\sum_{n=0}^N  \exp(sim(q,k_n)/ \tau)}$$

    where the encoded query $q$ is brought close to one positive key $k_0 = k^+$ and pushed apart from $N$ negative keys $\{ k_1, k_2, ... , k_N\}$, $\tau$ is a temperature hyperparameter, and $sim(u,v) = u^{\mathsf{T}}v/(||u||||v||)$ is the cosine similarity between two feature vectors.

### Social NCE

**Social NCE Description:**

智能体 $i$ 在时刻 $t$ 上的位置记为 $s^i_t=(x^i_t,y^i_t)$ 。那么 $M$ 个智能体的联合状态记为 $s_t = \{ s_t^1, ..., s^M_t\}$ 。给定一个历史观测序列 $\{s_1, s_2, ..., s_t\}$ ，任务是预测所有智能体未来直至 $T$ 时刻的轨迹 $\{s_{t+1}, ..., s_T\}$，许多最近的预测模型被设计为编码器 - 解码器神经网络，其中运动编码器 $f(\cdot)$ 首先提取与 $i$ 相关的紧密表示 $h_t^i$ ，然后解码器 $g(\cdot)$ 随后推测出其未来的轨迹 $\hat{s}^i_{t+1,T}$ :

$$h^i_t = f(s_{1:t}, i),  $$
$$\hat{s}^i_{t+1:T} = g(h^i_t)$$

为了多智能体之间的社交互动，$f(\cdot)$通常包含两个子模块：一个序列建模模块 $f_S(\cdot)$ 用于编码每个单独的序列，以及一个交互模块 $f_I(\cdot)$ 用于在多智能体之间共享信息：

$$z^i_t = f_S(h^i_{t-1}, s^i_t),$$
$$h^i_t = f_I(z_t, i)$$

其中， $z^i_t$ 是给定智能体 $i$ 在时间 $t$ 观察其自身状态的潜在表示， $z_t = \{z^1_t,...,z^M_t\}$ 。很多方法已经探索了各种架构，并验证了其准确性。尽管如此，它们的鲁棒性仍然是一个悬而未决的问题。 最近的几项工作表明，现有模型预测的轨迹通常会输出社会不可接受的解决方案（例如，碰撞），表明缺乏关于社会准则的常识。

![SocialNCE](images/social_nce.png#pic_center)

- `query`: embedding of history observations $q = \psi(h^i_t)$, where $\psi(\cdot)$ is an MLP projection head;
- `key`: embedding of a future event $k = \phi(s^i_{s+\delta t}, \delta t)$, where $\phi(\cdot)$ is an event encoder modeled by an MLP, $s_{t+\delta t}^i$ is a sampled spatial location and $\delta_t > 0$ is the sampling horizon.

    > tuning $\delta_t \in \Lambda$, e.g. $\Lambda = \{1,...,4\}$, then future events in the next few step can be taken in account simultaneously. Nevertheless, when $\delta_t$ is a fixed value, then $\phi(\cdot)$ can be simplified as a location encoder, i.e., $\phi(s^i_{t+\delta t})$.

给定一个场景，包括感兴趣的主智体（蓝色）和附近多个相邻智体（灰色），Social-NCE 损失鼓励在嵌入空间中提取的运动表示，接近未来的正样本事件，并远离可能导致碰撞或不适的合成负样本事件. Social NCE的损失函数如下:

$$\mathcal{L_{SocialNCE}} = -\log\frac{\exp(\psi(h^i_t)\cdot\phi(s^{i,+}_{t+\delta t}, \delta t)/\tau)}{\sum_{\delta t\in\Lambda}\sum_{n=0}^{N}\exp(\psi(h^i_t)\cdot\phi(s^{i,n}_{t+\delta t}, \delta t)/\tau))}$$


最终的训练损失函数为Social-NCE和传统任务损失项之和，即轨迹预测的mean squared error (MSE) 或者negative log-likelihood (NLL)：

$$\mathcal{L}(f,g,\psi, \phi) = \mathcal{L}_{task}(f,g) + \lambda \mathcal{L}_{SocialNCE}(f, \psi, \phi)$$

其中，$\lambda$ 为超参数，控制SocialNCE损失函数的重要程度。

### sampling strategy in multi-agent context 采样策略


![sampling strategy](https://github.com/jianye0428/hello-hugo/raw/master/img/posts/tech/2022-06-12_Social_NCE/sampling_strategy.png#pic_center)

在其他智能体附近寻求更多信息的负样本:

$$s^{i,n-}_{t+\delta t} = s^{j}_{t+\delta t} + \bigtriangleup{s_p} + \epsilon$$

其中， $j\in\{1,2,...,M\} \backslash i$ 是其他agent的index, $\bigtriangleup{s_p}$ 是适合社交距离的局部位移。

对于positive sample, 对该agent周围直接采样获得:

$$s^{i,n-}_{t+\delta t} = s^{i}_{t+\delta t} +  \epsilon$$


---

> 作者: <no value>  
> URL: https://lruihao.cn/posts/social_nce/  

