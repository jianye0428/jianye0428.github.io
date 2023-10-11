# Vim Installation


# VIM 8.2 安装
## 1. Install Python3.9 from source

- **Update the packages list and install the packages necessary to build Python**

    ```shell
    sudo apt update && sudo apt install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev libsqlite3-dev wget libbz2-dev
    ```
- **Download the latest release’s source code from the Python download page using wget**

    ```shell
    wegt https://www.python.org/ftp/python/3.9.0/Python-3.9.1.tgz
    ```
- **Switch to the Python source directory and execute the configure script which performs a number of checks to make sure all of the dependencies on your system are present**

    ```shell
    cd Python-3.9.1
    ./configure --enable-optimizations --with-lto --enable-shared --prefix=/usr/local/python39
    make -j8
    ```
- **When the build process is complete, install the Python binaries by typing**

    ```shell
    sudo make altinstall
    ```

    > Do not use the standard make install as it will overwrite the default system python3 binary.

- **copy the dynamic library to usr/lib/x86_64-linux-gnu/libpython3.9.so.1.0**
   ```shell
   sudo cp /usr/local/python39/lib/libpython3.9.so.1.0 /usr/lib/x86_64-linux-gnu/
   ```
   > the command can slove the error: error while loading shared libraries: libpython3.9.so.1.0: cannot open shared object file: No such file or directory

- **make the soft link to set python39 as default python3**
   ```shell
    sudo ln -sf /usr/local/python39/bin/python3.9 /usr/bin/python3
    sudo ln -s /usr/local/python39/bin/python3.9 /usr/bin/python3.9
   ```
- **using update-alternatives to switch different python version**

  - list all the python versions
    ```shell
    sudo update-alternatives --list python3
    ```
  - display python3

    ```shell
    sudo update-alternatives --display python3
    ```
  - set different number for different version

    ```
    sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.10 1
    sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 2
    ```
  - show different mode and select number to switch another mode

    ```shell
    sudo update-alternatives --config python3
    ```

## 2. 源码安装cmake
### 2.1 download the cmake source code
- download source code
    ```shell
    wget  https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1.tar.gz
    ```

### 2.2 extract the source code directory and run the command to install
- extraction and configuration
    ```shell
    cd cmake-2.23.0
    ./bootstrap     //需要的话也可以指定安装目录，例如--prefix=/usr/local/cmake
    make && sudo make install
    ```

### 2.3 create soft link and set cmake as default
- set cmake as default
    ```shell
    sudo ln -s /usr/local/bin/cmake /usr/bin/cmake
    ```

## 3. 首先从github下载源码vim 8.2

### 3.1 源码安装vim8.2
- run the following command to downlaod source code of VIM from github

    ```shell
    git clone git clone https://github.com/vim/vim.git
    cd vim
    git pull
    cd src/
    sudo make distclean # 如果您以前构建国vim

    ```

- cofigure the installation file
  ```shell
    ./configure --with-features=huge --enable-multibyte --enable-python3interp=dynamic --with-python3-config-dir=/usr/lib/python3.10/config-3.10-x86_64-linux-gnu/ --enable-cscope --enable-gui=auto --enable-gtk2-check --enable-fontset --enable-largefile --disable-netbeans --with-compiledby="18817571704@163.com" --enable-fail-if-missing --prefix=/usr/local/vim82
    sudo make
    sudo make install
    ```
- enable clipboard
    - then you can copy the content from system clipboard to vim
        ```shell
        sudo apt-get install vim-gtk3
        ```
- 卸载vim
    - 使用以下命令重置编译操作
        ```shell
        sudo make distclean
        ```

    - 使用以下命令，可以卸载命令
        ```shell
        sudo make uninstall
        ```

### 3.2 安装vim-plug以及插件

- 安装vim-plug:

    ```shell
    curl -fLo ~/.vim/autoload/plug.vim --create-dirs \
    https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
    ```
- 安装主题gruvbox

   **to fix error: Cannot find color scheme 'gruvbox'**
    ```shell
    mkdir ~/.vim/colors/
    cp ~/.vim/plugged/gruvbox/gruvbox.vim ~/.vim/colors/
    ```
- 安装YCM(YouCompleteMe)
    根据~/.vimrc按装YCM

    ```shell
    cd ~/.vim/plugged/YouCompleteMe/
    ./install.py --clang-completer
    ```
- 安装ctags
    ```shell
    sudo apt-get install exuberant-ctags
    ```
- 其他主题直接编辑:PlugInstall进行安装

### 3.2 reference
- 参考链接:
    [1] https://github.com/ycm-core/YouCompleteMe/wiki/Building-Vim-from-source
    [2] https://wizardforcel.gitbooks.io/use-vim-as-ide/content/0.html


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/installation/  

