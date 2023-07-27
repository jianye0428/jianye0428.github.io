# Zsh Installation



## zsh说明

- zsh是一个Linux下强大的shell, 由于大多数Linux产品安装以及默认使用bash shell, 但是丝毫不影响极客们对zsh的热衷, 几乎每一款Linux产品都包含有zsh，通常可以用apt-get、urpmi或yum等包管理器进行安装.

- zsh是bash的增强版，其实zsh和bash是两个不同的概念，zsh更加强大。

- 通常zsh配置起来非常麻烦，且相当的复杂，所以oh-my-zsh是为了简化zsh的配置而开发的，因此oh-my-zsh算是zsh的配置.

## 准备
 - 查看当前系统用shell版本
    ```shell
    echo $SHELL
    ```

- 查看系统自带哪些shell
    ```shell
    cat /etc/shells
    ```

## 安装zsh
- 通过命令行安装zsh
    ```shell
    sudo apt install zsh
    ```

## zsh配置

- 将zsh设置为默认的shell

    ```shell
    chsh -s /bin/zsh
    ```

- 然后重启电脑
    ```shell
    reboot
    ```

## 安装oh-my-zsh及其个性化配置

### 安装oh-my-zsh
- 执行以下命令安装oh-my-zsh
    ```shell
    sh -c "$(wget https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh -O -)"
    ```
    或者
    ```shell
    sh -c "$(curl -fsSL https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh)"
    ```

### 主题配置

- 打开配置文件~/.zshrc
    输入:

    ```txt
    ZSH_THEME="xxf"
    ```

  xxf.zsh-theme文件下载地址: [xxf.zsh-theme文件下载](https://github.com/xfanwu/oh-my-zsh-custom-xxf/blob/master/themes/xxf.zsh-theme)

  下载之后将文件拷贝到以下路径: ```/home/username/.oh-my-zsh/themes/```

### 插件

#### 安装自动补全插件incr
- 首先，下载incr插件到本地
    ```shell
    cd ~/.oh-my-zsh/plugins/
    mkdir incr && cd incr
    wget http://mimosa-pudica.net/src/incr-0.2.zsh
    ```
- 编辑~/.zshrc文件，添加以下内容:
    ```
    source ~/.oh-my-zsh/plugins/incr/incr*.zsh
    ```
- 然后，source一下:
    ```shell
    source ~/.zshrc
    ```

#### 直接使用默认插件

- 在~/.zshrc文件中添加插件:

    ```txt
    plugins=(git extract z)
    ```

#### 安装autojump插件

- 通过命令行安装autojump
    ```shell
    sudo apt install autojump
    ```
- 在~/.zshrc文件中编辑:
    ```
    . /usr/share/autojump/autojump.sh
    ```
- 然后，source一下:
    ```shell
    source ~/.zshrc
    ```

#### 安装zsh-syntax-highlighting语法高亮插件

- 从gihub下载源码并放在~/.oh-my-zsh/plugins/文件夹下:
    ```shell
    cd ~/.oh-my-zsh/plugins/
    git clone https://github.com/zsh-users/zsh-syntax-highlighting.git
    ```

- 在~/.zshrc文件中编辑:
    ```
    source ~/.oh-my-zsh/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh
    ```
- 然后，source一下:
    ```shell
    source ~/.zshrc
    ```

#### 安装zsh-autosuggestions语法历史记录插件

- 从gihub下载源码并放在~/.oh-my-zsh/plugins/文件夹下:
    ```shell
    cd ~/.oh-my-zsh/plugins/
    git clone git@github.com:zsh-users/zsh-autosuggestions.git
    ```

- 在~/.zshrc文件中编辑:
    ```
    source ~/.oh-my-zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh
    ```
- 然后，source一下:
    ```shell
    source ~/.zshrc
    ```

### 其他

- 设置更新日期
    在~/.zshrc文件中编：
    ```
    exprot UPDATE_ZSH_DAYS=13
    ```
- 禁止自动更新
    ```
    DISABLE_AUTO_UPDATE="true"
    ```
- 手动更新oh-my-zsh
    ```shell
    upgrade_oh_my_zsh
    ```
- 卸载oh-my-zsh
    ```shell
    uninstall_on_my_zsh zsh
    ```

### 从bash到zsh的切换

- 直接执行zsh和oh-my-zsh的安装以及配置，并且在~/.zshrc文件中添加:
    ```
    source ~/.bashrc
    ```


## zsh 快捷键
- 快捷键
    `⌃ + u:` 清空当前行
    `⌃ + a:` 移动到行首
    `⌃ + e:` 移动到行尾
    `⌃ + f:` 向前移动
    `⌃ + b:` 向后移动
    `⌃ + p:` 上一条命令
    `⌃ + n:` 下一条命令
    `⌃ + r:` 搜索历史命令
    `⌃ + y:` 召回最近用命令删除的文字
    `⌃ + h:` 删除光标之前的字符
    `⌃ + d:` 删除光标所指的字符
    `⌃ + w:` 删除光标之前的单词
    `⌃ + k:` 删除从光标到行尾的内容
    `⌃ + t:` 交换光标和之前的字符





---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/installation/  

