# Argoverse 2 数据集


## 一、简介

Argoverse数据集是由Argo AI、卡内基梅隆大学、佐治亚理工学院发布的用于支持自动驾驶汽车3D Tracking和Motion Forecasting研究的数据集。
数据集包括:
  - **带标注的传感器数据集:** 含1000个多模态数据序列，包括来自七个环视摄像机和两个双目摄像机的高分辨率图像，以及激光雷达点云和6自由度地图配准位姿。序列包含26个目标类别的三维长方体标注，所有这些标注都是充分采样的，以支持训练和三维感知模型的评估。
  - **激光雷达数据集:** 包含20,000个未标记的激光雷达点云序列和地图配准位姿。该数据集是有史以来最大的激光雷达传感器数据集合，支持自监督学习和新兴的点云预测任务。
  - **运动预测数据集:** 包含250,000个场景，挖掘每个场景中自车与其他参与者之间有趣和具有挑战性的交互。模型的任务是预测每个场景中scored actors的未来运动，并提供跟踪历史，捕捉目标的位置、航向、速度和类别。

在所有三个数据集中，每个场景都包含自己的高精地图，带有3D车道和人行横道几何形状--来自六个不同城市的数据。所有数据集都是在CC BY-NC-SA 4.0许可下发布的。

## 二、Argoverse 2 Datasets

### 2.1 Sensor Dataset 传感器数据集

Argoverse 2传感器数据集是Argoverse 1 3D跟踪数据集的后续。AV2更大，有1000个场景，高于Argoverse 1中的113个，但每个AV2场景也更丰富--AV2中有23倍的非车辆、非行人长方体。作者手工选择Argoverse 2传感器数据集中的30s组成场景，以包含拥挤的场景，其中包含未被表示的对象、值得注意的天气和有趣的行为，如插队和乱穿马路。每个场景的持续时间为15秒。表1将AV2传感器数据集与自动驾驶数据集进行了比较。图1、2和3显示了AV2的场景在标注范围、目标多样性、目标密度和场景动态性方面如何优于其他数据集。

与本文最相似的传感器数据集是非常有影响力的nuScenes[4]--这两个数据集都有1000个场景和高清地图，尽管Argoverse在拥有地面高度地图方面是独一无二的。nuScenes包含毫米波雷达数据，而AV2包含双目图像。nuScenes有一个很大的分类学--23个目标类别，其中10个有适合训练和评估（evaluation）的数据。本文的数据集包含30个目标类别，其中26个被很好地采样，足以用于训练和评估。nuScenes横跨两个城市，而本文的提出的数据集横跨六个城市。

**传感器套件。** 激光雷达扫描收集在10赫兹，以及20 fps图像从7个摄像头定位，以提供一个完整的全景视野。此外，还提供了全局坐标系下的摄像机内参、外参和6自由度 ego-vehicle 姿态。激光雷达回波由两个32波束激光雷达捕获，激光雷达在同一方向以10赫兹旋转，但在方向上相隔180°。摄像机触发与两个激光雷达同步，导致20赫兹的帧率。七个全局快门摄像机与激光雷达同步，使它们的曝光集中在激光雷达上，扫描它们的视野。在附录中，本文提供了一个示意图，说明了汽车传感器套件及其坐标框架。

**激光雷达同步精度。** 在AV2中，本文改进了摄像机和激光雷达的同步比Argoverse 1明显。本文的同步精度在[-1.39,1.39]ms，与Waymo开放数据集[-6,7]ms[45]相比较。

**标注。** AV2传感器数据集包含本文30个类分类法中的对象的10 Hz 3D长方体标注（图1）。长方体的轨道标识符随着时间的推移对于相同的目标实例是一致的。如果对象在“感兴趣区域”(ROI)内--在映射的“可驾驶”区域的五米内，则对其进行标注。

**隐私。** 为了保护隐私，所有的脸和车牌，无论是在车辆内还是在可驾驶区域外，都被广泛模糊。

