# DQN


`[DQN]paper link:` https://arxiv.org/pdf/1312.5602v1.pdf

## DQN: Playing Atari with Deep Reinforcement Learning
### General Architecture

Here is Network listed:
- play Atari games using RL and perform better than human
- CNN + Q Learning: CNN for frame-skiped images features extraction; and Q Learning for policy generation

| Network | Channel | Kernel Size | Stride | Activation | Output Size|
|  :----: | :----:  |    :----:   |    :----:  |:----:|:----:|
|Input |NA    |NA    |NA    |NA    |$84\times84\times4$|
|First Conv|16|8x8|4|Relu|$20 \times 20 \times 6$|
|Second Conv|32|4x4|2|Relu|$9 \times 9 \times 32$|
|Hidden|NA|NA|NA|Relu|256|
|Output|NA|NA|NA|None|4 to 18|

> **在当时，普遍的做法是为每一个action学习一个函数，而不是一个网络结构直接输出所有q的value.**

### Key 1: Input Info Process

  > 图像处理部分
  - Grayscale, Downsampling and Cropping
    - RGB channels to gray scale channel (将RGB取均值为灰度图):
      216 x 163 x 3 =>(grayscale) 216 x 163 x 1 =>(downsampling) 110 x 84 x 1 =>(cropping) 84 x 84 x 1

  > 游戏部分
  - **Key Frame and Action Repeat**
    - select skipped frames (每个4帧选取关键帧)，假设智能体看不见中间过程; 而且agent在每k帧选择一个action，可以加速训练
    - **作用**:
      + 加速游戏进行: 计算Q-Value是最耗时的步骤;
      + 减少噪声: 过分紧密的frame重复信息过多，之前的action容易被否决;
      + 缩短reward signal到具体aciton之间的时间间隔。
  - **History as Input**
    - continuous history key frames as input (连续四个关键帧作为输入)
    - **作用**:
      + 可以帮助智能体获得更多有效信息进行训练
  - **Reward Clipping**
    - 将多有的reward简化为+1, -1和0
    - **缺点**: 有可能对训练效果有影响
    - **作用**: 损失了部分信息，但是可以保证不同游戏的reward scale相同，可以用相同的参数进行训练(因为在论文中，作者在多个游戏上对DQN进行了验证)。

### Key 2: Replay Buffer

  - **原理**:
    1. DQN中对神经网络的训练本质依然是SGD，SGD要求多次利用样本，并且样本独立，但相邻的transition都是高度相关的，所以要记住过去的transition一起抽样;
    2. Replay Buffer通过记忆一段时间内的trainsition，可以让训练数据分布更平稳;
    3. Replay Buffer通过忘记很久之前的trainsition，可以保证记住的分布大致模拟当前policy的分布，从而进行policy update;
    4. 可以多次重复采样，提升data efficiency.

  - Replay Buffer生效的一个**重要条件**: 存储transition数量合适
    + **太多**: 可能使reward signal太过稀疏，影响训练
    + **太少**: 可能会导致训练数据的分布迅速变化

### Key 3: Semi-Gradient Method

  在Eauation3中，

  $$y_i = r + \gamma \max_{a'}Q(s', a'; \theta_{t-1})$$

  不和之后的Q函数共享参数;

  但是在实际的训练过程中，采用
  $$ y_i = r + \gamma \max_{a'}Q(s', a'; \theta_{t})$$

  和之后的Q函数共享参数，但是实际上不参与导数计算，这种方法称为**Semi-Gradient Method**。

  - 作用: 使训练更新更稳定。


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/dqn/  

