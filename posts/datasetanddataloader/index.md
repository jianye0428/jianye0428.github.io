# PyTorch Dataset And DataLoader


ref: </br>
[1] https://chenllliang.github.io/2020/02/04/dataloader/</br>
[2] https://blog.csdn.net/zyq12345678/article/details/90268668</br>
[3] https://cloud.tencent.com/developer/article/1877393</br>
</br>
## Dataset

PyTorch为我们提供的两个`Dataset`和`DataLoader`类分别负责可被Pytorch使用的数据集的创建以及向训练传递数据的任务。如果想个性化自己的数据集或者数据传递方式，也可以自己重写子类。

Dataset是DataLoader实例化的一个参数，所以这篇文章会先从Dataset的源代码讲起，然后讲到DataLoader，关注主要函数，少细枝末节，目的是使大家学会自定义自己的数据集。

### 什么时候使用Dataset

CIFAR10是CV训练中经常使用到的一个数据集，在PyTorch中CIFAR10是一个写好的Dataset，我们使用时只需以下代码：

```python
data = datasets.CIFAR10("./data/", transform=transform, train=True, download=True)
```

datasets.CIFAR10就是一个Datasets子类，data是这个类的一个实例。

我们有的时候需要用自己在一个文件夹中的数据作为数据集，这个时候，我们可以使用ImageFolder这个方便的API。

```python
FaceDataset = datasets.ImageFolder('./data', transform=img_transform)
```

### 如何定义一个自己的数据集合

`torch.utils.data.dataset` 是一个表示数据集的抽象类。任何自定义的数据集都需要继承这个类并覆写相关方法。

所谓数据集，其实就是一个负责处理索引(index)到样本(sample)映射的一个类(class)。

Pytorch提供两种数据集：

- Map式数据集
- Iterable式数据集

#### Map式数据集

一个Map式的数据集必须要重写`__getitem__(self, index)`, `len(self)` 两个内建方法，用来表示从索引到样本的映射(Map).

这样一个数据集dataset，举个例子，当使用dataset[idx]命令时，可以在你的硬盘中读取你的数据集中第idx张图片以及其标签（如果有的话）;len(dataset)则会返回这个数据集的容量。

例子-1： 自己实验中写的一个例子：这里我们的图片文件储存在“./data/faces/”文件夹下，图片的名字并不是从1开始，而是从final_train_tag_dict.txt这个文件保存的字典中读取，label信息也是用这个文件中读取。大家可以照着上面的注释阅读这段代码。


```python
from torch.utils import data
import numpy as np
from PIL import Image


class face_dataset(data.Dataset):
	def __init__(self):
		self.file_path = './data/faces/'
		f=open("final_train_tag_dict.txt","r")
		self.label_dict=eval(f.read())
		f.close()

	def __getitem__(self,index):
		label = list(self.label_dict.values())[index-1]
		img_id = list(self.label_dict.keys())[index-1]
		img_path = self.file_path+str(img_id)+".jpg"
		img = np.array(Image.open(img_path))
		return img,label

	def __len__(self):
		return len(self.label_dict)
```

下面我们看一下官方MNIST数据集的例子