传感器数据集分割。 本文随机地将数据集划分为700、150和150个场景的训练、验证和测试拆分。

### 2.2 Lidar Dataset 激光雷达数据集
Argoverse 2 激光雷达数据集旨在支持激光雷达域中的自监督学习研究以及点云预测[48,49]。由于激光雷达数据比完整的传感器套件更紧凑，本文可以包括两倍长度的场景（30秒而不是15秒），和更多 （20,000 而不是 1,000），相当于大约40倍的驾驶小时，空间预算是5倍。AV2激光雷达数据集的挖掘标准与预测数据集（第3.3.2节）相同，以确保每个场景都是有趣的。虽然激光雷达数据集没有3D目标标注，但每个场景都带有一张高清地图，其中包含关于场景的丰富的3D信息。

本文的数据集是迄今为止最大的此类集合，有20,000个30秒序列。唯一一个类似的数据集，是同时发布的ONCE[36]，包含1M激光雷达帧，而本文的是6M激光雷达帧。本文的数据集以10 Hz采样，而不是像ONCE[36]中那样以2 Hz采样，使本文的数据集更适合于点云预测或自监督任务，这些任务点云随时间的演变是重要的。

**激光雷达数据集分割。** 本文用分别为16,000个、2000个和2000个场景的train、validation和test拆分 随机划分数据集。

### 2.3 Motion Forecasting Dataset 运动预测数据集

运动预测解决了预测局部环境中动态行为者的未来状态（或占用图）的问题。自动驾驶相关行为者的一些例子包括：车辆（停车和移动）、行人、骑自行车的人、滑板车和宠物。由预测系统生成的预测未来被用作运动规划的主要输入，运动规划根据这种预测条件进行轨迹选择。生成这些预测提出了一个复杂的、多模态的问题，涉及许多不同的、部分观察的和社会交互的主体。然而，通过利用观察到的ground truth futures 来“自我标记”数据的能力，运动预测成为机器学习应用的理想领域（ideal domain）。

在Argoverse 1成功的基础上，Argoverse 2运动预测数据集提供了从自动驾驶车队收集的一组更新的预测场景。下面列举的设计决策总结了本文从内部研究/开发中吸取的集体经验教训，以及来自3个竞赛中近260个独特团队提交的2700多份submissions的反馈意见[43]:
  1. 运动预测是长尾域中的一个安全关键系统。 因此，本文的数据集偏向于包含不同类型focal agent的不同和有趣的场景（见第3.3.2节）。本文的目标是鼓励开发确保尾部事件（tail events）期间安全的方法，而不是优化“轻松里程”上的预期性能。

  2. There is a “Goldilocks zone” of task difficulty. Argoverse1测试集的性能已经开始稳定下来，如附录的图10所示。Argoverse 2的设计是为了增加预测的难度，在未来几年刺激富有成效的重点研究。这些变化旨在激励在扩展预测范围(3s→6s)上表现良好的方法，处理多种类型的动态对象(1→5)，并确保长尾场景的安全性。未来的Argoverse releases可能会通过减少观测窗口和增加预测层位来继续增加问题的难度。

  3. 可用性很重要。 Argoverse 1受益于一个庞大而活跃的研究社区--在很大程度上是由于设置和使用的简单性。因此，本文注意确保现有的Argoverse模型可以很容易地移植到Argoverse 2上运行。特别是，本文优先考虑对地图元素的直观访问，鼓励使用车道图作为强优先级的方法。为了提高训练和泛化，所有姿态也被插值和重新采样在精确的10赫兹（Argoverse 1是近似的）。新的数据集包括更少，但更长和更复杂的场景；这确保总的数据集大小保持足够大，可以训练复杂的模型，但足够小，可以方便地访问。

### 2.4 HD Maps 高精地图

