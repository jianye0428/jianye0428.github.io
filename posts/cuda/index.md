# CUDA Introduction


[1] https://blog.csdn.net/Augusdi/article/details/12187291


## CUDA编程

### 1.什么是CUDA

CUDA(Compute Unified Device Architecture)，统一计算架构，是NVidia推出的并行计算平台。NVidia官方对其的解释是：一个并行计算平台和简单（简洁）地使用图像处理单元（GPU）进行通用计算的编程模型。利用GPU的能力在计算性能上有惊人的提升。

简单地说CUDA是便于程序员利用NVidia GPU进行通用计算的开发环境及工具，目前支持C/C++语言，将来还会支持Fortran语言。

### 2.为什么要用到CUDA

CPU主频要比GPU高2-3倍左右，但是通常情况下GPU核心的数量要比CPU多2-3个数量级以上。因此GPU的计算能力要远大于CPU，充分发挥GPU的计算能力，可以有成倍的性能提升。

早期利用GPU的计算能力是使用着色器和着色语言（GLSL等）。目前广泛使用的是CUDA和OpenCL。CUDA是针对NVidia GPU硬件设备设计的，而 OpenCL是针对跨平台设计的。因此CUDA可充分发挥NVidia GPU的计算性能。

CUDA可以直接使用C/C++语言来开发GPU程序，省去了程序员重新学一种新语言的麻烦。


### 3.CUDA环境搭建

CUDA环境主要分为四点：硬件（GPU设备）、操作系统、C/C++编译器和CUDA工具包。

硬件（GPU设备），必须是支持CUDA的GPU。可到NVidia官网查询支持CUDA的GPU设备，具体地址为：http://www.nvidia.com/object/cuda_home_new.html 。

操作系统，支持Microsoft Windows、Mac OS X和Linux。

C/C++编译器，对不同的操作系统有不同的要求。

CUDA工具包，NVidia提供了不同操作系统对应的CUDA Toolkit，可从https://developer.nvidia.com/cuda-downloads 下载对应的版本。

本文只以Microsoft Windows为例介绍如何搭建CUDA环境。

准备材料：

·一台装有支持CUDA GPU的电脑。

·Microsoft Windows操作系统（Microsoft Windows XP,Vista,7,or 8 or Windows Server 2003 or 2008）。

·CUDA工具包（相应操作系统）。下载地址：https://developer.nvidia.com/cuda-downloads

·C/C++编译器：Microsoft Visual Studio 2008 或 2010，或者对应版本的Microsoft Visual C++ Express产品。

安装步骤：

·在装有支持CUDA GPU的电脑上安装Microsoft Windows操作系统（一般情况下都已经完成这步骤）。

·安装C/C++编译器，可只安装其中的C++编译器部分。

·安装CUDA工具包。（CUDA工具包中有NVidia GPU的驱动程序，尚未安装的请选择安装。）

安装验证：

Windows XP系统：进入 C:\Documents and Settings\All Users\Application Data\NVIDIA Corporation\CUDA Samples\v5.0\bin\win32\Release 目录运行deviceQuery.exe文件。

Windows Vista, Windows 7, Windows 8, Windows Server 2003, and Windows Server 2008系统：进入 C:\ProgramData\NVIDIA Corporation\CUDA Samples\v5.0\bin\win32\Release 目录运行deviceQuery.exe文件。

如果安装正确，执行deviceQuery.exe文件会得到GPU设备的相应信息。如果没有安装支持CUDA的GPU也会得出GPU的信息，其中CUDA Capability Major/Minor version number信息为9999.9999。

Microsoft Windows上更详细的安装信息请查看：http://docs.nvidia.com/cuda/cuda-getting-started-guide-for-microsoft-windows/index.html 。

Mac OS X的安装：http://docs.nvidia.com/cuda/cuda-getting-started-guide-for-mac-os-x/index.html 。
Linux的安装：http://docs.nvidia.com/cuda/cuda-getting-started-guide-for-linux/index.html 。


