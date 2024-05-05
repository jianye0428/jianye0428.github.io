# Line Fitting


## 曲线拟合合集
https://durant35.github.io/2017/07/21/Algorithms_LeastSquaresLineFitting/

### 一、 最小二乘法拟合直线

### 二、 三次样条曲线

- 1. 根据起始点和终点求三次样条曲线的系数

    已知三次样条曲线的方程为 $y = a_0 + a_1 \cdot x + a_2 \cdot x ^ 2 + a_3 \cdot x ^ 3$ ， 并且已知起始点坐标 $(x_0, y_0)$, 起始点导数k_1, 终点坐标(x_1, y_1), 终点导数k_2, 求三次样条曲线的系数

    解: 通过平移变换可知， 将起始点置于零点，则终点为$(x_1 - x_0, y_1 - y_0)$，那么根据点和相关点之间的导数可以求相应的系数。方程如下:

    $a_0 = y_0$
    $a_1 = k_0$
    $(y_1 - y_1) = a_1 * (x_1 - x_0) + a_2 * (x_1 - x_0) ^ 2 + a_3 * (x_1 - x_0) ^ 3$
    $k_1 = a_1 + 2 * a_2 * (x_1 - x_0) + 3 * a_3 * (x_1 - x_0) ^ 2$

    或者也可以设三次样条曲线方程为:$y = a_0 + a_1 * (x - x_0) + a_2 * (x - x_1) ^ 2 + a_3 * (x - x_2) ^ 3$

    代码参考:
    ```c++
    void PredictorManager::GetCubicPolynomialCofficients(double start_s, double start_ds, double end_s, double end_ds, double start_t, double end_t, std::array<double, 4>* coeffs) {
      coeffs->operator[](0) = start_s;
      coeffs->operator[](1) = start_ds;
      double p = end_t - start_t;
      double p2 = p * p;
      double p3 = p2 * p;
      double tmp_var1 = (end_ds - start_ds) * p;
      double tmp_var2 = end_s - start_s -  start_ds * p;
      coeffs->operator[](2) = (3.0 * tmp_var2 - tmp_var1) / p2;
      coeffs->operator[](3) = (tmp_var1 - 2.0 * tmp_var2) / p3;
   }

    double EvaluateQuarticPolynomial(const std::array<double, 5>& coeffs, const double t, const uint32_t order, const double end_t,
                                 const double end_v) {
      if (t >= end_t) {
        switch (order) {
          case 0: {
            double end_value = (((coeffs[4] * end_t + coeffs[3]) * end_t + coeffs[2]) * end_t + coeffs[1]) * end_t + coeffs[0];
            return end_value + (t - end_t) * end_v;
          }
          case 1: {
            return end_v;
          }
          default: { return 0.0; }
        }
      }
      switch (order) {
        case 0: {
          return (((coeffs[4] * t + coeffs[3]) * t + coeffs[2]) * t + coeffs[1]) * t + coeffs[0];
        }
        case 1: {
          return ((4.0 * coeffs[4] * t + 3.0 * coeffs[3]) * t + 2.0 * coeffs[2]) * t + coeffs[1];
        }
        case 2: {
          return (12.0 * coeffs[4] * t + 6.0 * coeffs[3]) * t + 2.0 * coeffs[2];
        }
        case 3: {
          return 24.0 * coeffs[4] * t + 6.0 * coeffs[3];
        }
        case 4: {
          return 24.0 * coeffs[4];
        }
        default:
          return 0.0;
      }
    }
    ```

### 三、Bezier曲线


    参考文献:
    [1]. [三次样条曲线求系数1](https://huaweicloud.csdn.net/63a571ddb878a5454594788c.html?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Eactivity-2-90477388-blog-118017126.pc_relevant_vip_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Eactivity-2-90477388-blog-118017126.pc_relevant_vip_default&utm_relevant_index=3#devmenu7)

    [2]. [三次样条曲线求系数2](https://blog.csdn.net/ymj7150697/article/details/105713587)

    [3]. [三次样条曲线求系数3](https://blog.csdn.net/weixin_37722026/article/details/103778202)

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/linefitting/  

