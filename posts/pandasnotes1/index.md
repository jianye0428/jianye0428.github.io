# Pandas Notes 1


# Pandas Notes
## Input/Output

1. **```pd.read_csv(filepath)```: 读取csv文件**
    ref: https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.read_csv.html?highlight=read_csv
2. **```pd.read_pickle()```:读取pickle数据**

    ```python
    import pandas
    pandas.read_pickle(filepath_or_buffer, compression='infer', storage_options=None)
    ```
    ref: https://pandas.pydata.org/docs/reference/api/pandas.read_pickle.html
    Parameters:
    - **```filepath_or_buffer:```** 文件名或者文件路径
    字符串、路径对象(实现 os.PathLike[str] )或 file-like 对象实现二进制 readlines() 函数。
    - **```compression:```** ```str or dict, default ‘infer’```
    用于on-disk 数据的即时解压缩。如果 ‘infer’ 和 ‘filepath_or_buffer’ 是 path-like，则从以下扩展名检测压缩：“.gz”、“.bz2”、“.zip”、“.xz”或“.zst”(否则不压缩)。如果使用‘zip’，ZIP 文件必须只包含一个要读入的数据文件。设置为None 不解压缩。也可以是键 'method' 设置为 {```'zip'``` , ```'gzip'``` , ```'bz2'``` , ```'zstd'``` } 之一的字典，其他键值对分别转发到 zipfile.ZipFile , gzip.GzipFile , bz2.BZ2File 或 zstandard.ZstdDecompressor 。例如，可以使用自定义压缩字典为 Zstandard 解压缩传递以下内容：<u>compression={'method': 'zstd', 'dict_data': my_compression_dict}</u>。
    - **```storage_options:```** ```dict, optional```
    对特定存储连接有意义的额外选项，例如主机、端口、用户名、密码等。对于 HTTP(S) URL，键值对作为标头选项转发到 urllib。对于其他 URL(例如以 “s3://” 和 “gcs://” 开头)，键值对被转发到fsspec 。有关详细信息，请参阅fsspec和urllib。


## General functions 通用函数

## Series

## DataFrame
DataFrame是一个【表格型】的数据结构，可以看做是【由Series组成的字典】（共用同一个索引）。DataFrame由按一定顺序排列的多列数据组成。设计初衷是将Series的使用场景从一维拓展到多维。
### Constructor
1. **```DataFrame[data, index, columns, dtype, copy]```: 构造一个DataFrame对象**
### Attributes and underlying data
1. **```DataFrame.index```: 行标签(行信息)->第0列的信息**
2. **```DataFrame.columns```: 列标签(列信息)-> 第0行的信息**
3. **```DataFrame.dtypes```: 返回DataFrame的数据类型**
4. **```DataFrame.info([verbose, buf, max_cols, ...])```: 返回df的信息**
5. **```DataFrame.select_dtypes([include, exclude])```: 返回DataFrame中根据columns筛选的部分数据**
6. **```DataFrame.values```: 以numpy数组的形式返回数据**
7. **```DataFrame.axes```: 返回一个list，其中是df的axes**
8. **```DataFrame.ndim```: 返回int，代表axes/array的数量**
9. **```DataFrame.shape```: 返回tuple, 代表df维度**
10. **```DataFrame.memory_usage([index, deep])```: 返回数据内存使用情况**
11. **```DataFrame.empty```: 判断df是否为空**
12. **```DataFrame.set_flags(*[, copy, ...])```: 返回带有更新标记的df**
    DataFrame.set_flags(*, copy=False, allows_duplicate_labels=None)
    - 参数：```allows_duplicate_labels```：布尔型，可选。返回的对象是否允许重复标签。
    - 返回：Series或DataFrame, 与调用者相同的类型。
    - 注意：此方法返回一个新对象，该对象是与输入数据相同的视图。改变输入或输出值将反映在另一个中。此方法旨在用于方法链中。“Flags” 与 “metadata” 不同。标志反映了 pandas 对象(Series 或 DataFrame)的属性。元数据是 index 据集的属性，应存储在 DataFrame.attrs 中。
    - demo:
        ```python
        >>> df = pd.DataFrame({"A":[1, 2]})
        >>> df.flags.allows_duplicate_labels
        True
        >>> df2 = df.set_flags(allows_duplicate_labels=False)
        >>> df2.flags.allows_duplicate_labels
        False
        ```
13. **```DataFrame.groupby()```:**

### Conversion
1. **```DataFrame.astype(dtype[,copy, errors])```:数据类型转换**
2. **```DataFrame.convert_dtypes([infer_objects, ...])```:根据现存数据推断pd.NA数据类型**
3. **```DataFrame.infer_objects()```:根据现有数据大部分数据推断类型**
4. **```DataFrame.copy([deep])```:深度拷贝**
   - demo
    ```python
    s = pd.Series([1,2], index=["a","b"])
    deep = s.copy()# 深拷贝
    shallow = s.copy(deep=False) # 浅拷贝
    ```
5. **```DataFrame.bool()```:判断数据是ture还是false，只针对单个元素对象**

### Indexing，iteration

1. **```DataFrame.head([n])```: return the first n rows**
2. **```DataFrame.at[4,'B']```: 用标签取值(行名为4，列名为B的值)**
3. **```DataFrame.iat[1,2]```: 用行列的整数取值(第1行,第二列的值)**
4. **```DataFrame.loc['cobra':'viper', 'max_speed']```: 取行名为'cobra'至'viper', 列名为'max_speed'的值**
5. **```DataFrame.iloc```: 通过行列的值取值**
   - ```df.iloc[0]:取第0行，所有列的值，返回series类型```
   - ```df.iloc[[0]]:取得第0行，所有列的值，返回df类型```
   - ```df.iloc[[0,1]]:取得第0行和第1行的所有列的值```
   - ```df.iloc[:3]:取得第0，1，2行的值```
   - ```df.iloc[[True, False, True]]: 用True/False标记要取的行```
   - ```df.iloc[lambda x:x.index % 2 == 0]: 用lambda标记要取的行```
   - ```df.iloc[0,1]:取得第0行，第1列的值```
   - ```df.iloc[[0,2],[1,3]]: 取得第0行，第2行，第1列，第3列的值```
   - ```df.iloc[1:3, 0:3]: 取得第1行，第2行，第0列，第1列，第2列的值```
   - ```df.iloc[:, [True,False,True,False]]:取所有的行，用True/False取相应的列```
   - ```df.iloc[:,lambda df:[0,2]]: 取所有的行，取第0列，第2列```
6. **```df.insert(loc, column, value, allow_duplicates=False):插入相应的列```**
   - loc:(int), 列的位置
   - column: 列的名字，一般类型为string
   - value: 列数据的值
7. **```df.drop()```:删除固定的行或者列**
8. **```df.drop_duplicates(subset, keep, inplace=False,ignore_index=False):删除重复的行或者列```**
   - ```如果default: 删除完全相同的行
   - ```subset: 根据某一列的值，删除行数据```
   - ```keep: 设置保留第一次出现的数据或者最后一次出现的数据```
9.
10.
##


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/pandasnotes1/  