上述三个数据集中的每个场景共享相同的HD地图表示。每个场景都带有自己的本地地图区域，类似于Waymo Open Motion[12]数据集。这与最初的Argoverse数据集不同，在最初的数据集中，所有场景都被本地化到两张城市地图上--一张是匹兹堡的，一张是迈阿密的。在附录中，本文提供了例子。每个场景映射的优点包括更高效的查询和处理映射更改的能力。在本文的数据集中，一个特定的十字路口可能会被观察多次，在此期间车道、人行横道甚至地面高度都可能发生变化。

**车道图。** HD地图的核心特征是车道图，由图组成，其中是单个车道段。在附录中，本文列举并定义了本文为每个车道段提供的属性。与Argoverse 1不同，本文提供了实际的3D车道边界，而不仅仅是中心线。但是，本文的API提供了代码，可以在任何期望的采样分辨率下快速推断中心线。折线被量化到1cm分辨率。本文的表示比nuScenes更丰富，它只在2D中提供车道几何，而不是3D。

**可驾驶区域。** 而不是像在Argoverse 1中所做的那样，以光栅化格式提供可驾驶区域分割，本文以矢量格式释放它，即作为3D多边形。这提供了多种优势，主要是在压缩方面，允许本文为成千上万的场景存储单独的地图，然而光栅格式仍然很容易衍生。将多边形顶点量化到1cm分辨率。

**地表高度。**只有传感器数据集包括密集的地表高度图（尽管其他数据集仍然有关于折线的稀疏的三维高度信息）。地地面高度为可行驶区域边界5m等值线内的区域提供，本文将其定义为感兴趣区域(ROI)[6]。本文这样做是因为对于建筑物内部和建筑密集的城市街区内部，地面车辆由于遮挡而无法观察的区域，地表高度的概念定义不清(ill-defined)。光栅栅格被量化到30cm分辨率，比Argoverse 1中的1m分辨率更高。

**本地地图的面积。**每个场景的局部地图都包括在ego-vehicle轨迹的l2范数中100米膨胀范围内找到的所有实体。

## 三、Argoverse 2 API 简介

轨迹预测常用的有场景数据`ArgoverseScenario`和地图`ArgoverseStaticMap`

轨迹序列读取的API为`scenario_serialization`

可视化的API为`visualize_scenario`

argoverse2和argoverse1不一样的地方是，每一段轨迹序列（Scenario）内有自己的json地图文件（虽然说都是同一幅HD map，但是对应HD map中的不同的位置），而argoverse1是所有轨迹序列共享一个地图文件

```python
# 存放轨迹序列的类
from av2.datasets.motion_forecasting.data_schema import ArgoverseScenario
# 用于读取轨迹序列的API
from av2.datasets.motion_forecasting import scenario_serialization
# 用于可视化的API
from av2.datasets.motion_forecasting.viz.scenario_visualization import visualize_scenario
# 用于读取地图的API
from av2.map.map_api import ArgoverseStaticMap
```

**ArgoverseScenario**
每个scenario有11s长的序列，包含actor的历史轨迹集合，就是这里面的tracks，对于每一个scenario，提供了以下的顶层属性:
  - `scenario_id`: 该scenario的特有ID
  - `timestamps_ns`: 该scenario的所有时间戳
  - `tracks`: 该scenario的所有轨迹序列
  - `focal_track_id`: 该scenario的焦点agent(focal agent)的track ID
  - `city_name`: 该scenario对应的城市名

每个`track`包含以下属性:
  - `track_id`: 该track的特有ID
  - `object_states`: 该轨迹序列对应的object在这11s内的有效观测的状态，以timestep表示时间步，一般来说最多有110步，因为采样频率为10Hz，一步对应0.1s
  - `object_type`: 该轨迹序列对应的object的类型，如vehicle等
  - `category`: 给轨迹序列分配种类，用于给轨迹预测的数据质量提供参考，一般来说，有四种：SCORED_TRACK，UNSCORED_TRACK，FOCAL_TRACK，TRACK_FRAGMENT。其中FOCAL_TRACK和SCORED_TRACK数据质量较好，UNSCORED_TRACK用于当作上下文输入，数据质量一般，而TRACK_FRAGMENT的时间长度不定，数据质量较差
    - `TRACK_FRAGMENT`: Lower quality track that may only contain a few timestamps of observations. 在数据中以整数0表示
    - `UNSCORED_TRACK`: Unscored track used for contextual input. 在数据中以整数1表示
    - `SCORED_TRACK`: High-quality tracks relevant to the AV - scored in the multi-agent prediction challenge. 在数据中以整数2表示
    - `FOCAL_TRACK`: The primary track of interest in a given scenario - scored in the single-agent prediction challenge. 在数据中以整数3表示

