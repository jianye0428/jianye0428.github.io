# Diffusion 扩散模型（DDPM）


## 一、引入

![](images/DDPM_0.png)

近年AIGC的爆火离不开人工智能在图像生成、文本生成以及多模态等领域的不断累积，其中**生成模型**的发展占据了很大功劳，如：<mark>生成对抗网络 GAN</mark> 及其一系列变体、<mark>变分自编码器 VAE</mark> 及其一系列变体、<mark>自回归模型 AR</mark>、<mark>流模型 flow</mark> ，以及近年大火的**扩散模型 Diffusion Model** 等。

扩散模型的大火并非横空出世，早在2015年就有人提出了类似的想法，直到2020年才提出了经典的 **Denoising Diffusion Probabilistic Models（DDPM）**，像OpenAI、NovelAI、NVIDIA和Google成功的训练了大规模模型之后，它们吸引了很多人注意，后续有了很多基于扩散模型的变体，比如有：GLIDE、DALLE-2、Imagen和年底爆火的完全开源的稳定扩散模型（Stable Diffusion）。

扩散模型与之前所有的生成方法有着本质的区别：

![](images/DDPM_1.png)

直观的说它是<mark>将图像生成过程（采样）分解为许多小的去噪步骤</mark>，其实 Diffusion 的含义本质上就是一个迭代过程，实线箭头用于扩散步骤中添加随机噪声，虚线箭头代表的是通过学习逆向扩散过程<mark>从噪声中重构所需的数据样本</mark>。**引入噪声导致了信息的衰减，再通过噪声尝试还原原始数据，多次迭代最小化损失后，能够使模型在给定噪声输入的情况下学习生成新图像。**

所以Diffusion模型和其它生成模型的区别是，它不是直接的**图像->潜变量、潜变量->图像**的一步到位，它是一步一步的<mark><font color=red>**逐渐分解、逐渐去噪**</font></mark>的过程。

当然有关Diffusion的理解和变体有很多，但是扩散模型从本质上讲就是DDPM，所以本文主要对DDPM的原理进行讲解，并给出DDPM的扩散过程、去噪过程、训练损失的详细推导，对于掌握Diffusion算法原理只需要抓住以下四点即可：
- 前向过程（扩散）；
- 反向过程（去噪、采样）；
- 如何训练；
- 如何推断。

## 二、扩散原理阐述

扩散模型包括 **前向扩散过程** 和 **反向去噪过程(采样)**，前向阶段对图像逐步施加噪声，直至图像被破坏变成完全的高斯噪声，然后在反向阶段学习从高斯噪声还原为原始图像的过程。

### 2.1、直观理解

- 扩散模型的目的是什么？
  - 学习从纯噪声生成图片的方法。
- 扩散模型是怎么做的？
  - 训练一个UNet，接受一系列加了噪声的图片，学习预测所加的噪声。
- 前向过程在干什么？
  - 逐步向真实图片添加噪声最终得到一个纯噪声；
  - 对于训练集中的每张图片，都能生成一系列的噪声程度不同的加噪图片；
  - 在训练时，这些 【不同程度的噪声图片 + 生成它们所用的噪声】 是实际的训练样本。
- 反向过程在干什么？
  - 训练好模型后，采样、生成图片。

### 2.2、前向过程（扩散）
![](images/DDPM_2.png)

前向过程在原始输入图像$x_0$上逐步添加随机噪声，这个噪声服从高斯分布$N(0, 1)$，每一步得到的图像$x_t$只和上一步的加噪结果$x_{t-1}$相关，逐步添加噪声至$T$步，可以得到趋向于纯粹噪声的图像，如下图所示：
![](images/DDPM_3.png)

> 后面有详细的推导，公式比较多，这里先提前把主要的列一下方便阐述。

对于将一张图片，从$x_{t-1}\rightarrow x_{t}$的逐步加噪破坏的公式为：