```python
class MNIST(data.Dataset):
    """`MNIST <http://yann.lecun.com/exdb/mnist/>`_ Dataset. Args: root (string): Root directory of dataset where ``processed/training.pt`` and ``processed/test.pt`` exist. train (bool, optional): If True, creates dataset from ``training.pt``, otherwise from ``test.pt``. download (bool, optional): If true, downloads the dataset from the internet and puts it in root directory. If dataset is already downloaded, it is not downloaded again. transform (callable, optional): A function/transform that takes in an PIL image and returns a transformed version. E.g, ``transforms.RandomCrop`` target_transform (callable, optional): A function/transform that takes in the target and transforms it. """
    urls = [
        'http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz',
        'http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz',
        'http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz',
        'http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz',
    ]
    raw_folder = 'raw'
    processed_folder = 'processed'
    training_file = 'training.pt'
    test_file = 'test.pt'
    classes = ['0 - zero', '1 - one', '2 - two', '3 - three', '4 - four',
               '5 - five', '6 - six', '7 - seven', '8 - eight', '9 - nine']
    class_to_idx = {_class: i for i, _class in enumerate(classes)}

    @property
    def targets(self):
        if self.train:
            return self.train_labels
        else:
            return self.test_labels

    def __init__(self, root, train=True, transform=None, target_transform=None, download=False):
        self.root = os.path.expanduser(root)
        self.transform = transform
        self.target_transform = target_transform
        self.train = train  # training set or test set
        if download:
            self.download()

        if not self._check_exists():
            raise RuntimeError('Dataset not found.' +
                               ' You can use download=True to download it')

        if self.train:
            self.train_data, self.train_labels = torch.load(
                os.path.join(self.root, self.processed_folder, self.training_file))
        else:
            self.test_data, self.test_labels = torch.load(
                os.path.join(self.root, self.processed_folder, self.test_file))

    def __getitem__(self, index):
        """ Args: index (int): Index Returns: tuple: (image, target) where target is index of the target class. """
        if self.train:
            img, target = self.train_data[index], self.train_labels[index]
        else:
            img, target = self.test_data[index], self.test_labels[index]

        # doing this so that it is consistent with all other datasets         # to return a PIL Image         img = Image.fromarray(img.numpy(), mode='L')

        if self.transform is not None:
            img = self.transform(img)

        if self.target_transform is not None:
            target = self.target_transform(target)

        return img, target

    def __len__(self):
        if self.train:
            return len(self.train_data)
        else:
            return len(self.test_data)

    def _check_exists(self):
        return os.path.exists(os.path.join(self.root, self.processed_folder, self.training_file)) and \
            os.path.exists(os.path.join(self.root, self.processed_folder, self.test_file))

    def download(self):
        """Download the MNIST data if it doesn't exist in processed_folder already."""
        from six.moves import urllib
        import gzip

        if self._check_exists():
            return

        # download files         try:
            os.makedirs(os.path.join(self.root, self.raw_folder))
            os.makedirs(os.path.join(self.root, self.processed_folder))
        except OSError as e:
            if e.errno == errno.EEXIST:
                pass
            else:
                raise

        for url in self.urls:
            print('Downloading ' + url)
            data = urllib.request.urlopen(url)
            filename = url.rpartition('/')[2]
            file_path = os.path.join(self.root, self.raw_folder, filename)
            with open(file_path, 'wb') as f:
                f.write(data.read())
            with open(file_path.replace('.gz', ''), 'wb') as out_f, \
                    gzip.GzipFile(file_path) as zip_f:
                out_f.write(zip_f.read())
            os.unlink(file_path)

        # process and save as torch files         print('Processing...')

        training_set = (
            read_image_file(os.path.join(self.root, self.raw_folder, 'train-images-idx3-ubyte')),
            read_label_file(os.path.join(self.root, self.raw_folder, 'train-labels-idx1-ubyte'))
        )
        test_set = (
            read_image_file(os.path.join(self.root, self.raw_folder, 't10k-images-idx3-ubyte')),
            read_label_file(os.path.join(self.root, self.raw_folder, 't10k-labels-idx1-ubyte'))
        )
        with open(os.path.join(self.root, self.processed_folder, self.training_file), 'wb') as f:
            torch.save(training_set, f)
        with open(os.path.join(self.root, self.processed_folder, self.test_file), 'wb') as f:
            torch.save(test_set, f)

        print('Done!')

    def __repr__(self):
        fmt_str = 'Dataset ' + self.__class__.__name__ + '\n'
        fmt_str += ' Number of datapoints: {}\n'.format(self.__len__())
        tmp = 'train' if self.train is True else 'test'
        fmt_str += ' Split: {}\n'.format(tmp)
        fmt_str += ' Root Location: {}\n'.format(self.root)
        tmp = ' Transforms (if any): '
        fmt_str += '{0}{1}\n'.format(tmp, self.transform.__repr__().replace('\n', '\n' + ' ' * len(tmp)))
        tmp = ' Target Transforms (if any): '
        fmt_str += '{0}{1}'.format(tmp, self.target_transform.__repr__().replace('\n', '\n' + ' ' * len(tmp)))
        return fmt_str
