# Line Fitting


https://durant35.github.io/2017/07/21/Algorithms_LeastSquaresLineFitting/

## 最小二乘法拟合直线

## 三次样条曲线

- 1. 根据起始点和终点求三次样条曲线的系数

    已知三次样条曲线的方程为`y = a_0 + a_1 * x + a_2 * x ^ 2 + a_3 * x ^ 3`， 并且已知起始点坐标(x_0, y_0), 起始点导数k_1, 终点坐标(x_1, y_1), 终点导数k_2, 求三次样条曲线的系数

    解: 通过平移变换可知， 将起始点置于零点，则终点为(x_1 - x_0, y_1 - y_0)，那么根据点和相关点之间的导数可以求相应的系数。方程如下:

    ```
    a_0 = 0
    a_1 = k_0
    (y_1 - y_1) = a_1 * (x_1 - x_0) + a_2 * (x_1 - x_0) ^ 2 + a_3 * (x_1 - x_0) ^ 3
    k_1 = a_1 + 2 * a_2 * (x_1 - x_0) + 3 * a_3 * (x_1 - x_0) ^ 2
    ```
    或者也可以设三次样条曲线方程为:`y = a_0 + a_1 * (x - x_0) + a_2 * (x - x_1) ^ 2 + a_3 * (x - x_2) ^ 3`
    参考文献:
    [1] [三次样条曲线求值](https://huaweicloud.csdn.net/63a571ddb878a5454594788c.html?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Eactivity-2-90477388-blog-118017126.pc_relevant_vip_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Eactivity-2-90477388-blog-118017126.pc_relevant_vip_default&utm_relevant_index=3#devmenu7)

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/math/linefitting/  