$$x_t=\sqrt{\alpha_t}\left.x_{t-1}+\sqrt{1-\alpha_t}\right.\varepsilon_t\quad\quad\quad\quad\quad\quad(1)$$

其中:
- $x_t$表示第$t$步的图像；
- $\varepsilon$ 是一个满足正态分布的随机噪声，$\varepsilon \sim N(0, 1)$；
- $\sqrt{\alpha_{t}}$ 是图片的权重，$\sqrt{1 - \alpha_{t}}$ 是噪声的权重；

定义：
- $\alpha_t=1-\beta_t$
- $\overline{\alpha}=\prod_{s=1}^t\alpha_s$

随着$t$的增加，**噪声的占比会越来越大**，所以添加的**噪声强度也会越来越大**，也就是说图片的权重要越来越小，噪声的权重要越来越大。因为随着扩散过程的增加，图像中噪声的占比也会越来越大，我们想要进一步破坏它的结构，就需要添加更多的噪声。

> 换句话说，一开始图像比较清晰，这个时候添加的噪声小一些，随着图像的噪声越来越多，这个时候再加一点噪声的话，对原来的图像就没什么影响了，因为它本身就有好多噪声了，所以随着图像的噪声越来越多，后面的步骤就要加更多的噪声。

实际训练过程中会比较大（DDPM原文中为1000），所以会有从$x_0$递推到$x_t$的公式：

$$x_t=\sqrt{\overline{\alpha}_t}\left.x_0+\sqrt{1-\overline{\alpha}_t}\right.\varepsilon\quad\quad\quad\quad(2)$$

其中：
- $\alpha_t$、$\beta_t$ 有一个固定的已知函数，是可以直接进行计算的；
- $\varepsilon$ 为随机产生的噪声；

所以整个式子是已知的，式 $(1)$、$(2)$ 就可以描述前向过程了，$(1)$ 用于将一张图片的逐步破坏，$(2)$ 用于一步到位的破坏。

### 2.3、反向过程（去噪）

反向过程则是不断去除噪声的过程，给定一个噪声图片 $x_T$，对它一步步的去噪还原，直至最终将原始图像 $x_0$ 给恢复出来，如下图所示：

![](images/DDPM_4.png)

去噪的过程，$x_t$、$\alpha_t$、$\beta_t$ 都是已知的，只有公式 $(2)$ 中的真实噪声是未知的，因为它是随机采样的。所以需要一个神经网络把 $\varepsilon$ 给学出来，也就是说训练一个由 $x_t$ 和 $t$ 估测噪声的模型:

$$x_{t-1}=\frac{1}{\sqrt{\alpha_t}}(x_t-\frac{\beta_t}{\sqrt{1-\overline{\alpha}_t}}\varepsilon_\theta(x_t,t))$$

其中 $\theta$ 就是模型的参数，通常使用UNet作为预估噪声的模型。

### 2.4、模型训练

所以说反向过程其实就是**训练网络去学习分解过程每一步的噪声**，当网络训练好之后，输入一张噪声图片，通过网络就能把加的噪声给求出来，噪声有了代入公式，就能把 $x_{t-1}$ 步的比较清晰的图给求出来了，一步步往前迭代就行了。

采用L2距离刻画相近程度就可以，DDPM的关键是训练 $\varepsilon_{\theta}(x_t, t)$，目的就是使预测的噪声与真实用于破坏的噪声相近：

$$Loss=\mid\mid\varepsilon-\varepsilon_\theta(x_t,t)\mid\mid^2=\mid\mid\varepsilon-\varepsilon_\theta(\sqrt{\overline{\alpha}_t}~x_0+\sqrt{1-\overline{\alpha}_t}~\varepsilon_t,t)\mid\mid^2$$

![](images/DDPM_5.png)

模型训练完后，只要给定随机高斯噪声，就可以生成一张从未见过的图像。


UNet本文不做介绍，结构图为：