### 4.第一个CUDA程序

在Microsoft Windows系统上，如果成功搭建了CUDA环境，则在Microsoft Visual Studio中已经集成了CUDA的开发组件。

以下以Windows 7 + Microsoft Visual Studio 2008为例，创建第一个CUDA程序。

打开Microsoft Visual Studio 2008，依次：File->New->Project->NVIDIA->CUDA->CUDA 5.0 Runtime，输入相应的项目名称确定即可。

默认会生成一个kernel.cu文件，内容如下：

```c
#include "cuda_runtime.h"

#include "device_launch_parameters.h"

#include <stdio.h>

void addWithCuda(int *c, const int *a, const int *b, size_t size);

__global__ void addKernel(int *c, const int *a, const int *b)

{

    int i = threadIdx.x;

    c[i] = a[i] + b[i];

}

int main()

{

    const int arraySize = 5;

    const int a[arraySize] = { 1, 2, 3, 4, 5 };

    const int b[arraySize] = { 10, 20, 30, 40, 50 };

    int c[arraySize] = { 0 };

    // Add vectors in parallel.

    addWithCuda(c, a, b, arraySize);

    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",

        c[0], c[1], c[2], c[3], c[4]);

    // cudaThreadExit must be called before exiting in order for profiling and

    // tracing tools such as Nsight and Visual Profiler to show complete traces.

    cudaThreadExit();

    return 0;

}

// Helper function for using CUDA to add vectors in parallel.

void addWithCuda(int *c, const int *a, const int *b, size_t size)

{

    int *dev_a = 0;

    int *dev_b = 0;

    int *dev_c = 0;

    // Choose which GPU to run on, change this on a multi-GPU system.

    cudaSetDevice(0);

    // Allocate GPU buffers for three vectors (two input, one output)    .

    cudaMalloc((void**)&dev_c, size * sizeof(int));

    cudaMalloc((void**)&dev_a, size * sizeof(int));

    cudaMalloc((void**)&dev_b, size * sizeof(int));

    // Copy input vectors from host memory to GPU buffers.

    cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);

    cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);

    // Launch a kernel on the GPU with one thread for each element.

    addKernel<<<1, size>>>(dev_c, dev_a, dev_b);

    // cudaThreadSynchronize waits for the kernel to finish, and returns

    // any errors encountered during the launch.

    cudaThreadSynchronize();

    // Copy output vector from GPU buffer to host memory.

    cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);

    cudaFree(dev_c);

    cudaFree(dev_a);

    cudaFree(dev_b);
}
```

代码1

这是一个将两个一维数组相加的例子。

其中addKernel是内核函数，它的计算过程是在GPU上实现的，用函数类型限定符__global__限制，且函数类型为void型。

cuda_runtime.h头文件包括了运行时API和其参数的定义。（如果使用驱动API则使用cuda.h头文件）。

device_launch_parameters.h头文件包含了内核函数的5个变量threadIdx、blockDim、blockIdx、gridDim和wrapSize。

对其中CUDA运行时API函数的解释：

  - cudaSetDevice()：选择设备（GPU）。（可以不使用，不使用的情况下，默认选择设备0）

  - cudaMalloc()：动态分配显存。

  - cudaMemcpy()：设备与主机之内的数据拷贝。

  - cudaThreadSynchronize()：同步所有设备上的线程，等待所有线程结束。

  - cudaFree():释放由cudaMalloc分配的显存。

  - cudaThreadExit():结束CUDA上下文环境，释放其中的资源。

这些函数的具体介绍在 http://docs.nvidia.com/cuda/cuda-runtime-api/index.html 中。

### 5. CUDA编程

#### 5.1. 基本概念

CUDA编程中需要注意一些基本概念，分别为：主机(host)、设备(device)、运行时API、驱动API、warp、bank、函数类型限定符、变量类型限定符、thread、block、grid、计算能力、SIMT、内置变量、纹理、CUDA数组等。

主机(host)：可理解为CPU与内存的组合。