```

### Iterable数据集

一个Iterable（迭代）式数据集是抽象类data.IterableDataset的子类，并且覆写了__iter__方法成为一个迭代器。这种数据集主要用于数据大小未知，或者以流的形式的输入，本地文件不固定的情况，需要以迭代的方式来获取样本索引。

关于迭代器与生成器的知识可以参见博主的另一篇文章Python迭代器与生成器介绍及在Pytorch源码中应用[https://chenllliang.github.io/2020/02/06/PyIter/]。

## DataLoader

> Data loader. Combines a dataset and a sampler, and provides an iterable over the given dataset. –PyTorch Documents

一般来说PyTorch中深度学习训练的流程是这样的：

  1. 创建Dateset
  2. Dataset传递给DataLoader
  3. DataLoader迭代产生训练数据提供给模型

对应的一般都会有这三部分代码

```python
# 创建Dateset(可以自定义)
dataset = face_dataset # Dataset部分自定义过的face_dataset
# Dataset传递给DataLoader
dataloader = torch.utils.data.DataLoader(dataset,batch_size=64,shuffle=False,num_workers=8)
# DataLoader迭代产生训练数据提供给模型
for i in range(epoch):
    for index,(img,label) in enumerate(dataloader):
        pass
```

到这里应该就PyTorch的数据集和数据传递机制应该就比较清晰明了了。Dataset负责建立索引到样本的映射，DataLoader负责以特定的方式从数据集中迭代的产生 一个个batch的样本集合。在enumerate过程中实际上是dataloader按照其参数sampler规定的策略调用了其dataset的getitem方法。

## 参数介绍

先看一下实例化一个DataLoader所需的参数，我们只关注几个重点即可。

```python
DataLoader(dataset, batch_size=1, shuffle=False, sampler=None,
           batch_sampler=None, num_workers=0, collate_fn=None,
           pin_memory=False, drop_last=False, timeout=0,
           worker_init_fn=None)
```
参数介绍:


- `dataset` (Dataset) – 定义好的Map式或者Iterable式数据集。
- `batch_size` (python:int, optional) – 一个batch含有多少样本 (default: 1)。
- `shuffle` (bool, optional) – 每一个epoch的batch样本是相同还是随机 (default: False)。
- `sampler` (Sampler, optional) – 决定数据集中采样的方法. 如果有，则shuffle参数必须为False。
- `batch_sampler` (Sampler, optional) – 和 sampler 类似，但是一次返回的是一个batch内所有样本的index。和 batch_size, shuffle, sampler, and drop_last 三个参数互斥。
- `num_workers` (python:int, optional) – 多少个子程序同时工作来获取数据，多线程。 (default: 0)
- `collate_fn` (callable, optional) – 合并样本列表以形成小批量。
- `pin_memory` (bool, optional) – 如果为True，数据加载器在返回前将张量复制到CUDA固定内存中。
- `drop_last` (bool, optional) – 如果数据集大小不能被batch_size整除，设置为True可删除最后一个不完整的批处理。如果设为False并且数据集的大小不能被batch_size整除，则最后一个batch将更小。(default: False)
- `timeout` (numeric, optional) – 如果是正数，表明等待从worker进程中收集一个batch等待的时间，若超出设定的时间还没有收集到，那就不收集这个内容了。这个numeric应总是大于等于0。 (default: 0)
- `worker_init_fn` (callable, optional) – 每个worker初始化函数 (default: None)


dataset 没什么好说的，很重要，需要按照前面所说的两种dataset定义好，完成相关函数的重写。

batch_size 也没啥好说的，就是训练的一个批次的样本数。

shuffle 表示每一个epoch中训练样本的顺序是否相同，一般True。

### 采样器

`sampler` 重点参数，采样器，是一个迭代器。PyTorch提供了多种采样器，用户也可以自定义采样器。

所有sampler都是继承 `torch.utils.data.sampler.Sampler`这个抽象类。

关于迭代器的基础知识在博主这篇文章中可以找到Python迭代器与生成器介绍及在Pytorch源码中应用。[]

```python
class Sampler(object):
    # """Base class for all Samplers.     # Every Sampler subclass has to provide an __iter__ method, providing a way     # to iterate over indices of dataset elements, and a __len__ method that     # returns the length of the returned iterators.     # """     # 一个 迭代器 基类     def __init__(self, data_source):
        pass

    def __iter__(self):
        raise NotImplementedError

    def __len__(self):
        raise NotImplementedError
```

#### PyTorch自带的Sampler

- SequentialSampler
- RandomSampler
- SubsetRandomSampler
- WeightedRandomSampler

**SequentialSampler** 很好理解就是顺序采样器。

其原理是首先在初始化的时候拿到数据集`data_source`，之后在`__iter__`方法中首先得到一个和data_source一样长度的`range`可迭代器。每次只会返回一个索引值。

```python
class SequentialSampler(Sampler):
    # r"""Samples elements sequentially, always in the same order.     # Arguments:     # data_source (Dataset): dataset to sample from     # """    # 产生顺序 迭代器     def __init__(self, data_source):
        self.data_source = data_source

    def __iter__(self):
        return iter(range(len(self.data_source)))

    def __len__(self):
        return len(self.data_source)