![U_Net 结构图](images/DDPM_6.png)

> 额外强调的是：Unet里有一个位置编码，是关于时间步的，每个时间步是有一个线性调度器的，每个时间添加的噪声的方差是不一样的，所以将时间步作为编码嵌入的话，可以将模型预测的噪声更加的准确。

## 三、算法流程概述

![](images/DDPM_7.png)

再次总结，扩散模型两个步骤如下：
- 一个固定的（预先定义好的）前向扩散过程 $q(x_t | x_{t-1})$：逐步向图片增加噪声直到最终得到一张纯粹的噪声图；
- 一个学习得到的去噪过程 $p_{\theta}(x_{t-1} | x_t)$：训练一个神经网络去逐渐的从一张纯噪声中消除噪声，直到得到一张真正的图片。

![](images/DDPM_8.png)

算法1 为训练流程：
 - line2：从数据中采样 $x_0$，$q(x_0)$ 的意思是给 $x_0$ 加上噪声；
 - line3：随机选取 time step $t$；
   - 真实训练过程中我们不可能一步步的从 $t$ 到 $T$，因为会很大，这就意味着每输入一张图片 $x$，就会产生张噪声图像，也就是一张图像的网络要训练 $T$ 个噪声样本，非常耗时。
   - 所以对 $T$ 进行了采样，$t$ 就是从 $T$ 里采集若干个的意思。
   - 举个例子：假设采集 $t$ 的分别为100、20、3，对应的 $x$ 为 $x_{100}$、$x_{20}$、$x_{3}$，对应噪声为 $\varepsilon_{100}$、$\varepsilon_{20}$、$\varepsilon_{3}$，对于的预测噪声为 $\hat{\varepsilon}_{100}$、$\hat{\varepsilon}_{20}$、$\hat{\varepsilon}_{3}$, 只需要将 $\varepsilon$ 和 $\hat{\varepsilon}$ 代入MSE公式即可（相减、平方、最小化）。
- line 4：生成随机高斯噪声；
- line 5：调用模型估计 $\varepsilon_{\theta}(\sqrt{\overline{\alpha}_t}~x_0+\sqrt{1-\overline{\alpha}_t}~\varepsilon_t,t)$ ，计算真实噪声与估计噪声之间的MSE Loss，反向传播更新模型。
  - 网络的作用是预测噪声，随着的增加，噪声强度会越来越大，因此预测的噪声是和迭代是直接相关的，所以要把作为参数送入到网络当中。
- 直到收敛。

算法2 为采样流程：

- line 1：从高斯分布采样 $x_T$；
- line 2：按照 $T, ..., 1$ 的顺序进行迭代；
- line 3：如果 $t = 1$ 令 $z = 0$；如果 $t > 1$ ，从高斯分布中采样；
- line 4：利用公式求出均值和方差，进而求得 $x_{t-1}$；
- 经过上述迭代，恢复 $x_0$。

## 四、数学描述

我们来推导如何从原始图像直接到第t时刻的图像 $(X_0 - X_t)$。

首先回顾 2.1小节 的两个定义：
- $\alpha_t = 1 - \beta_{t}$, $\beta_t$ 要越大越好，论文中从0.0001到0.02;
- $\overline{\alpha}=\prod_{s=1}^t\alpha_s$累乘，下面会用到；
- $x_t=\sqrt{\alpha_t}x_{t-1}+\sqrt{1-\alpha_t}\varepsilon_t\text{,}\varepsilon_t\sim N(0,1)$ 每一时刻添加的噪声均独立；

我们要求$x_t$时刻的图像，它需要一步步的加噪迭代，这样太慢了。因为每一步添加的噪声独立且服从正太分布，我们可以做如下推导：

> 为了不混淆，只需要记住：**下标越小，噪声越小**，即 $x_{t-1}$ 的噪声是小于 $x_t$ 的。