设备(device)：可理解为GPU与显存的组合。

运行时API：是指CUDA运行时API是在驱动API的基础上封装而成的，简化了CUDA的开发。

驱动API：是指CUDA驱动API，相比运行时API更接近于设备，可灵活运用设备的特性开发CUDA，可实现运行时API无法实现的功能。

warp：多处理器激活、管理、调度和执行并行任务的单位。计算能力2.x的设备warp为32个线程。未来的设备可能不同，可以通过内置变量warpSize查询。

bank：为了获得较高的存储器带宽，共享存储器被划分为多个大小相等的存储器模块，称为存储体，这些存储体就叫bank，可同步访问。

函数类型限定符：是CUDA C中特有的，用来修饰是主机函数，设备调用的设备函数，还是主机调用的设备函数。有__device__、__global__、__host__。

变量类型限定符：是用来修饰设备变量的。有__device__、__constant__、__shared__。

thread：设备中的线程，与主机中的线程是同一个概念。

block：线程块，由一组线程组成。一个线程块中的所以线程会在同一个多处理器上执行，一个多处理器上可同时执行多个线程块。

grid：有所有线程块组成的网格。

计算能力：是NVidia GPU不同架构的计算能力。

SIMT：单指令多线程，与单指令多数据（SIMD）类似。一条指令多个线程一同执行，实现程序的并行化。

内置变量：有threadIdx、blockDim、blockIdx、gridDim、warpSize。其中threadIdx指此线程在线程块中的位置；blockDim指线程块维度；blockIdx指该线程块在网格中的位置；gridDim指线程块网格维度；warpSize指一个warp多少个线程。

纹理：本文主要涉及到的是纹理参考、纹理绑定、纹理获取。

CUDA数组：区别于线性存储器，对数据进行了对齐等的处理，包括一维、二维和三维。其中的数据为：一元、二元或四元组。

**CUDA编程模型基础**

在给出CUDA的编程实例之前，这里先对CUDA编程模型中的一些概念及基础知识做个简单介绍。CUDA编程模型是一个异构模型，需要CPU和GPU协同工作。在CUDA中，host和device是两个重要的概念，我们用host指代CPU及其内存，而用device指代GPU及其内存。CUDA程序中既包含host程序，又包含device程序，它们分别在CPU和GPU上运行。同时，host与device之间可以进行通信，这样它们之间可以进行数据拷贝。典型的CUDA程序的执行流程如下：

    分配host内存，并进行数据初始化；分配device内存，并从host将数据拷贝到device上；调用CUDA的核函数在device上完成指定的运算；将device上的运算结果拷贝到host上；释放device和host上分配的内存。

上面流程中最重要的一个过程是调用CUDA的核函数来执行并行计算，kernel是CUDA中一个重要的概念，kernel是在device上线程中并行执行的函数，核函数用__global__符号声明，在调用时需要用<<<grid, block>>>来指定kernel要执行的线程数量，在CUDA中，每一个线程都要执行核函数，并且每个线程会分配一个唯一的线程号thread ID，这个ID值可以通过核函数的内置变量threadIdx来获得。

由于GPU实际上是异构模型，所以需要区分host和device上的代码，在CUDA中是通过函数类型限定词开区别host和device上的函数，主要的三个函数类型限定词如下：

    __global__：在device上执行，从host中调用（一些特定的GPU也可以从device上调用），返回类型必须是void，不支持可变参数参数，不能成为类成员函数。注意用__global__定义的kernel是异步的，这意味着host不会等待kernel执行完就执行下一步。__device__：在device上执行，单仅可以从device中调用，不可以和__global__同时用。__host__：在host上执行，仅可以从host上调用，一般省略不写，不可以和__global__同时用，但可和__device__，此时函数会在device和host都编译。