```

参数作用:

- `data_source`: 同上
- `num_sampler`: 指定采样的数量，默认是所有。
- `replacement`: 若为True，则表示可以重复采样，即同一个样本可以重复采样，这样可能导致有的样本采样不到。所以此时我们可以设置num_samples来增加采样数量使得每个样本都可能被采样到。

```python
class RandomSampler(Sampler):
    # r"""Samples elements randomly. If without replacement, then sample from a shuffled dataset.     # If with replacement, then user can specify ``num_samples`` to draw.     # Arguments:     # data_source (Dataset): dataset to sample from     # num_samples (int): number of samples to draw, default=len(dataset)     # replacement (bool): samples are drawn with replacement if ``True``, default=False     # """
    def __init__(self, data_source, replacement=False, num_samples=None):
        self.data_source = data_source
        self.replacement = replacement
        self.num_samples = num_samples

        if self.num_samples is not None and replacement is False:
            raise ValueError("With replacement=False, num_samples should not be specified, "
                             "since a random permute will be performed.")

        if self.num_samples is None:
            self.num_samples = len(self.data_source)

        if not isinstance(self.num_samples, int) or self.num_samples <= 0:
            raise ValueError("num_samples should be a positive integeral "
                             "value, but got num_samples={}".format(self.num_samples))
        if not isinstance(self.replacement, bool):
            raise ValueError("replacement should be a boolean value, but got "
                             "replacement={}".format(self.replacement))

    def __iter__(self):
        n = len(self.data_source)
        if self.replacement:
            return iter(torch.randint(high=n, size=(self.num_samples,), dtype=torch.int64).tolist())
        return iter(torch.randperm(n).tolist())

    def __len__(self):
        return len(self.data_source)
```

这个采样器常见的使用场景是将训练集划分成训练集和验证集:

```python
class SubsetRandomSampler(Sampler):
    # r"""Samples elements randomly from a given list of indices, without replacement.     # Arguments:     # indices (sequence): a sequence of indices     # """
    def __init__(self, indices):
        self.indices = indices

    def __iter__(self):
        return (self.indices[i] for i in torch.randperm(len(self.indices)))

    def __len__(self):
        return len(self.indices)
```


**batch_sampler**
前面的采样器每次都只返回一个索引，但是我们在训练时是对批量的数据进行训练，而这个工作就需要`BatchSampler`来做。也就是说`BatchSampler`的作用就是将前面的Sampler采样得到的索引值进行合并，当数量等于一个batch大小后就将这一批的索引值返回。

```python
class BatchSampler(Sampler):
    # Wraps another sampler to yield a mini-batch of indices.     # Args:     # sampler (Sampler): Base sampler.     # batch_size (int): Size of mini-batch.     # drop_last (bool): If ``True``, the sampler will drop the last batch if     # its size would be less than ``batch_size``     # Example:     # >>> list(BatchSampler(SequentialSampler(range(10)), batch_size=3, drop_last=False))     # [[0, 1, 2], [3, 4, 5], [6, 7, 8], [9]]     # >>> list(BatchSampler(SequentialSampler(range(10)), batch_size=3, drop_last=True))     # [[0, 1, 2], [3, 4, 5], [6, 7, 8]]
# 批次采样     def __init__(self, sampler, batch_size, drop_last):
        if not isinstance(sampler, Sampler):
            raise ValueError("sampler should be an instance of "
                             "torch.utils.data.Sampler, but got sampler={}"
                             .format(sampler))
        if not isinstance(batch_size, _int_classes) or isinstance(batch_size, bool) or \
                batch_size <= 0:
            raise ValueError("batch_size should be a positive integeral value, "
                             "but got batch_size={}".format(batch_size))
        if not isinstance(drop_last, bool):
            raise ValueError("drop_last should be a boolean value, but got "
                             "drop_last={}".format(drop_last))
        self.sampler = sampler
        self.batch_size = batch_size
        self.drop_last = drop_last

    def __iter__(self):
        batch = []
        for idx in self.sampler:
            batch.append(idx)
            if len(batch) == self.batch_size:
                yield batch
                batch = []
        if len(batch) > 0 and not self.drop_last:
            yield batch

    def __len__(self):
        if self.drop_last:
            return len(self.sampler) // self.batch_size
        else:
            return (len(self.sampler) + self.batch_size - 1) // self.batch_size