每个`object_states`包含以下属性，对应某一actor在某一时间点的所有信息：
  - `observed`: Boolean 指示这个object state是否在该scenario的观测区间内(observed segment)
  - `timestep`: 时间步，范围是[0, num_scenario_timesteps) Time step corresponding to this object state [0, num_scenario_timesteps).
  - `position`: (x, y) Coordinates of center of object bounding box. object bounding box的xy坐标
  - `heading`: Heading associated with object bounding box (in radians, defined w.r.t the map coordinate frame). object bounding box的航向角，单位是弧度，是在地图坐标系下的
  - `velocity`: (x, y) Instantaneous velocity associated with the object (in m/s). object的xy方向的速度

每个track有以下10种label:
- `Dynamic`
  - `VEHICLE`
  - `PEDESTRIAN`
  - `MOTORCYCLIST`
  - `CYCLIST`
  - `BUS`
- `Static`
  - `STATIC`
  - `BACKGROUND`
  - `CONSTRUCTION`
  - `RIDERLESS_BICYCLE`
- `UNKNOWN`

**ArgoverseStaticMap**

- Vector Map: Lane Graph and Lane Segments
  > The core feature of the HD map is the lane graph, consisting of a graph G = (V, E), where V are individual lane segments.

  Argoverse2 提供了3D的道路边界线，而不是仅仅有centerlines，也提供了快速获取特定采样分辨率的centerlines的API，在release中多边形的分辨率被设置为1cm

  地图以json文件的形式提供, 可以通过以下方式读取：
  ```python
  from av2.map.map_api import ArgoverseStaticMap
  log_map_dirpath = Path("av2") / "00a6ffc1-6ce9-3bc3-a060-6006e9893a1a" / "map"
  avm = ArgoverseStaticMap.from_map_dir(log_map_dirpath=log_map_dirpath, build_raster=False)
  ```

- LaneSegment
  LaneSegment中包含以下属性：
  - `id:` unique identifier for this lane segment (guaranteed to be unique only within this local map). 该lane segment的特有ID（仅在局部地图中保证是特有的ID）
  - `is_intersection:` boolean value representing whether or not this lane segment lies within an intersection. boolean value，用来表示该lane segment是否位于一个路口内
  - `lane_type:` designation of which vehicle types may legally utilize this lane for travel. 表示车道线类型
  - `right_lane_boundary:` 3d polyline representing the right lane boundary. 3D线条，表示右车道边界线
  - `left_lane_boundary:` 3d polyline representing the left lane boundary. 3D线条，表示左车道边界线
  - `right_mark_type:` type of painted marking found along the right lane boundary . 右车道边界线的线型
  - `left_mark_type:` type of painted marking found along the left lane boundary. 左车道边界线的线型
  - `predecessors:` unique identifiers of lane segments that are predecessors of this object. 该lane segment的前继lane segment的unique ID
  - `successors:` unique identifiers of lane segments that represent successor of this object. Note: this list will be empty if no successors exist. 该lane segment的后继lane segment的unique ID
  - `right_neighbor_id:` unique identifier of the lane segment representing this object’s right neighbor. 该lane segment的右邻lane segment的unique ID
  - `left_neighbor_id:` unique identifier of the lane segment representing this object’s left neighbor. 该lane segment的左邻lane segment的unique ID

- Vector Map: Drivable Area
  多边形向量的分辨率也是1cm, 包含以下属性：
  - `id:` unique identifier. 特有ID
  - `area_boundary:` 3d vertices of polygon, representing the drivable area’s boundary. 3D多边形，用来表示可行驶区域的边