$$
\begin{aligned}
q(x_{t}\mid x_{t-1})& =N(x_t;\sqrt{\alpha_t}x_{t-1},(1-\alpha_t)I)  \cr
&=\underbrace{\sqrt{\alpha_t}x_{t-1}}_{x_{t-2}\text{来表示}x_{t-1}}+\sqrt{1-\alpha_t}\varepsilon_t \cr
&=\sqrt{\alpha_t}\left(\sqrt{\alpha_{t-1}}\right.x_{t-2}+\sqrt{1-\alpha_{t-1}}\left.\varepsilon_{t-1}\right)+\sqrt{1-\alpha_t}\left.\varepsilon_t\right. \cr
&=\sqrt{\alpha_t\alpha_{t-1}}x_{t-2}+\underbrace{\sqrt{\alpha_t-\alpha_t\alpha_{t-1}}\varepsilon_{t-1}+\sqrt{1-\alpha_t}\varepsilon_t}_{\text{两个独立正太分布相加}} \cr
&=\sqrt{\alpha_t\alpha_{t-1}}\left.x_{t-2}+\sqrt{1-\alpha_t\alpha_{t-1}}\right.\varepsilon  \cr
&\text{...} \\
&=\sqrt{\overline{\alpha}_t}\left.x_0+\sqrt{1-\overline{\alpha}_t}\right.\varepsilon  \cr
&\therefore q(x_t\mid x_0)=N(x_t;\sqrt{\overline{\alpha}_t}x_0,\sqrt{1-\overline{\alpha}_t}I)
\end{aligned}
$$

> 上述用的就是重参数化技巧。

方差参数 $\beta_{t}$ 可以固定为一个常数，也可以选择作为 $T$ 时间段的一个时间表。事实上，人们可以定义一个方差表，它可以是线性的、二次的、余弦的等等。最初的DDPM作者利用了一个从 $\beta_1 = 10^{-4}$ 到$\beta_T = 0.02$增加的线性时间表。Nichol等人2021年的研究表明，采用余弦时间表效果更好。

![](images/DDPM_9.png)

### 4.2、反向过程（去噪）

接下来是反向过程的推导：
$$p(x_{t-1}\mid x_t)=N(x_{t-1};\underbrace{\mu_\theta(x_t,t)}_\text{要反预测这个},\overbrace{\Sigma_\theta(x_t,t)}^{fixed})$$

给定$x_t$要预测 $x_{t-1}$，它是一个高斯分布，$x_t$和$t$的方差是固定的，论文作者使用原始的噪声调度器作为方差，也就是说噪声调度器一旦确立，方差的大小也就固定了。所以我们只需要预测这个均值就好了，下面给出具体的推导过程：

我们先看整个损失函数，是个**负对数似然**：

$$-\log{p_{\theta}(x_0)}$$

希望神经网络的参数 $\theta$，可以使得生成 $x_0$的概率越大越好。

但问题在于 $x_0$ 的概率不好计算，因为它依赖于 $x_0$ 之前的所有步长，从 $x_T$ 开始。作为一种解决方案，我们可以计算这个目标的**变分下界**，并得到一个更易于计算的公式：

$$-log(p_\theta(x_0))\leq-log(p_\theta(x_0))+D_{KL}(q(x_{1:T}\mid x_0)\parallel p_\theta(x_{1:T}\mid x_0))$$

其中：
- $x_{1:T}$ 指的是 $x_1, ..., x_T$ 整个序列。

现在依然无法计算，我们继续推导：

$$
\begin{gathered}
-log(p_\theta(x_0)) \leq-log(p_\theta(x_0))+D_{KL}(q(x_{1:T}\mid x_0)\mid\mid p_\theta(x_{1:T}\mid x_0)) \cr
\leq-log(p_\theta(x_0))+log(\frac{q(x_{1:T}\mid x_0)}{p_\theta(x_{1:T}\mid x_0)})
\end{gathered}
$$

我们将 KL divergence 改写后，再利用贝叶斯公式进行变形，即分母可以改写为：