要深刻理解kernel，必须要对kernel的线程层次结构有一个清晰的认识。首先GPU上很多并行化的轻量级线程。kernel在device上执行时实际上是启动很多线程，一个kernel所启动的所有线程称为一个网格（grid），同一个网格上的线程共享相同的全局内存空间，grid是线程结构的第一层次，而网格又可以分为很多线程块（block），一个线程块里面包含很多线程，这是第二个层次。线程两层组织结构如下图所示，这是一个gird和block均为2-dim的线程组织。grid和block都是定义为dim3类型的变量，dim3可以看成是包含三个无符号整数（x，y，z）成员的结构体变量，在定义时，缺省值初始化为1。因此grid和block可以灵活地定义为1-dim，2-dim以及3-dim结构，对于图中结构（主要水平方向为x轴），定义的grid和block如下所示，kernel在调用时也必须通过执行配置<<<grid, block>>>来指定kernel所使用的线程数及结构。


所以，一个线程需要两个内置的坐标变量（blockIdx，threadIdx）来唯一标识，它们都是dim3类型变量，其中blockIdx指明线程所在grid中的位置，而threaIdx指明线程所在block中的位置，如图中的Thread (1,1)满足：

```c
threadIdx.x = 1
threadIdx.y = 1
blockIdx.x = 1
blockIdx.y = 1
```
一个线程块上的线程是放在同一个流式多处理器（SM)上的，但是单个SM的资源有限，这导致线程块中的线程数是有限制的，现代GPUs的线程块可支持的线程数可达1024个。有时候，我们要知道一个线程在blcok中的全局ID，此时就必须还要知道block的组织结构，这是通过线程的内置变量blockDim来获得。它获取线程块各个维度的大小。对于一个2-dim的block ，线程 的ID值为 ，如果是3-dim的block ，线程 的ID值为

。另外线程还有内置变量gridDim，用于获得网格块各个维度的大小。

kernel的这种线程组织结构天然适合vector,matrix等运算，如我们将利用上图2-dim结构实现两个矩阵的加法，每个线程负责处理每个位置的两个元素相加，代码如下所示。线程块大小为(16, 16)，然后将N*N大小的矩阵均分为不同的线程块来执行加法运算。

此外这里简单介绍一下CUDA的内存模型，如下图所示。可以看到，每个线程有自己的私有本地内存（Local Memory），而每个线程块有包含共享内存（Shared Memory）,可以被线程块中所有线程共享，其生命周期与线程块一致。此外，所有的线程都可以访问全局内存（Global Memory）。还可以访问一些只读内存块：常量内存（Constant Memory）和纹理内存（Texture Memory）。内存结构涉及到程序优化，这里不深入探讨它们。


还有重要一点，你需要对GPU的硬件实现有一个基本的认识。上面说到了kernel的线程组织层次，那么一个kernel实际上会启动很多线程，这些线程是逻辑上并行的，但是在物理层却并不一定。这其实和CPU的多线程有类似之处，多线程如果没有多核支持，在物理层也是无法实现并行的。但是好在GPU存在很多CUDA核心，充分利用CUDA核心可以充分发挥GPU的并行计算能力。GPU硬件的一个核心组件是SM，前面已经说过，SM是英文名是 Streaming Multiprocessor，翻译过来就是流式多处理器。SM的核心组件包括CUDA核心，共享内存，寄存器等，SM可以并发地执行数百个线程，并发能力就取决于SM所拥有的资源数。当一个kernel被执行时，它的gird中的线程块被分配到SM上，一个线程块只能在一个SM上被调度。SM一般可以调度多个线程块，这要看SM本身的能力。那么有可能一个kernel的各个线程块被分配多个SM，所以grid只是逻辑层，而SM才是执行的物理层。SM采用的是SIMT (Single-Instruction, Multiple-Thread，单指令多线程)架构，基本的执行单元是线程束（warps)，线程束包含32个线程，这些线程同时执行相同的指令，但是每个线程都包含自己的指令地址计数器和寄存器状态，也有自己独立的执行路径。所以尽管线程束中的线程同时从同一程序地址执行，但是可能具有不同的行为，比如遇到了分支结构，一些线程可能进入这个分支，但是另外一些有可能不执行，它们只能死等，因为GPU规定线程束中所有线程在同一周期执行相同的指令，线程束分化会导致性能下降。当线程块被划分到某个SM上时，它将进一步划分为多个线程束，因为这才是SM的基本执行单元，但是一个SM同时并发的线程束数是有限的。这是因为资源限制，SM要为每个线程块分配共享内存，而也要为每个线程束中的线程分配独立的寄存器。所以SM的配置会影响其所支持的线程块和线程束并发数量。总之，就是网格和线程块只是逻辑划分，一个kernel的所有线程其实在物理层是不一定同时并发的。所以kernel的grid和block的配置不同，性能会出现差异，这点是要特别注意的。还有，由于SM的基本执行单元是包含32个线程的线程束，所以block大小一般要设置为32的倍数。