- Vector Map: Pedestrian Crossings
  代表人行道，由两条沿同一主轴的edge构成
  - `id:` unique identifier of pedestrian crossing. 人行道的特有ID
  - `edge1:` 3d polyline representing one edge of the crosswalk, with 2 waypoints. 3D多边形，代表人行道的其中一边，由2个waypoints组成
  - `edge2:` 3d polyline representing the other edge of the crosswalk, with 2 - waypoints. 3D多边形，代表人行道的其中一边，由2个waypoints组成

- Area of Local Maps
  Each scenario’s local map includes all entities found within a 100 m dilation in l2-norm from the ego-vehicle trajectory.
  每个scenario的局部地图包含距离ego-vehicle的轨迹100m l2距离内的所有实体

常用API汇总:
```python
import pandas as pd
from av2.datasets.motion_forecasting.data_schema import ArgoverseScenario
from av2.datasets.motion_forecasting import scenario_serialization
from av2.datasets.motion_forecasting.viz.scenario_visualization import visualize_scenario
from av2.map.map_api import ArgoverseStaticMap
from av2.map.map_primitives import Polyline
from av2.utils.io import read_json_file

# 利用pandas读取轨迹序列parquet文件
df = pd.read_parquet文件(os.path.join(self.raw_dir, raw_file_name, f'scenario_{raw_file_name}.parquet'))
# 加载地图json文件
map_data = read_json_file(map_path)
# 从地图的json文件读取车道中心线
centerlines = {lane_segment['id']: Polyline.from_json_data(lane_segment['centerline'])
                           for lane_segment in map_data['lane_segments'].values()}
# 加载地图API
map_api = ArgoverseStaticMap.from_json(map_path)
# 以某个query中心搜索附近一定半径的的lane_segments
query_center = scenario_df.loc[0, ['position_x', 'position_y']].values
search_radius_m = 30
nearby_lane_segments = map_api.get_nearby_lane_segments(query_center, search_radius_m)
# 通过map_api获取lane_segments的车道中心线
nearby_lane_centerlines = get_lane_centerlines(map_api, nearby_lane_segments)
# 获取地图内的所有pedestrian crossings（目前av2 API没有提供获取附近pedestrian crossings的API）
crosswalks = map_api.get_scenario_ped_crossings()
```

## 四、Argoverse 2 数据提取

raw_dir 文件夹内包含`.parquet`和`.json`文件，其中文件组织形式为`log_map_archive_{$scenario_name}.parquet`和`scenario_{$scenario_name}.json`，分别代表**障碍物时序信息**和**地图信息**。

- 设置原始数据路径:
  ```python
  raw_dir = "/home/yejian/yejian_personal/QCNet/train/"
  raw_file_name = "ffffe3df-8d26-42c3-9e7a-59de044736a0"
  ```
- 读取障碍物信息和地图信息
  ```python
  parquet_file = os.path.join(raw_dir, raw_file_name, f'scenario_{raw_file_name}.parquet')
  print(f"parquet_file: {parquet_file}") # 障碍物信息
  map_file = os.path.join(raw_dir, raw_file_name, f'log_map_archive_{raw_file_name}.json')
  print(f"map_file: {map_file}")# 地图信息
  df = pd.read_from_parquet(parquet_file)
  map_file = read_json_file(map_file)
  ```
- 查看障碍物信息文件内容
  ```python
  df.columns.values.tolist()
  ```
  `['observed', 'track_id', 'object_type', 'object_category', 'timestep', 'position_x', 'position_y', 'heading', 'velocity_x', 'velocity_y', 'scenario_id', 'start_timestamp', 'end_timestamp', 'num_timestamps', 'focal_track_id', 'city']`


ref:
https://blog.csdn.net/Yong_Qi2015/article/details/128731798
https://blog.csdn.net/m0_56423263/article/details/134593815


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/argoverse2/  