$$
\begin{aligned}
p_\theta(x_{1:T}\mid x_0) &=\frac{p_\theta(x_0\mid x_{1:T})\mathrm{~}p_\theta(x_{1:T})}{p_\theta(x_0)} \cr
&=\frac{p_\theta(x_0,x_{1:T})}{p_\theta(x_0)} \cr
&=\frac{p_\theta(x_{0:T})}{p_\theta(x_0)}
\end{aligned}
$$

将其代回原式：

$$
\begin{aligned}
log(\frac{q(x_{1:T}\mid x_0)}{p_\theta(x_{1:T}\mid x_0)})& =log(\frac{q(x_{1:T}\mid x_0)}{\frac{p_\theta(x_{0:T})}{p_\theta(x_0)}})  \cr
&=log(\frac{q(x_{1:T}\mid x_0)}{p_\theta(x_{0:T})})+log(p_\theta(x_0))
\end{aligned}
$$

所以原式可简化为：

$$-log(p_\theta(x_0))\leq\underbrace{log(\frac{q(x_{1:T}\mid x_0)}{p_\theta(x_{0:T})})}_{\text{变分下界,可以优化它}}$$

- 分子，就是前向过程，它是固定的，从 $x_0$ 到 $x_{1:T}$ 的采样，换句话说就是从我们数据中的一些图像开始；
- 分母，$p_\theta(x_{0:T})=p(x_T)\prod_{t=1}^Tp_\theta(x_{t-1}\mid x_t)$；
  - 将 $p(x_T)$ 提出来，是因为 $p(x_T)$ 是指当前图像，它是不依赖于网络参数 $\theta$ 的.

  $$
  \begin{aligned}
  log(\frac{q(x_{1:T}\mid x_0)}{p_{\theta}(x_{0:T})})& =log(\frac{\prod_{t=1}^Tq(x_t\mid x_{t-1})}{p(x_T)\prod_{t=1}^Tp_\theta(x_{t-1}\mid x_t)})  \cr
  &=-log(p(x_T))+log(\frac{\prod_{t=1}^Tq(x_t\mid x_{t-1})}{\prod_{t=1}^Tp_\theta(x_{t-1}\mid x_t)}) \cr
  &=-log(p(x_T))+\sum_{t=1}^Tlog(\frac{q(x_t\mid x_{t-1})}{p_\theta(x_{t-1}\mid x_t)}) \cr
  &=-log(p(x_T))+\sum_{t=2}^Tlog(\frac{q(x_t\mid x_{t-1})}{p_\theta(x_{t-1}\mid x_t)})+\underbrace{log(\frac{q(x_1\mid x_0)}{p_\theta(x_0\mid x_1)})}_{t=1}
  \end{aligned}
  $$

![](images/DDPM_10.png)

$q(x_t|x_{t-1})$ 根据贝叶斯公式可以变换如下：

$$q(x_t\mid x_{t-1})=\frac{q(x_{t-1}\mid x_t)q(x_t)}{q(x_{t-1})}$$



$q(x_{t-1}|x_{t})$具有比较高的方差，因为根据这张照片，我们无法确定它来自哪里，但是引入 $x_0$，我们就可以容易的预测出 $x_{t-1}$，

![](images/DDPM_11.png)

因此我们使用：

$$\frac{q(x_{t-1}\mid x_t,x_0)\mathrm{~}q(x_t\mid x_0)}{q(x_{t-1}\mid x_0)}$$

替换贝叶斯重写后的式子，我们得到：

