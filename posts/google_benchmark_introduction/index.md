# Google Benchmark 性能测试分析工具


## 0. 简介

作为一个程序而言，benchmark是非常关键的一个衡量指标，无论是程序算法的指标还是程序运行性能的指标，这些我们都可以去完成衡量。对于性能衡量而言google benchmark无疑是一个比较好的选择。

<br>
<center>
  <img src="images/0_1.png" width="640" height="320" align=center style="border-radius: 0.3125em; box-shadow: 0 2px 4px 0 rgba(34,36,38,.12),0 2px 10px 0 rgba(34,36,38,.08);">
  <br>
  <div style="color:orange; border-bottom: 1px solid #d9d9d9; display: inline-block; color: #999; padding: 2px;">性能测试工具对比</div>
</center>
<br>

## 1. google benchmark安装

[google benchmark 下载地址](https://github.com/google/benchmark)

**编译安装:**

登录 linux环境，执行以下命令，进行编译安装：

```shell
git clone https://github.com/google/benchmark.git
cd benchmark
git clone https://github.com/google/googletest.git
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make
sudo make install
```

## 2.  代码编写

创建一个C++源文件，并编写包含基准测试函数的代码。例如，创建一个名为benchmark_example.cpp的文件，并编写如下内容：

```c++
#include <benchmark/benchmark.h>

static void BM_MyFunction(benchmark::State& state) {
    // 在这里编写您要测试的代码
    for (auto _ : state) {
        // 执行您的代码
    }
}

BENCHMARK(BM_MyFunction);

BENCHMARK_MAIN();
```

在上述示例中，`BM_MyFunction`是您要测试的函数。

然后我们可以使用C++编译器编译您的代码，并链接Google Benchmark库。

```shell
g++ benchmark_example.cpp -o benchmark_example -lbenchmark -lpthread
```

如果是cmakelist，则可以使用

```CMake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
# benchmark依赖thread线程库
add_library(benchmark STATIC IMPORTED)
set_property(TARGET benchmark PROPERTY IMPORTED_LOCATION /usr/local/lib/libbenchmark.a)

add_executable(demo demo.cpp)
target_link_libraries(demo
    benchmark
)

install(TARGETS
    demo
    DESTINATION "bin/"
)
```

### 2.1 基础代码调用测试

我们可以看到每一个benchmark测试用例都是一个类型为`std::function`的函数，其中`benchmark::State&`负责测试的运行及额外参数的传递。

测试用例编写完成后，我们需要使用`BENCHMARK()`将我们的测试用例注册进benchmark，这样程序运行时才会执行我们的测试。

最后是用`BENCHMARK_MAIN();`替代直接编写的main函数，它会处理命令行参数并运行所有注册过的测试用例生成测试结果。

```c++

#include <benchmark/benchmark.h>
#include <vector>
#include <array>

constexpr int len = 6;
std::vector<int> vec{1, 2, 3, 4, 5, 6};
std::array<int, len> array{1, 2, 3, 4, 5, 6};

// benchmark::State &state用于维护测试上下文信息，以及控制迭代次数
static void vector_test(benchmark::State &state)
{
    for (auto _ : state)
    {
        vec[0];
        vec[1];
        vec[2];
        vec[3];
        vec[4];
        vec[5];
    }
}

static void array_test(benchmark::State &state)
{
    for (auto _ : state)
    {
        array[0];
        array[1];
        array[2];
        array[3];
        array[4];
        array[5];
    }
}

// 注册测试用例
BENCHMARK(vector_test);
BENCHMARK(array_test);
// benchmark的主函数
BENCHMARK_MAIN();
```

结果格式如下:

```shell
Load Average: 0.43, 0.25, 0.10
------------------------------------------------------
Benchmark            Time             CPU   Iterations
------------------------------------------------------
vector_test       6.81 ns         6.81 ns    102373755
array_test        13.6 ns         13.6 ns     51227934
```

### 2.2 传参调用测试

上面的测试用例都只接受一个`benchmark::State&`类型的参数，所以我们可以使用`BENCHMARK`宏生成的对象的`Arg`方法来完成参数的传递。

传递进来的参数会被放入`state`对象内部存储，通过`range`方法获取，调用时的参数`0`是传入参数的需要，对应第一个参数

```c++

#include <benchmark/benchmark.h>
#include <vector>
#include <array>

static void bench_array_ring_insert_int(benchmark::State& state)
{
    auto length = state.range(0);
    auto ring = ArrayRing<int>(length);
    for (auto _: state) {
        for (int i = 1; i <= length; ++i) {
            ring.insert(i);
        }
        state.PauseTiming();
        ring.clear();
        state.ResumeTiming();
    }
}
// 下面我们生成测试插入10次的测试用例
BENCHMARK(bench_array_ring_insert_int)->Arg(10);

static void bench_array_ring_insert_int(benchmark::State& state)
{
    auto ring = ArrayRing<int>(state.range(0));
    for (auto _: state) {
        for (int i = 1; i <= state.range(1); ++i) {
            ring.insert(i);
        }
        state.PauseTiming();
        ring.clear();
        state.ResumeTiming();
    }
}
BENCHMARK(bench_array_ring_insert_int)->Args({10, 10});

// benchmark的主函数
BENCHMARK_MAIN();
```

### 2.3 模板类的调用测试

如果针对每一种情况写测试函数，显然违反了`DRY原则`，因为除了vector的类型参数不同，其他代码几乎是完全一样的。

```c++
#include <benchmark/benchmark.h>
#include <vector>
#include <array>

template <typename T, std::size_t length, bool is_reserve = true>
void bench_vector_reserve(benchmark::State& state)
{
    for (auto _ : state) {
        std::vector<T> container;
        if constexpr (is_reserve) {
            container.reserve(length);
        }
        for (std::size_t i = 0; i < length; ++i) {
            container.push_back(T{});
        }
    }
}

BENCHMARK_TEMPLATE(bench_vector_reserve, std::string, 100);
// benchmark的主函数
BENCHMARK_MAIN();
```

## 3. 使用Benchmark接口

这里将待测试的函数注册为一个基准测试用例，并指定测试用例的名称和参数。该代码中使用了三种不同的注册方式：`函数指针`、`Lambda 函数`和`带参数的函数指针`。最后，使用 `benchmark::RunSpecifiedBenchmarks` 函数运行所有注册的基准测试用例，并使用 `benchmark::Shutdown` 函数释放资源。

```c++
#include <benchmark/benchmark.h>
#include <chrono>
#include <thread>

void BM_DemoSleep(benchmark::State& state) {
  for (auto _ : state){
    std::this_thread::sleep_for(std::chrono::nanoseconds(1000)); //待测试的代码
  }
}

void BM_DemoSleep1(benchmark::State& state, int id) {
  std::cout << "id:"<< id << std::endl;
  for (auto _ : state){
    std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
  }
}

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv); // 初始化Benchmark
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;

  // 使用函数指针注册
  benchmark::RegisterBenchmark("BM_DemoSleep", &BM_DemoSleep);
  // 使用Lamba函数注册
  benchmark::RegisterBenchmark("BM_DemoSleep1", [](benchmark::State& state){
    for (auto _ : state){
      std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    }
  });

  // 使用带参数的函数指针注册
  int id = 10;
  benchmark::RegisterBenchmark("BM_DemoSleep2", &BM_DemoSleep1, id);

  benchmark::RunSpecifiedBenchmarks(); // 运行
  benchmark::Shutdown();
}
```

## ref:

[1]. https://mp.weixin.qq.com/s/hrKwlKj6i2twd_qNqaHyYg
[2]. [Google Benchmark 用户手册](https://github.com/google/benchmark/blob/main/docs/user_guide.md)

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/google_benchmark_introduction/  