```

### 多线程

`num_workers` 参数表示同时参与数据读取的线程数量，多线程技术可以加快数据读取，提供GPU/CPU利用率。

未来会出一篇文章讲一讲PyTorch多线程实现的原理。


### DataLoader 和 Dataset 简单举例


```python
# construct dataset
import torch
from torch.utils.data import Dataset,DataLoader

class MyDataset(Dataset):
    def __init__(self):
        self.data = torch.tensor([[1,2,3],[2,3,4],[3,4,5],[4,5,6]])
        self.label = torch.LongTensor([1,1,0,0])

    def __getitem__(self,index):
        return self.data[index],self.label[index]

    def __len__(self):
        return len(self.data)

# dataloader
mydataloader = DataLoader(dataset=mydataset,
                          batch_size = 2,
                          shuffle=True)

for i, (data, label) in enumerate(mydataloader):
    print(data, label)
```


## DEMO1 - MLP's Dataset and DataLoader

```python
dim_output = 2
class TrainValidDataset(Dataset):
    '''
    Args:
      - root_dir (string): Directory containing all folders with different
        dates, each folder containing .cruise.h5 data files.
    '''

    def __init__(self, list_of_files):
        self.list_of_files_ = list_of_files
        self.data_size_until_this_file_ = []
        self.dataset_size = 0
        for file in self.list_of_files_:
            with h5py.File(file, 'r') as h5_file:
                data_size = h5_file[list(h5_file.keys())[0]].shape[0]
            self.dataset_size += data_size
            self.data_size_until_this_file_.append(self.dataset_size)
        print ('Total size of dataset: {}'.format(self.data_size_until_this_file_))

    def __len__(self):
        return self.dataset_size

    def __getitem__(self, index):
        bin_idx = self.FindBin(index, 0, len(
            self.data_size_until_this_file_)-1)
        with h5py.File(self.list_of_files_[bin_idx], 'r') as h5_file:
            idx_offset = self.data_size_until_this_file_[bin_idx] - \
                h5_file[list(h5_file.keys())[0]].shape[0]
            data = h5_file[list(h5_file.keys())[0]][index-idx_offset]
        label = data[-dim_output:]
        label[0] = (label[0] > 0.0).astype(float)
        return data[:-dim_output], label

    # Binary search to expedite the data-loading process.
    def FindBin(self, index, start, end):
        if (start == end):
            return start

        mid = int((start+end)/2.0)
        if (self.data_size_until_this_file_[mid] <= index):
            return self.FindBin(index, mid+1, end)
        else:
            return self.FindBin(index, start, mid)

# search all the files in the directory
def getListOfFiles(dirName):
    listOfFiles = os.listdir(dirName)
    allFiles = list()

    for entry in listOfFiles:
        fullPath = os.path.join(dirName, entry)
        if os.path.isdir(fullPath):
            allFiles = allFiles + getListOfFiles(fullPath)
        else:
            allFiles.append(fullPath)

    return allFiles

if __name__ == '__main__':

    list_of_training_files = getListOfFiles('data')
    train_dataset = TrainValidDataset(list_of_training_files)

    myDataLoader = DataLoader(dataset=train_dataset, batch_size=2, drop_last=True, shuffle=True)
    for i, (data, label) in enumerate(myDataLoader):
        print(data.shape)