$$
\begin{aligned}
log(\frac{q(x_{1:T}\mid x_0)}{p_{\theta}(x_{0:T})})& =-log(p(x_T))+\sum_{t=2}^Tlog(\frac{q(x_{t-1}\mid x_t,x_0)q(x_t\mid x_0)}{p_\theta(x_{t-1}\mid x_t)q(x_{t-1}\mid x_0)})+log(\frac{q(x_1\mid x_0)}{p_\theta(x_0\mid x_1)})  \cr
&=-log(p(x_T))+\sum_{t=2}^Tlog(\frac{q(x_{t-1}\mid x_t,x_0)}{p_\theta(x_{t-1}\mid x_t)})+\underbrace{\sum_{t=2}^Tlog(\frac{q(x_t\mid x_0)}{q(x_{t-1}\mid x_0)})}+log(\frac{q(x_1\mid x_0)}{p_\theta(x_0\mid x_1)})
\end{aligned}
$$

上述标记的式子，也可以简化，我们假设 $t=4$：

$$
\begin{gathered}
\begin{aligned}\sum_{t=2}^{T=4}log(\frac{q(x_t\mid x_0)}{q(x_{t-1}\mid x_0)})\end{aligned} =log(\frac{q(x_2\mid x_0)}{q(x_1\mid x_0)}\cdot\frac{q(x_3\mid x_0)}{q(x_2\mid x_0)}\cdot\frac{q(x_4\mid x_0)}{q(x_3\mid x_0)}) \\
=log(\frac{q(x_4\mid x_0)}{q(x_1\mid x_0)})
\end{gathered}
$$

因此我们可以简化为：

$$
\begin{aligned}
&=-log(p(x_T))+\sum_{t=2}^Tlog(\frac{q(x_{t-1}\mid x_t,x_0)}{p_\theta(x_{t-1}\mid x_t)})+log(\frac{q(x_t\mid x_0)}{q(x_1\mid x_0)})+log(\frac{q(x_1\mid x_0)}{p_\theta(x_0\mid x_1)}) \cr
&=-log(p(x_T))+\sum_{t=2}^Tlog(\frac{q(x_{t-1}\mid x_t,x_0)}{p_\theta(x_{t-1}\mid x_t)})+log(q(x_t\mid x_0))-log(p_\theta(x_0\mid x_1)) \cr
&=log(\frac{q(x_t\mid x_0)}{p(x_T)})+\sum_{t=2}^Tlog(\frac{q(x_{t-1}\mid x_t,x_0)}{p_\theta(x_{t-1}\mid x_t)})-log(p_\theta(x_0\mid x_1))\cr
&=\overbrace{\underbrace{D_{KL}(q(x_t\mid x_0)\mid\mid p(x_T))}_{q\text{只是个正向过程没有可学习参数}}}^{\text{可以忽略}} + \sum_{t=2}^TD_{KL}(q(x_{t-1}\mid x_t,x_0)\mid\mid p_\theta(x_{t-1}\mid x_t))-log(p_\theta(x_0\mid x_1))
\end{aligned}
$$

- 第一项KL散度可以忽略，因为$q$只是个正向过程，没有可学习参数，换句话说就是它是固定的。
- 第二项KL散度，左边和右边都是正太分布，分别服从 $N(x_{t-1};\tilde{\mu_t}(x_t,x_0),\tilde{\mathsf{\beta}_t}I)$ 、$N(x_{t-1};\mu_\theta(x_t,t),\text{β}I)$：

