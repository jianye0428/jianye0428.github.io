# 强化学习笔记 [13] | 策略梯度(Policy Gradient)


# 0. 引言

在前面讲到的DQN系列强化学习算法中，我们主要对价值函数进行了近似表示，基于价值来学习。这种Value Based强化学习方法在很多领域都得到比较好的应用，但是Value Based强化学习方法也有很多局限性，因此在另一些场景下我们需要其他的方法，比如本篇讨论的策略梯度(Policy Gradient)，它是Policy Based强化学习方法，基于策略来学习。

本文参考了Sutton的强化学习书第13章和策略梯度的[论文](https://homes.cs.washington.edu/~todorov/courses/amath579/reading/PolicyGradient.pdf)。

# 1. Value Based强化学习方法的不足

DQN系列强化学习算法主要的 **<font color=red>问题</font>** 主要有三点。

  - 第一点是对连续动作的处理能力不足。DQN之类的方法一般都是只处理离散动作，无法处理连续动作。虽然有NAF DQN之类的变通方法，但是并不优雅。比如我们之前提到的经典的冰球世界(PuckWorld) 强化学习问题，具体的动态demo见[这里](https://cs.stanford.edu/people/karpathy/reinforcejs/puckworld.html)。环境由一个正方形区域构成代表着冰球场地，场地内大的圆代表着运动员个体，小圆代表着目标冰球。在这个正方形环境中，小圆会每隔一定的时间随机改变在场地的位置，而代表个体的大圆的任务就是尽可能快的接近冰球目标。大圆可以操作的行为是在水平和竖直共四个方向上施加一个时间乘时长的力，借此来改变大圆的速度。假如此时这个力的大小和方向是可以灵活选择的，那么使用普通的DQN之类的算法就不好做了。因为此时策略是一个有具体值有方向的力，我们可以把这个力在水平和垂直方向分解。那么这个力就是两个连续的向量组成，这个策略使用离散的方式是不好表达的，但是用Policy Based强化学习方法却很容易建模。

  - 第二点是对受限状态下的问题处理能力不足。在使用特征来描述状态空间中的某一个状态时，有可能因为个体观测的限制或者建模的局限，导致真实环境下本来不同的两个状态却再我们建模后拥有相同的特征描述，进而很有可能导致我们的value Based方法无法得到最优解。此时使用Policy Based强化学习方法也很有效。

  - 第三点是无法解决随机策略问题。Value Based强化学习方法对应的最优策略通常是确定性策略，因为其是从众多行为价值中选择一个最大价值的行为，而有些问题的最优策略却是随机策略，这种情况下同样是无法通过基于价值的学习来求解的。这时也可以考虑使用Policy Based强化学习方法。

由于上面这些原因，Value Based强化学习方法不能通吃所有的场景，我们需要新的解决上述类别问题的方法，比如Policy Based强化学习方法。

# 2. Policy Based强化学习方法引入

回想我们在Value Based强化学习方法里，我们对价值函数进行了近似表示，引入了一个动作价值函数 $\hat{q}$，这个函数由参数 $w$ 描述，并接受状态 $s$ 与动作 $a$ 作为输入，计算后得到近似的动作价值，即：

$$\hat{q}\left(s,a,w\right)\approx q\_\pi(s,a)$$

在Policy Based强化学习方法下，我们采样类似的思路，只不过这时我们对策略进行近似表示。此时策略 $π$可以被被描述为一个包含参数 $θ$ 的函数,即：

$$\pi\_\theta(s,a)=P(a|s,\theta)\approx\pi(a|s)$$

将策略表示成一个连续的函数后，我们就可以用连续函数的优化方法来寻找最优的策略了。而最常用的方法就是梯度上升法了，那么这个梯度对应的优化目标如何定义呢？

# 3. 策略梯度的优化目标

我们要用梯度上升来寻找最优的梯度，首先就要找到一个可以优化的函数目标。

最简单的优化目标就是初始状态收获的期望，即优化目标为：

$$J\_1(\theta)=V\_{\pi\_\theta}(s\_1)=\mathbb{E}\_{\pi\_\theta}(G\_1)$$

但是有的问题是没有明确的初始状态的，那么我们的优化目标可以定义平均价值，即：
$$J\_{avV}(\theta)=\sum\_sd\_{\pi\_\theta}(s)V\_{\pi\_\theta}(s)$$

其中，$d\_πθ(s)$ 是基于策略 $π\_θ$生成的马尔科夫链关于状态的静态分布。

或者定义为每一时间步的平均奖励，即：

$$J\_{avR}(\theta)==\sum\_sd\_{\pi\_\theta}(s)\sum\_a\pi\_\theta(s,a)R\_s^a$$

无论我们是采用 $J\_1$, $J\_{av}V$, 还是 $J\_{av}R$ 来表示优化目标，最终对 $θ$求导的梯度都可以表示为：

$$\nabla\_\theta J(\theta)=\mathbb{E}\_{\pi\_\theta}[\nabla\_\theta log\pi\_\theta(s,a)Q\_\pi(s,a)]$$

具体的证明过程这里就不再列了，如果大家感兴趣，可以去看策略梯度的[论文](https://homes.cs.washington.edu/~todorov/courses/amath579/reading/PolicyGradient.pdf)的附录1，里面有详细的证明。

当然我们还可以采用很多其他可能的优化目标来做梯度上升，此时我们的梯度式子里面的 $\nabla\_\theta log\pi\_\theta(s,a)$ 部分并不改变，变化的只是后面的 $Q\_\pi(s,a)$ 部分。对于 $\nabla\_\theta log\pi\_\theta(s,a)$,我们一般称为**分值函数**(score function)。

现在梯度的式子已经有了，后面剩下的就是策略函数 $\pi\_\theta(s,a)$的设计了。

# 4. 策略函数的设计

现在我们回头看一下策略函数 $\pi\_\theta(s,a)$ 的设计，在前面它一直是一个数学符号。

最常用的策略函数就是softmax策略函数了，它主要应用于离散空间中，softmax策略使用描述状态和行为的特征 $ϕ(s,a)$ 与参数 $θ$的线性组合来权衡一个行为发生的几率,即:

$$\pi\_\theta(s,a)=\frac{e^{\phi(s,a)^T\theta}}{\sum\_be^{\phi(s,b)^T\theta}}$$

则通过求导很容易求出对应的分值函数为：

$$\nabla\_\theta log\pi\_\theta(s,a)=\phi(s,a)-\mathbb{E}\_{\pi\_\theta}[\phi(s,.)]$$

另一种高斯策略则是应用于连续行为空间的一种常用策略。该策略对应的行为从高斯分布 $\mathbb{N}(\phi(\mathrm{s})^{\mathbb{T}}\theta,\sigma^2)$中产生。高斯策略对应的分值函数求导可以得到为:

$$\nabla\_\theta log\pi\_\theta(s,a)==\frac{(a-\phi(s)^T\theta)\phi(s)}{\sigma^2}$$

有策略梯度的公式和策略函数，我们可以得到第一版的策略梯度算法了。

# 5. 蒙特卡罗策略梯度reinforce算法

这里我们讨论最简单的策略梯度算法，蒙特卡罗策略梯度reinforce算法, 使用价值函数 $v(s)$ 来近似代替策略梯度公式里面的 $Q\_π(s,a)$。算法的流程很简单，如下所示:
- 输入：N个蒙特卡罗完整序列,训练步长 $α$
- 输出：策略函数的参数 $θ$
  - (1). for 每个蒙特卡罗序列:
    - a. 用蒙特卡罗法计算序列每个时间位置t的状态价值 $v\_t$
    - b. 对序列每个时间位置t，使用梯度上升法，更新策略函数的参数 $θ$：
      - $$\theta=\theta+\alpha\nabla\_\theta log\pi\_\theta(s\_t,a\_t)v\_t$$
  - (2).返回策略函数的参数 $θ$


　　这里的策略函数可以是softmax策略，高斯策略或者其他策略。

#  6. 策略梯度实例

这里给出第5节的蒙特卡罗策略梯度reinforce算法的一个实例。仍然使用了OpenAI Gym中的CartPole-v0游戏来作为我们算法应用。CartPole-v0游戏的介绍参见[这里](https://github.com/openai/gym/wiki/CartPole-v0)。它比较简单，基本要求就是控制下面的cart移动使连接在上面的pole保持垂直不倒。这个任务只有两个离散动作，要么向左用力，要么向右用力。而state状态就是这个cart的位置和速度， pole的角度和角速度，4维的特征。坚持到200分的奖励则为过关。

 完整的代码参见我的github：https://github.com/ljpzzz/machinelearning/blob/master/reinforcement-learning/policy_gradient.py

这里我们采用softmax策略作为我们的策略函数，同时，softmax的前置部分，也就是我们的策略模型用一个三层的softmax神经网络来表示。这样好处就是梯度的更新可以交给神经网络来做。

我们的softmax神经网络的结构如下，注意这个网络不是价值Q网络，而是策略网络：

```python
def create_softmax_network(self):
  # network weights
  W1 = self.weight_variable([self.state_dim, 20])
  b1 = self.bias_variable([20])
  W2 = self.weight_variable([20, self.action_dim])
  b2 = self.bias_variable([self.action_dim])
  # input layer
  self.state_input = tf.placeholder("float", [None, self.state_dim])
  self.tf_acts = tf.placeholder(tf.int32, [None, ], name="actions_num")
  self.tf_vt = tf.placeholder(tf.float32, [None, ], name="actions_value")
  # hidden layers
  h_layer = tf.nn.relu(tf.matmul(self.state_input, W1) + b1)
  # softmax layer
  self.softmax_input = tf.matmul(h_layer, W2) + b2
  #softmax output
  self.all_act_prob = tf.nn.softmax(self.softmax_input, name='act_prob')
  self.neg_log_prob = tf.nn.sparse_softmax_cross_entropy_with_logits(logits=self.softmax_input, labels=self.tf_acts)
  self.loss = tf.reduce_mean(self.neg_log_prob * self.tf_vt)  # reward guided loss

  self.train_op = tf.train.AdamOptimizer(LEARNING_RATE).minimize(self.loss)
```

注意我们的损失函数是softmax交叉熵损失函数和状态价值函数的乘积，这样TensorFlow后面可以自动帮我们做梯度的迭代优化。

另一个要注意的点就是蒙特卡罗法里面价值函数的计算，一般是从后向前算，这样前面的价值的计算可以利用后面的价值作为中间结果，简化计算，对应代码如下：

```python
def learn(self):
  discounted_ep_rs = np.zeros_like(self.ep_rs)
  running_add = 0
  for t in reversed(range(0, len(self.ep_rs))):
      running_add = running_add * GAMMA + self.ep_rs[t]
      discounted_ep_rs[t] = running_add

  discounted_ep_rs -= np.mean(discounted_ep_rs)
  discounted_ep_rs /= np.std(discounted_ep_rs)
```

其余部分和之前的DQN的代码类似。

# 7. 策略梯度小结

策略梯度提供了和DQN之类的方法不同的新思路，但是我们上面的蒙特卡罗策略梯度reinforce算法却并不完美。由于是蒙特卡罗法，我们需要完全的序列样本才能做算法迭代，同时蒙特卡罗法使用收获的期望来计算状态价值，会导致行为有较多的变异性，我们的参数更新的方向很可能不是策略梯度的最优方向。

因此，Policy Based的强化学习方法还需要改进，注意到我们之前有Value Based强化学习方法，那么两者能不能结合起来一起使用呢？下一篇我们讨论Policy Based+Value Based结合的策略梯度方法Actor-Critic。

　　　　

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/rl_learning_note_13/  