#### 5.2. 线程层次结构

CUDA线程的层次结构，由小到大依次为线程(thread)、线程块(block)、线程块网格(grid)。一维、二维或三维的线程组组成一个线程块，一维、二维或三维的线程块组组成一个线程块网格。

下图是由二维的线程块组组成的线程块网络，其中线程块是由二维的线程组组成。


图1 NVidia GPU的硬件结构是，一组流处理器组成一个多处理器，一个或多个多处理器组成一个GPU。其中流处理器，可以理解为处理计算的核心单元。多处理器类似于多核CPU。NVidia GPU从DX10（DirectX10）开始出现了Tesla、Fermi、Kepler架构，不同的架构多处理器中流处理器数量都有差别。

在进行CUDA编程前，可以先检查一下自己的GPU的硬件配置，这样才可以有的放矢，可以通过下面的程序获得GPU的配置属性：

```c
int dev = 0;
    cudaDeviceProp devProp;
    CHECK(cudaGetDeviceProperties(&devProp, dev));
    std::cout << "使用GPU device " << dev << ": " << devProp.name << std::endl;
    std::cout << "SM的数量：" << devProp.multiProcessorCount << std::endl;
    std::cout << "每个线程块的共享内存大小：" << devProp.sharedMemPerBlock / 1024.0 << " KB" << std::endl;
    std::cout << "每个线程块的最大线程数：" << devProp.maxThreadsPerBlock << std::endl;
    std::cout << "每个EM的最大线程数：" << devProp.maxThreadsPerMultiProcessor << std::endl;
    std::cout << "每个SM的最大线程束数：" << devProp.maxThreadsPerMultiProcessor / 32 << std::endl;

    // 输出如下
    使用GPU device 0: GeForce GT 730
    SM的数量：2
    每个线程块的共享内存大小：48 KB
    每个线程块的最大线程数：1024
    每个EM的最大线程数：2048
    每个EM的最大线程束数：64
```
ref: https://zhuanlan.zhihu.com/p/34587739
### 5.3. 存储器层次结构

CUDA存储器有：寄存器(register)、共享存储器(shared memory)、常量存储器(constant memory)、本地存储器(local memory)、全局存储器(global memory)、纹理存储器等。其中寄存器和本地存储器是线程(thread)私有的，共享存储器是对线程块(block)中的所有线程可见，常量存储器、全局存储器和纹理存储器是对网格(grid)中所有线程可见。

下图解释了存储器的层次结构：

#### 5.4. 运行时API

运用运行时API开发CUDA程序需要了解：初始化、设备管理、存储器管理、流管理、事件管理、纹理参考管理、OpenGL互操作和Direct3D互操作。

运行时API文档地址为：http://docs.nvidia.com/cuda/cuda-runtime-api/index.html 。

##### 5.4.1. 初始化

运行时API不存在显示初始化函数，初始化会在首次调用运行时函数时完成。虽然不需要调用初始化函数进行初始化，但是退出时需要调用退出函数cudaThreadExit()释放资源。

##### 5.4.2. 设备管理

有些电脑上可能有多块设备，因此对于不同的要求选择合适的设备。设备管理主要是获取设备信息和选择执行设备。