$$
\sum_{t=2}^TD_{KL}(\underbrace{q(x_{t-1}\mid x_t,x_0)}_{N(x_{t-1};\tilde{\mu}_t(x_t,x_0),\tilde{\mathsf{\beta}}_tI)}\mid\mid\overbrace{p_\theta(x_{t-1}\mid x_t)}^{N(x_{t-1};\mu_\theta(x_t,t),\mathsf{\beta}I})
$$

第一项的 $\tilde{\mu_{t}}(x_{t},x_{0})$、$\tilde{\beta_{t}}$ 就是我们要求的值，这里省略了这部分的推导，不影响算法的理解，

$$
\begin{gathered}\tilde{\mu}_t(x_t,x_0)=\frac{\sqrt{\alpha_t}(1-\overline{\alpha}_{t-1})}{1-\overline{\alpha}_t}x_t+\frac{\sqrt{\alpha}_{t-1}\beta_t}{1-\overline{\alpha}_t}x_0\\\tilde{\mathsf{\beta}}_t=\frac{1-\overline{\alpha}_{t-1}}{1-\overline{\alpha}_t}\beta_t\end{gathered}
$$

> 凡是涉及到 $\alpha_t$ 的，就是学习调度器的，我们不需要关注它

我们可以化简 $\tilde{\mu}_{t}$，我们知道 $x_t=\sqrt{\overline{\alpha}_t}x_0+\sqrt{1-\overline{\alpha}_t}\varepsilon $, 即:

$$
x_0=\frac1{\sqrt{\overline{\alpha}_t}}(x_t-\sqrt{1-\overline{\alpha}_t}\left.\varepsilon\right)
$$

还知道: $\overline{\alpha}=\prod_{s=1}^t\alpha_s$、$\alpha_t=1-\beta_t$:

代入 $\tilde{\mu}_{t}$ 得到：

$$
\begin{aligned}
\underbrace{\tilde{\mu}_t(x_t,x_0)}_{\text{不再依赖}x_0}& =\frac{\sqrt{\alpha_{t}}(1-\overline{\alpha}_{t-1})}{1-\overline{\alpha}_{t}}x_{t}+\frac{\sqrt{\overline{\alpha}_{t-1}}\beta_{t}}{1-\overline{\alpha}_{t}}\frac{1}{\sqrt{\overline{\alpha}_{t}}}(x_{t}-\sqrt{1-\overline{\alpha}_{t}}\varepsilon)  \cr
&=\frac{\alpha_t(1-\overline{\alpha}_{t-1})x_t}{\sqrt{\alpha_t}(1-\overline{\alpha}_t)}+\frac{\beta_t}{\sqrt{\alpha_t}(1-\overline{\alpha}_t)}(x_t-\sqrt{1-\overline{\alpha}_t}\left.\varepsilon\right) \cr
&=\frac{\alpha_tx_t-\overline{\alpha}_tx_t+(1-\alpha_t)x_t-(1-\alpha_t)\sqrt{1-\overline{\alpha}_t}\varepsilon}{\sqrt{\alpha_t}(1-\overline{\alpha}_t)} \cr
&=\frac{x_t(1-\overline{\alpha}_t)-(1-\alpha_t)\sqrt{1-\overline{\alpha}_t}\varepsilon}{\sqrt{\alpha_t}(1-\overline{\alpha}_t)} \cr
&=\frac{x_t}{\sqrt{\alpha_t}}-\frac{(1-\alpha_t)\varepsilon}{\sqrt{\alpha_t}\sqrt{(1-\overline{\alpha}_t)}} \cr
&=\frac1{\sqrt{\alpha_t}}(x_t-\frac{\beta_t}{\sqrt{1-\overline{\alpha}_t}}\left.\varepsilon\right)
\end{aligned}
$$

代入之后我们发现它就不再依赖于 $x_0$ 了，它就是和 $x_t$ 的一个关系式，式中的 $\alpha_t$、$\beta_t$、$\varepsilon$都是已知的，最后的本质就是我们只是从中减去缩放的随机噪声。

$$\therefore x_{t-1}=N(x_{t-1};\frac1{\sqrt{\alpha_t}}(x_t-\frac{\beta_t}{\sqrt{1-\overline{\alpha}_t}}\left.\varepsilon_\theta(x_t,t)\right),\Sigma_\theta(x_t,t))$$

这样一来，DDPM的每一步推断可以总结为：

- 每个时间步通过 $x_t$ 和 $t$ 来预测高斯噪声，图中用 $z$ 表示，根据上述公式计算得到均值 $\mu$；
- 得到方差 $\Sigma_\theta(x_t,t)$
- 入公式得到 $q(x_{t-1}\mid x_t)$ ，利用重参数化得到 $x_{t-1}$ 。

![](images/DDPM_12.png)

### 4.3、训练损

下面我们来看损失的推导，我们来回顾第二项：

![](images/DDPM_13.png)

我们需要减小KL散度，由于<mark>方差是固定的，我们无法优化，所以需要将它们的均值之差减小</mark>，原论文中使用的是简单的均方误差：

将$\mu$表达式代入：

$$
\begin{aligned}
L_{t}& =\frac1{2\sigma_t^2}\mid|\tilde{\mu}_t(x_t,x_0)-\mu_\theta(x_t,t)||^2  \cr
&=\frac1{2\sigma_t^2}\mid\mid\frac1{\sqrt{\alpha_t}}(x_t-\frac{\beta_t}{\sqrt{1-\overline{\alpha}_t}}\left.\varepsilon\right)-\frac1{\sqrt{\alpha_t}}(x_t-\frac{\beta_t}{\sqrt{1-\overline{\alpha}_t}}\left.\varepsilon_\theta(x_t,t)\right)\mid\mid^2 \cr
&=\frac{\beta_t^2}{2\sigma_t^2\alpha_t(1-\overline{\alpha}_t)}\underbrace{\mid\mid\varepsilon-\varepsilon_\theta(x_t,t)\mid\mid^2}_{mse} \cr
&->\mid\mid\varepsilon-\varepsilon_\theta(x_t,t)\mid\mid^2=\mid\mid\varepsilon-\varepsilon_\theta(\sqrt{\overline{\alpha}_t}\left.x_0+\sqrt{1-\overline{\alpha}_t}\left.\varepsilon_t,t\right)\mid\mid^2\right.
\end{aligned}
$$

研究人员发现，忽略前面的系数项会变得更简单，采样质量也会得到提高，所以前面这个系数项我们直接忽略，它是和噪声调度器有关的，我们加噪的话也会使计算复杂。

我们最小化 $\mid\mid\varepsilon-\varepsilon_\theta(x_t, t)\mid\mid^2$ 也就是**最小化了KL散度**，KL散度变小了也就是变分上限优化到最小，所以那个负对数似然也会变小。

上面还剩了最后一项 $-log(p_\theta(x_0\mid x_1))$ ，这个作者决定去掉它，即在 $t=1$ 时，我们不添加噪声。也就是下面横线的地方，只有 $t>1$ 的时候才服从高斯分布，如果 $t\leq {1}$，直接让 $z=0$，即噪声设置为0。

![](images/DDPM_14.png)

回顾上面整个推导过程：我们从**负对数似然 -> 优化下界 -> 简化下界 -> 预测噪声**。


## 五、torch复现
https://wangguisen.blog.csdn.net/article/details/128821008

ref:
[1]. https://arxiv.org/abs/2006.11239</br>
[2]. https://kexue.fm/archives/9119</br>
[3]. https://zhuanlan.zhihu.com/p/576475987</br>
[4]. https://zhuanlan.zhihu.com/p/525106459</br>
[5]. https://www.bilibili.com/video/BV1b541197HX</br>
[6]. https://www.bilibili.com/video/BV1WD4y1E7X5</br>
[7]. https://huggingface.co/blog/annotated-diffusion</br>
[8]. https://www.datalearner.com/blog/1051664857725795</br>
[9]. https://lilianweng.github.io/posts/2021-07-11-diffusion-models</br>
[10]. https://mp.weixin.qq.com/s?__biz=Mzk0MzIzODM5MA==&mid=2247486128&idx=1&sn=7ffef5d8c1bbf24565d0597eb5eaeb16&chksm=c337b729f4403e3f4ca4fcc1bc04704f72c1dc02876a2bf83c330e7857eba567864da6a64e18&scene=21#wechat_redirect
[11]. [paper link](https://arxiv.org/pdf/2006.11239.pdf)

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/ddpm/  

