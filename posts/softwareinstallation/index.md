# 程序安装教程


## 一、 apt-get source update
1. apt-get source
     change the `/etc/apt/sources.list` file to [aliyun source](https://developer.aliyun.com/mirror/ubuntu)
2. add sudo user in root[link](https://blog.csdn.net/acelove40/article/details/54343629)
    ```shell
    adduser [name]
    ```
    ```shell
    apt-get install sudo
    ```
   赋予用户`sudo`权限:
    ```shell
    sudo usermod -a -G adm username
    sudo usermod -a -G sudo username
    su [name]
    ```
    在文件`/etc/sudoers`中更改用户的`sudo`权限:
    ```text
    # sudoers file.
    #
    # This file MUST be edited with the 'vi sudo' command as root.
    #
    # See the sudoers man page for the details on how to write a sudoers file.
    #
    # Host alias specification
    # User alias specification
    # Cmnd alias specification
    # Defaults specification
    # User privilege specification
    root    ALL=(ALL) ALL
    [username] ALL=(ALL) ALL
    # Uncomment to allow people in group wheel to run all commands
    # %wheel        ALL=(ALL)       ALL
    # Same thing without a password
    # %wheel        ALL=(ALL)       NOPASSWD: ALL
    # Samples
    # %users  ALL=/sbin/mount /cdrom,/sbin/umount /cdrom
    # %users  localhost=/sbin/shutdown -h now
    ```

## 二、 Anaconda or Miniconda Installation

1. download anaconda or miniconda from [tsinghua source website](https://mirrors.tuna.tsinghua.edu.cn/anaconda/miniconda/)

    download command:
    ```shell
    wget https://mirrors.tuna.tsinghua.edu.cn/anaconda/miniconda/Miniconda3-latest-Linux-x86_64.sh
    ```

    run the command to install:
    ```shell
    bash Miniconda3-latest-linux-x86_64.sh
    ```
2. change the conda channels to tsinghua source

    ```shell
    nano ~/.condarc
    ```

    paste the following channels into your `~/.condarc` file:[ref link](https://blog.csdn.net/weixin_34910922/article/details/116721774)
    ```text
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/
    #Conda Forge
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/
    #msys2（可略）
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/msys2/
    #bioconda（可略）
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/bioconda/
    #menpo（可略）
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/menpo/
    #pytorch
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/pytorch/
    # for legacy win-64（可略）
    conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/peterjc123/
    conda config --set show_channel_urls yes
    ```

## 三、 Cmake Installation

[Ref Link](https://blog.csdn.net/liushao1031177/article/details/119799007)

  1. Download cmake source file:
      ```shell
      wget https://cmake.org/files/v3.20/cmake-3.20.0-linux-x86_64.tar.gz
      ```
  2. extract the file and move the file to `/opt/cmake-3.20.0`
      ```shell
      tar zxvf cmake-3.20.0-linux-x86_64.tar.gz
      mv cmake-3.20.0-linux-x86_64 /opt/cmake-3.20.0
      ```
  3. link the cmake as system cmake
       ```shell
        ln -sf /opt/cmake-3.20.0/bin/*  /usr/bin/
       ```
  4. check if successfully installed
     ```shell
     cmake --version
     ```

## 四、 Openmpi Installation

[Ref Link](https://blog.csdn.net/songbaiyao/article/details/72858184)
Install `openmpi` with command line:
```shell
sudo apt-get install openmpi-bin openmpi-doc libopenmpi-dev
```
在conda下安装openmapi:
```shell
conda install openmpi
```

## 五、 Anaconda下安装jupyter notebook

1、 安装jupyter notebook
  ```shell
  conda intall jupyter notebook
  ```

2、 安装nbextensions
  ```shell
  pip install jupyter_contrib_nbextensions
  jupyter contrib nbextension install --user
  ```
3、 安装nbextensions_configurator
  ```shell
  pip install jupyter_nbextensions_configurator
  jupyter nbextensions_configurator enable --user
  ```
4、 在`codemirror.css`文件中更改字体




---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/softwareinstallation/  