主要有三个函数：

·cudaGetDeviceCount()：得到电脑上设备的个数。

·cudaGetDeviceProperties()：获得对应设备的信息。

·cudaSetDevice()：设置CUDA上下文对应的设备。

运行__global__函数前需要提前选择设备，如果不调用cudaSetDevice()函数，则默认使用0号设备。

上面三个函数的具体用法请查看CUDA运行时API文档。

##### 5.4.3. 存储器管理

共享存储器、常量存储器、线性存储器和CUDA数组的使用是存储器管理的主要部分。

###### 5.4.3.1 共享存储器

共享存储器，使用__shared__变量限定符修饰，可静态或动态分配共享存储器。

代码一：
  - 静态分配共享存储器，是在设备代码中直接分配共享存储器的大小，如下代码：

```c
#define SHARED_MEM 16

__global__ void kernel(…)

{
       __shared__ int shared[SHARED_MEM];
}

void main()
{
       kernel<<<nBlock, nThread>>>(…);
}
```

代码2
  - 动态分配共享存储器，是在主机代码中使用内核函数的第三个特定参数传入分配共享存储器的大小，如下代码：
    ```c
    #define SHARED_MEM 16

    __global__ void kernel(…)
    {
        extern __shared__ int shared[];
    }

    void main()
    {
        int nSharedMem = (int)SHARED_MEM;
        kernel<<<nBlock, nThread, nSharedMem*sizeof(int)>>>(…);
    }
    ```

###### 5.4.3.2. 常量存储器

常量存储器，使用__constant__变量限定符修饰。使用常量存储器，是由于其在设备上有片上缓存，比全局存储器读取效率高很多。

使用常量存储器时会涉及的运行时API函数主要有：

·cudaMemcpyToSymbol()

·cudaMemcpyFromSymbol()

·cudaGetSymbolAddress()

·cudaGetSymbolSize()

主机代码中使用cudaGetSymbolAddress()获取__constant__或__device__定义的变量地址。设备代码中可通过提取__device__、__shared__或__constant__变量的指针获取变量地址。

###### 5.4.3.3. 线性存储器

线性存储器是使用cudaMalloc()、cudaMallocPitch()或cudaMalloc3D()分配的，使用cudaFree()释放。二维的时候建议使用cudaMallocPitch()分配，cudaMallocPitch()函数对对齐进行了调整。这三个分配函数对应cudaMemset()、cudaMemset2D()、cudaMemset3D()三个memset函数和cudaMemcpy()、cudaMemcpy2D()、cudaMemcpy3D()三个memcpy函数。

###### 5.4.3.4. CUDA数组

CUDA数组是使用cudaMallocArray()、cudaMalloc3DArray()分配的，使用cudaFreeArray()释放。

相关memcpy函数请查阅CUDA运行时API文档。

具体使用可查阅CUDA编程指南：http://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html 。

##### 5.4.4. 流管理

主机设备之间的内存拷贝与内核在设备上执行是异步的。在不使用流的情况下，是这样执行的：设备先从主机上拷贝内存，拷贝完成之后，再在设备上执行内核代码计算，最后当内核执行完毕，再把设备上的内存拷贝到主机上。当使用两个流的情况下，0号流执行内核代码的同时1号流拷贝主机内存到设备，1号流执行的同时0号流拷贝设备内存到主机（具体的实现并不一定如此，这里是为了说明流的作用简单做了假设）。两个流的情况下，部分内存拷贝和内置执行是同时进行的（异步的），比同步的内存拷贝和内核执行节省了时间。

 与流有关的函数有：

    ·cudaStreamCreate()：流的创建；

    ·cudaStreamDestroy()：流的销毁；

    ·cudaStreamSynchronize()：流同步；

    ·*Async：与流相关的其他函数。

    内核<<<…>>>的第四个参数为哪个流。

    CUDA编程指南中有对流具体实现的讲解。

    https://blog.csdn.net/a925907195/article/details/39500915


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cuda/  

