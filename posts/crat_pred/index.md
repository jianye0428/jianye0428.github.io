# CRAT-Prediction



##  Overview
`paper link:`https://arxiv.org/pdf/2202.04488.pdf

## 论文概览

- 文章提出了一种结合Crystal Graph Convolutional Neural Network和Multi-Head Self-Attention Mechanism对交通agent处理的方式

- 在argoverse数据集上进行验证，实现了map-free预测模型的SOTA效果; 相比较于其他模型，模型参数更少。

- 证明: 可以通过 Self-Attention Mechanism 学习到交通参与者之间的交互关系。

## 网络结构

![CART Pred Architecture](images/CART_Pred_Overview.png)

- 数据处理: 以argoverse2数据为例，取前50帧数据，两两作差值，取49组位移向量数据为输入
- - 首先用`EncoderLSTM`作为encoder
- - 再将每一个agent作为node，通过`Crystal Graph Convolutional Neural Network`构建图神经网络
- - 通过`Multi-Head Self-Attention`学习node之间的交互关系

## 实现原理

### Input Encoder 输入编码器
输入数据为过去5秒的离散位移:
$$s_i^t = (\Delta{\tau_i^t} || b_i^t)$$

其中， $\Delta \tau_i^t = \tau_i^{t-1}$.
### Interaction Module 交互模块

### Output Decoder 输出编码器

### Training 训练过程

## 代码实现结构

**数据处理结构**
`input = dict()`
`input['argo_id'] = list()`
`input['city'] = list()`
`input['past_trajs'] = list()`
`input['fut_trajs'] = list()`
`input['gt'] = list()`
`input['displ'] = list()`
`input['centers'] = list()`
`input['origin'] = list()`
`input['rotation'] = list()`

29 + 32 = 61
`argo_id:`
['01d7deae-31e9-4657-843f-c30009b09f1c', '01ca1736-ec51-41aa-8c73-3338c574a83a']
`city:`
['austin', 'austin']
`past_trajs:`
torch.Size([29, 50, 3])
torch.Size([32, 50, 3])
`fut_trajs:`
torch.Size([29, 60, 3])
torch.Size([32, 60, 3])
`gt:`
torch.Size([29, 60, 2])
torch.Size([32, 60, 2])
`displ:`
torch.Size([29, 49, 3])
torch.Size([32, 49, 3])
`centers:`
torch.Size([29, 2])
torch.Size([32, 2])
`origin:`
torch.Size([2])
torch.Size([2])
`rotation:`
torch.Size([2, 2])
torch.Size([2, 2])

**网络输入输出结构详解**
In Inference with two sample data:
`displ_cat:` 61 x 49 x 3
`centers_cat:` 61 x 2
`agents_per_sample:` [32, 29]

### encoder_lstm
**input:** `displ_cat`(61 x 49 x 3), `agents_per_sample` [32,29]
$\downarrow$  input_size = 3; hidden_size = 128; num_layers = 1
$\downarrow$`lstm_hidden_state = torch.randn(num_layers, lstm_in.shape[0], hidden_size) = torch.randn(1, 61, 128)`
$\downarrow$`lstm_cell_state = torch.randn(num_layers, lstm_in.shape[0], hidden_size) = torch.randn(1, 61, 128)`
$\downarrow$`lstm_out, lstm_hidden = self.lstm(lstm_in, lstm_hidden)` => lstm((61, 49, 3), (torch((1, 61, 128)), torch(1, 61, 128)))
$\downarrow$ `lstm_out`(61 x 49 x 128)
**output:** `lstm_out[:,-1,:]`(61 x 128)

### agent_gnn
**input:** `out_encoder_lstm`(61 x 128), `centers_cat` (61 x 2) `agents_per_sample` [32,29]
$\downarrow$ x = gnn_in => (61 x 128)
$\downarrow$ edge_index = build_fully_connected_edge_idx(agents_per_sample).to(gnn_in.device) => (2, 1804) 1804 = (29 x 29-1) + (32 x (32-1))
$\downarrow$
$\downarrow$ edge_attr = build_edge_attr(edge_index, centers).to(gnn_in.device) => (1804, 2)
$\downarrow$ x = F.relu(self.gcn1(x, edge_index, edge_attr)) => (61 x 128)
**output:** gnn_out = F.relu(self.gcn2(x, edge_index, edge_attr)) => (61 x 128)

$$\mathbf{x}^{\prime}_i = \mathbf{x}_i + \sum_{j \in \mathcal{N}(i)}
        \sigma \left( \mathbf{z}_{i,j} \mathbf{W}_f + \mathbf{b}_f \right)
        \odot g \left( \mathbf{z}_{i,j} \mathbf{W}_s + \mathbf{b}_s  \right)$$

### multihead_self_attention
**input:** `out_agent_gnn` (61 x 128) `agents_per_sample`[32,29]
$\downarrow$ max_agents = max(agents_per_sample) => 32
$\downarrow$ padded_att_in = torch.zeros((len(agents_per_sample), max_agents, self.latent_size), device=att_in[0].device) => torch: (2 x 32 x 128)
$\downarrow$ mask = torch.arange(max_agents) < torch.tensor(agents_per_sample)[:, None] && padded_att_in[mask] = att_in => torch: (2 x 32 x 128)
$\downarrow$ padded_att_in_swapped = torch.swapaxes(padded_att_in, 0, 1) => torch: (32, 2, 128)
$\downarrow$ padded_att_in_swapped, _ = self.multihead_attention(padded_att_in_swapped, padded_att_in_swapped, padded_att_in_swapped, key_padding_mask=mask_inverted) => torch: (32, 2, 128)
$\downarrow$ padded_att_in_reswapped = torch.swapaxes(padded_att_in_swapped, 0, 1) => torch: (2, 32, 128)
$\downarrow$ att_out_batch = [x[0:agents_per_sample[i]] for i, x in enumerate(padded_att_in_reswapped)] => list: 2
**output:** `att_out_batch` => list: 2 for each with shape (29, 128) and (32, 128)

### torchstack()
**input:** `out_self_attention:` list: 2 for each with shape (29, 128) and (32, 128)
$\downarrow$ out_self_attention = torch.stack([x[0] for x in out_self_attention])
**output:** `out_self_attention:` torch: (2, 128)

### PredictionNet(out_self_attention)

### decoder_residual
**input:** `out_self_attention`(torch: (2, 128)) `frozen = False`
$\downarrow$ [condition: frozen = False] sample_wise_out.append(PredictionNet(out_self_attention)) => torch: (2, 120)
$\downarrow$ decoder_out = torch.stack(sample_wise_out) => torch: (1, 2, 120)
$\downarrow$ decoder_out = torch.swapaxes(decoder_out, 0, 1) => torch: (2, 1, 120)
**output:** decoder_out => torch: (2, 1, 120)
### out = out_linear.view(len(displ), 1, -1, self.config['num_preds'], 2)
**input:** decoder_out: torch: (2, 1, 120)
$\downarrow$ out = out_linear.view(len(displ), 1, -1, self.config['num_preds'], 2) => torch: (2, 1, 1, 60, 2)
**output:** out => torch: (2, 1, 1, 60, 2)

### 将预测轨迹转换到全局坐标

```
for i in range(len(out)):
	out[i] = torch.matmul(out[i], rotation[i]) + origin[i].view(
                1, 1, 1, -1
            )
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/crat_pred/  

