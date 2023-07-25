# Transformer Introduction


reference:</br>
[1]. [The Transformer Family ](https://lilianweng.github.io/posts/2020-04-07-the-transformer-family/)
[2]. [Attention](https://lilianweng.github.io/posts/2018-06-24-attention/)
[3]. [细节考究](https://zhuanlan.zhihu.com/p/60821628)


## Transformer Family
### Notations
|Symbol|Meaning|
|---|---|
|$d$|The model size / hidden state dimension / positional encoding size.|
|$h$|The number of heads in multi-head attention layer.|
|$L$|The segment length of input sequence.|
|$X \in \mathbb R ^ {L \times d}$|The input sequence where each element has been mapped into an embedding vector of shape , same as the model size.|
|$W^k \in \mathbb R ^ {d \times d^k}$|The key weight matrix.|
|$W^q \in \mathbb R ^ {d \times d^k}$|The query weight matrix.|
|$W^v \in \mathbb R ^ {d \times d^k}$|The value weight matrix.Often we have $d_k = d_v = d$.|
|$W^K_i, W^q_i \in \mathbb R ^ {d \times d^k / h}; W^v_i \in \mathbb R^{d x d_v / h}$|The weight matrices per head.|
|$W^o \in \mathbb d_v \times d$|The output weight matrix.|
|$Q = XW^q \in \mathbb R^{L \times d_q}$|The query embedding inputs.|
|$K = XW^k \in \mathbb R^{L \times d_k}$|The key embedding inputs.|
|$V = XW^v \in \mathbb R^{L \times d_v}$|The value embedding inputs.|
|$S_i$|A collection of key positions for the -th query to attend to.|
|$A \in \mathbb R ^ {L \times L}$|The self-attention matrix between a input sequence of lenght $L$ and itself. $A = softmax (Q K^T/\sqrt{(d_k)} )$|
|$a_ij \ in A $|The scalar attention score between query $q_i$ and key $k_j$.|
|$P \in \mathbb R ^ {L \times d}$|position encoding matrix, where the $i-th$ row is the positional encoding for input $x_i$.|

### Attention and Self-Attention

Attention is a mechanism in the neural network that a model can learn to make predictions by **selectively attending to a given set of data**. The amount of attention is quantified by learned weights and thus the output is usually formed as a weighted average.

Self-attention is a type of attention mechanism where the model makes prediction for one part of a data sample using other parts of the observation about the same sample. Conceptually, it feels quite similar to non-local means. Also note that self-attention is permutation-invariant; in other words, it is an operation on sets.

There are various forms of attention / self-attention, Transformer ([Vaswani et al., 2017](https://arxiv.org/abs/1706.03762)) relies on the scaled dot-product attention: given a query matrix $Q$, a key matrix $K$ and a value matrix $V$, the output is a weighted sum of the value vectors, where the weight assigned to each value slot is determined by the dot-product of the query with the corresponding key:

$$\text{Attention}(Q, K, V) = softmax(\frac{QK^T}{\sqrt{d_k}})V$$

And for a query and a key vector $q_i, k_j \in \mathbb R ^ d$ (row vectors in query and key matrices), we have a scalar score:

$$a_{ij} = softmax(\frac{q_i k_j^T}{\sqrt{d_k}}) = \frac{\exp(q_i k_j^T)}{\sqrt{d_k}\sum_{r \in S_i}(q_i k_j^T)}$$

where $S_i$ is a collection of key positions for the $i$-th query to attend to.

See my old [post](https://lilianweng.github.io/posts/2018-06-24-attention/#a-family-of-attention-mechanisms) for other types of attention if interested.

### Multi-Head Self-Attention

The multi-head self-attention module is a key component in Transformer. Rather than only computing the attention once, the multi-head mechanism splits the inputs into smaller chunks and then computes the scaled dot-product attention over each subspace in parallel. The independent attention outputs are simply concatenated and linearly transformed into expected dimensions.

$$\text{MulitHeadAttention}(X_q, X_k, X_v) = [\text{head}_1,;...; \text{head}_h] W^o, where \text{head}_i = \text{Attention}(X_qW_i^q, X_kW_i^k, X_vW_i^v)$$

where $[.;.]$ is a concatenation operation. $W_i^q, W_i^k \in \mathbb R^{d \times d_{k} / h}$, $W_i^v \in \mathbb R^{d \times d_{v} / h}$ are weight matrices to map input embeddings of size $L \times d$ into query, key and value matrices. And $W^o \in \mathbb R ^ {d_v \times d}$ is the output linear transformation. All the weights should be learned during training.

![Fig. 1. Illustration of the multi-head scaled dot-product attention mechanism. ](images/Transformer_multi-head-attention.png)

### Transformer

The Transformer (which will be referred to as “vanilla Transformer” to distinguish it from other enhanced versions; [Vaswani, et al., 2017](https://arxiv.org/abs/1706.03762)) model has an encoder-decoder architecture, as commonly used in many [NMT](https://lilianweng.github.io/posts/2018-06-24-attention/#born-for-translation) models. Later decoder-only Transformer was shown to achieve great performance in language modeling tasks, like in [GPT and BERT](https://lilianweng.github.io/posts/2019-01-31-lm/#openai-gpt).

**Encoder-Decoder Architecture**

The encoder generates an attention-based representation with capability to locate a specific piece of information from a large context. It consists of a stack of 6 identity modules, each containing two submodules, a multi-head self-attention layer and a point-wise fully connected feed-forward network. By point-wise, it means that it applies the same linear transformation (with same weights) to each element in the sequence. This can also be viewed as a convolutional layer with filter size 1. Each submodule has a residual connection and layer normalization. All the submodules output data of the same dimension $d$.

The function of Transformer decoder is to retrieve information from the encoded representation. The architecture is quite similar to the encoder, except that the decoder contains two multi-head attention submodules instead of one in each identical repeating module. The first multi-head attention submodule is masked to prevent positions from attending to the future.

![Fig. 2. The architecture of the vanilla Transformer model ](images/Transformer_vanilla_transformer_model.png)

**Positional Encoding**

Because self-attention operation is permutation invariant, it is important to use proper **positional encoding** to provide order information to the model. The positional encoding $P \in \mathbb R ^ {L \times d}$ has the same dimension as the input embedding, so it can be added on the input directly. The vanilla Transformer considered two types of encodings:

(1). Sinusoidal positional encoding is defined as follows, given the token $i = 1, ..., L$ position and the dimension $\delta = 1, ..., d$:

$$ \text{PE}(i, \delta)  = \left\{
    \begin{aligned}
\sin\big(\frac{i}{10000^{2\delta'/d}}\big) , if \delta&=2\delta'\\
\cos\big(\frac{i}{10000^{2\delta'/d}}\big) , if \delta&=2\delta'+1 \\
\end{aligned}
\right.$$

In this way each dimension of the positional encoding corresponds to a sinusoid of different wavelengths in different dimensions, from $2\pi$ to 10000 * $2\pi$.

![Fig. 2. Sinusoidal positional encoding](images/Transformer_sinusoidal_positional_encoding.png)

(2). Learned positional encoding, as its name suggested, assigns each element with a learned column vector which encodes its absolute position ([Gehring, et al. 2017](https://arxiv.org/abs/1705.03122)).


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/transformerintroduction/  