```


## DEMO2 - LaneGCN's Dataset and DataLoader


**dataset description:**
```python
class ArgoDataset(Dataset):
    def __init__(self, split, config, train=True):
        self.config = config
        self.train = train

        if 'preprocess' in config and config['preprocess']:
            if train:
                self.split = np.load(self.config['preprocess_train'], allow_pickle=True)
            else:
                self.split = np.load(self.config['preprocess_val'], allow_pickle=True)
        else:
            self.avl = ArgoverseForecastingLoader(split)
            self.avl.seq_list = sorted(self.avl.seq_list)
            self.am = ArgoverseMap()

        if 'raster' in config and config['raster']:
            #TODO: DELETE
            self.map_query = MapQuery(config['map_scale'])

    def __getitem__(self, idx):
        if 'preprocess' in self.config and self.config['preprocess']:
            data = self.split[idx]

            if self.train and self.config['rot_aug']:
                new_data = dict()
                for key in ['city', 'orig', 'gt_preds', 'has_preds']:
                    if key in data:
                        new_data[key] = ref_copy(data[key])

                dt = np.random.rand() * self.config['rot_size']#np.pi * 2.0
                theta = data['theta'] + dt
                new_data['theta'] = theta
                new_data['rot'] = np.asarray([
                    [np.cos(theta), -np.sin(theta)],
                    [np.sin(theta), np.cos(theta)]], np.float32)

                rot = np.asarray([
                    [np.cos(-dt), -np.sin(-dt)],
                    [np.sin(-dt), np.cos(-dt)]], np.float32)
                new_data['feats'] = data['feats'].copy()
                new_data['feats'][:, :, :2] = np.matmul(new_data['feats'][:, :, :2], rot)
                new_data['ctrs'] = np.matmul(data['ctrs'], rot)

                graph = dict()
                for key in ['num_nodes', 'turn', 'control', 'intersect', 'pre', 'suc', 'lane_idcs', 'left_pairs', 'right_pairs', 'left', 'right']:
                    graph[key] = ref_copy(data['graph'][key])
                graph['ctrs'] = np.matmul(data['graph']['ctrs'], rot)
                graph['feats'] = np.matmul(data['graph']['feats'], rot)
                new_data['graph'] = graph
                data = new_data
            else:
                new_data = dict()
                for key in ['city', 'orig', 'gt_preds', 'has_preds', 'theta', 'rot', 'feats', 'ctrs', 'graph']:
                    if key in data:
                        new_data[key] = ref_copy(data[key])
                data = new_data

            if 'raster' in self.config and self.config['raster']:
                data.pop('graph')
                x_min, x_max, y_min, y_max = self.config['pred_range']
                cx, cy = data['orig']

                region = [cx + x_min, cx + x_max, cy + y_min, cy + y_max]
                raster = self.map_query.query(region, data['theta'], data['city'])

                data['raster'] = raster
            return data

        data = self.read_argo_data(idx)
        data = self.get_obj_feats(data)
        data['idx'] = idx

        if 'raster' in self.config and self.config['raster']:
            x_min, x_max, y_min, y_max = self.config['pred_range']
            cx, cy = data['orig']

            region = [cx + x_min, cx + x_max, cy + y_min, cy + y_max]
            raster = self.map_query.query(region, data['theta'], data['city'])

            data['raster'] = raster
            return data

        data['graph'] = self.get_lane_graph(data)
        return data

    def __len__(self):
        if 'preprocess' in self.config and self.config['preprocess']:
            return len(self.split)
        else:
            return len(self.avl)

    def read_argo_data(self, idx):
        city = copy.deepcopy(self.avl[idx].city)

        """TIMESTAMP,TRACK_ID,OBJECT_TYPE,X,Y,CITY_NAME"""
        df = copy.deepcopy(self.avl[idx].seq_df)

        agt_ts = np.sort(np.unique(df['TIMESTAMP'].values))
        mapping = dict()
        for i, ts in enumerate(agt_ts):
            mapping[ts] = i

        trajs = np.concatenate((
            df.X.to_numpy().reshape(-1, 1),
            df.Y.to_numpy().reshape(-1, 1)), 1)

        steps = [mapping[x] for x in df['TIMESTAMP'].values]
        steps = np.asarray(steps, np.int64)

        objs = df.groupby(['TRACK_ID', 'OBJECT_TYPE']).groups
        keys = list(objs.keys())
        obj_type = [x[1] for x in keys]

        agt_idx = obj_type.index('AGENT')
        idcs = objs[keys[agt_idx]]

        agt_traj = trajs[idcs]
        agt_step = steps[idcs]

        del keys[agt_idx]
        ctx_trajs, ctx_steps = [], []
        for key in keys:
            idcs = objs[key]
            ctx_trajs.append(trajs[idcs])
            ctx_steps.append(steps[idcs])

        data = dict()
        data['city'] = city
        data['trajs'] = [agt_traj] + ctx_trajs
        data['steps'] = [agt_step] + ctx_steps
        return data

    def get_obj_feats(self, data):
        orig = data['trajs'][0][19].copy().astype(np.float32)

        if self.train and self.config['rot_aug']:
            theta = np.random.rand() * np.pi * 2.0
        else:
            pre = data['trajs'][0][18] - orig
            theta = np.pi - np.arctan2(pre[1], pre[0])

        rot = np.asarray([
            [np.cos(theta), -np.sin(theta)],
            [np.sin(theta), np.cos(theta)]], np.float32)

        feats, ctrs, gt_preds, has_preds = [], [], [], []
        for traj, step in zip(data['trajs'], data['steps']):
            if 19 not in step:
                continue

            gt_pred = np.zeros((30, 2), np.float32)
            has_pred = np.zeros(30, np.bool)
            future_mask = np.logical_and(step >= 20, step < 50)
            post_step = step[future_mask] - 20
            post_traj = traj[future_mask]
            gt_pred[post_step] = post_traj
            has_pred[post_step] = 1

            obs_mask = step < 20
            step = step[obs_mask]
            traj = traj[obs_mask]
            idcs = step.argsort()
            step = step[idcs]
            traj = traj[idcs]

            for i in range(len(step)):
                if step[i] == 19 - (len(step) - 1) + i:
                    break
            step = step[i:]
            traj = traj[i:]

            feat = np.zeros((20, 3), np.float32)
            feat[step, :2] = np.matmul(rot, (traj - orig.reshape(-1, 2)).T).T
            feat[step, 2] = 1.0

            x_min, x_max, y_min, y_max = self.config['pred_range']
            if feat[-1, 0] < x_min or feat[-1, 0] > x_max or feat[-1, 1] < y_min or feat[-1, 1] > y_max:
                continue

            ctrs.append(feat[-1, :2].copy())
            feat[1:, :2] -= feat[:-1, :2]
            feat[step[0], :2] = 0
            feats.append(feat)
            gt_preds.append(gt_pred)
            has_preds.append(has_pred)

        feats = np.asarray(feats, np.float32)
        ctrs = np.asarray(ctrs, np.float32)
        gt_preds = np.asarray(gt_preds, np.float32)
        has_preds = np.asarray(has_preds, np.bool)

        data['feats'] = feats
        data['ctrs'] = ctrs
        data['orig'] = orig
        data['theta'] = theta
        data['rot'] = rot
        data['gt_preds'] = gt_preds
        data['has_preds'] = has_preds
        return data


    def get_lane_graph(self, data):
        """Get a rectangle area defined by pred_range."""
        x_min, x_max, y_min, y_max = self.config['pred_range']
        radius = max(abs(x_min), abs(x_max)) + max(abs(y_min), abs(y_max))
        lane_ids = self.am.get_lane_ids_in_xy_bbox(data['orig'][0], data['orig'][1], data['city'], radius)
        lane_ids = copy.deepcopy(lane_ids)

        lanes = dict()
        for lane_id in lane_ids:
            lane = self.am.city_lane_centerlines_dict[data['city']][lane_id]
            lane = copy.deepcopy(lane)
            centerline = np.matmul(data['rot'], (lane.centerline - data['orig'].reshape(-1, 2)).T).T
            x, y = centerline[:, 0], centerline[:, 1]
            if x.max() < x_min or x.min() > x_max or y.max() < y_min or y.min() > y_max:
                continue
            else:
                """Getting polygons requires original centerline"""
                polygon = self.am.get_lane_segment_polygon(lane_id, data['city'])
                polygon = copy.deepcopy(polygon)
                lane.centerline = centerline
                lane.polygon = np.matmul(data['rot'], (polygon[:, :2] - data['orig'].reshape(-1, 2)).T).T
                lanes[lane_id] = lane

        lane_ids = list(lanes.keys())
        ctrs, feats, turn, control, intersect = [], [], [], [], []
        for lane_id in lane_ids:
            lane = lanes[lane_id]
            ctrln = lane.centerline
            num_segs = len(ctrln) - 1

            ctrs.append(np.asarray((ctrln[:-1] + ctrln[1:]) / 2.0, np.float32))
            feats.append(np.asarray(ctrln[1:] - ctrln[:-1], np.float32))

            x = np.zeros((num_segs, 2), np.float32)
            if lane.turn_direction == 'LEFT':
                x[:, 0] = 1
            elif lane.turn_direction == 'RIGHT':
                x[:, 1] = 1
            else:
                pass
            turn.append(x)

            control.append(lane.has_traffic_control * np.ones(num_segs, np.float32))
            intersect.append(lane.is_intersection * np.ones(num_segs, np.float32))

        node_idcs = []
        count = 0
        for i, ctr in enumerate(ctrs):
            node_idcs.append(range(count, count + len(ctr)))
            count += len(ctr)
        num_nodes = count

        pre, suc = dict(), dict()
        for key in ['u', 'v']:
            pre[key], suc[key] = [], []
        for i, lane_id in enumerate(lane_ids):
            lane = lanes[lane_id]
            idcs = node_idcs[i]

            pre['u'] += idcs[1:]
            pre['v'] += idcs[:-1]
            if lane.predecessors is not None:
                for nbr_id in lane.predecessors:
                    if nbr_id in lane_ids:
                        j = lane_ids.index(nbr_id)
                        pre['u'].append(idcs[0])
                        pre['v'].append(node_idcs[j][-1])

            suc['u'] += idcs[:-1]
            suc['v'] += idcs[1:]
            if lane.successors is not None:
                for nbr_id in lane.successors:
                    if nbr_id in lane_ids:
                        j = lane_ids.index(nbr_id)
                        suc['u'].append(idcs[-1])
                        suc['v'].append(node_idcs[j][0])

        lane_idcs = []
        for i, idcs in enumerate(node_idcs):
            lane_idcs.append(i * np.ones(len(idcs), np.int64))
        lane_idcs = np.concatenate(lane_idcs, 0)

        pre_pairs, suc_pairs, left_pairs, right_pairs = [], [], [], []
        for i, lane_id in enumerate(lane_ids):
            lane = lanes[lane_id]

            nbr_ids = lane.predecessors
            if nbr_ids is not None:
                for nbr_id in nbr_ids:
                    if nbr_id in lane_ids:
                        j = lane_ids.index(nbr_id)
                        pre_pairs.append([i, j])

            nbr_ids = lane.successors
            if nbr_ids is not None:
                for nbr_id in nbr_ids:
                    if nbr_id in lane_ids:
                        j = lane_ids.index(nbr_id)
                        suc_pairs.append([i, j])

            nbr_id = lane.l_neighbor_id
            if nbr_id is not None:
                if nbr_id in lane_ids:
                    j = lane_ids.index(nbr_id)
                    left_pairs.append([i, j])

            nbr_id = lane.r_neighbor_id
            if nbr_id is not None:
                if nbr_id in lane_ids:
                    j = lane_ids.index(nbr_id)
                    right_pairs.append([i, j])
        pre_pairs = np.asarray(pre_pairs, np.int64)
        suc_pairs = np.asarray(suc_pairs, np.int64)
        left_pairs = np.asarray(left_pairs, np.int64)
        right_pairs = np.asarray(right_pairs, np.int64)

        graph = dict()
        graph['ctrs'] = np.concatenate(ctrs, 0)
        graph['num_nodes'] = num_nodes
        graph['feats'] = np.concatenate(feats, 0)
        graph['turn'] = np.concatenate(turn, 0)
        graph['control'] = np.concatenate(control, 0)
        graph['intersect'] = np.concatenate(intersect, 0)
        graph['pre'] = [pre]
        graph['suc'] = [suc]
        graph['lane_idcs'] = lane_idcs
        graph['pre_pairs'] = pre_pairs
        graph['suc_pairs'] = suc_pairs
        graph['left_pairs'] = left_pairs
        graph['right_pairs'] = right_pairs

        for k1 in ['pre', 'suc']:
            for k2 in ['u', 'v']:
                graph[k1][0][k2] = np.asarray(graph[k1][0][k2], np.int64)

        for key in ['pre', 'suc']:
            if 'scales' in self.config and self.config['scales']:
                #TODO: delete here
                graph[key] += dilated_nbrs2(graph[key][0], graph['num_nodes'], self.config['scales'])
            else:
                graph[key] += dilated_nbrs(graph[key][0], graph['num_nodes'], self.config['num_scales'])
        return graph
```

**DataLoader:**

```python
 # Data loader for training
    dataset = Dataset(config["train_split"], config, train=True)
    train_sampler = DistributedSampler(
        dataset, num_replicas=hvd.size(), rank=hvd.rank()
    )
    train_loader = DataLoader(
        dataset,
        batch_size=config["batch_size"],
        num_workers=config["workers"],
        sampler=train_sampler,
        collate_fn=collate_fn,
        pin_memory=True,
        worker_init_fn=worker_init_fn,
        drop_last=True,
    )

    # Data loader for evaluation
    dataset = Dataset(config["val_split"], config, train=False)
    val_sampler = DistributedSampler(dataset, num_replicas=hvd.size(), rank=hvd.rank())
    val_loader = DataLoader(
        dataset,
        batch_size=config["val_batch_size"],
        num_workers=config["val_workers"],
        sampler=val_sampler,
        collate_fn=collate_fn,
        pin_memory=True,
    )
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/datasetanddataloader/  

