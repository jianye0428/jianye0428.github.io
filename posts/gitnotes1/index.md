# Git 命令记录


## Git 命令汇总

### 1. **<font color=red>git rebase</font>**
   ref: https://git-scm.com/docs/git-rebase
- **用法一:```git rebase <branch-name>```** 将topic分支的base由E改为master
    ```shell
            A---B---C topic
            /
    D---E---F---G master
    ```
    运行:
    ```shell
    git rebase master
    git rebase master topic
    ```
    结果:
    ```shell
                    A'--B'--C' topic
                    /
    D---E---F---G master
    ```
    > if upstream branch already has a change like below:
    ```shell
            A---B---C topic
            /
    D---E---A'---F master
    ```
    then run the command ```git rebase master```, you will get following result:
    ```shell
                    B'---C' topic
                  /
    D---E---A'---F master
    ```
- **用法二:```git rebase --onto```**
    assume **topic** is based on **next**, and **next** is based on master
    ```shell
        o---o---o---o---o  master
            \
            o---o---o---o---o  next
                            \
                            o---o---o  topic
    ```
    run the command below:
    ```shell
    git rebase --onto master next topic
    ```
    then we get the result below:
    ```shell
    o---o---o---o---o  master
        |            \
        |             o'--o'--o'  topic
         \
          o---o---o---o---o  next
    ```
    Another example:
    A range of commits could also be removed with rebase. If we have the following situation:
    ```shel l
    E---F---G---H---I---J  topicA
    ```
    then the command
    ```shell
    git rebase --onto topicA~5 topicA~3 topicA
    ```
    would result in the removal of commits F and G:
    ```shell
    E---H'---I'---J'  topicA
    ```
- **用法三:```git rebase -i <commit_id> <commit_id>```** $\mathbb{\rightarrow}$ 将多个commit合并为一个。
    ```shell
    # 执行git log，得到以下commit_ids
    >>>21fd585
    >>>45j3483
    >>>9i8975d
    >>>73c20ec
    ```
    目标: 将**21fd585**、**45j3483**、**9i8975d** rebase 到 **73c20ec**
    ```shell
    git rebase -i 73c20ec  21fd585
    ```
    得到:
    ```shell
    pick
    pick
    pick
    pick
    ```
    改为
    ```shell
    pick
    squash
    squash
    squash
    ```
    最后，编辑commit内容，
    得到
    ```shell
    >>>b8bec33 # 此处为新的commit
    >>>73c20ec
    ```
    推送到remote:
    ```shell
    git push -f origin master
    ```
    ref:
    1. https://www.bilibili.com/video/BV15h411f74h/
    2. https://blog.csdn.net/weixin_45953517/article/details/114362752
    3. https://blog.csdn.net/weixin_44691608/article/details/118740059#t7
    > 遇到detached HEAD的解决办法

    ```shell
    git branch b1
    git checkout master
    git merge b1
    git push origin master
    git branch -d b1
    ```

### 2. **<font color=red>git cherrypick</font>**
- 将指定的提交用于其他分支
    例如:
    ```shell
    a - b - c - d   Master
         \
          e - f - g Feature
    ```
    run the command below and apply commit(f) to master
    ```shell
    git checkout master
    git cherry-pick f
    ```
    get the result
    ```shell
    a - b - c - d - f   Master
         \
          e - f - g Feature
    ```
- 转移多个提交
    ```shell
    # 将 A 和 B 两个提交应用到当前分支
    git cherry-pick <HashA> <HashB>
    ```
    或者
    ```shell
    # 该命令可以转移从 A 到 B 的所有提交,它们必须按照正确的顺序放置：提交 A 必须早于提交 B，否则命令将失败，但不会报错。
    git cherry-pick A..B
    ```
    ```shell
    # 使用上面的命令，提交 A 将不会包含在 Cherry pick 中， 如果要包含提交 A，可以使用下面的语法。
    git cherry-pick A^..B
    ```
    ref:https://www.ruanyifeng.com/blog/2020/04/git-cherry-pick.html

### 3. **<font color=red>git submodule</font>**
- 将一个repo添加为submodule
    ```shell
    git submodule add https://github.com/chaconinc/DbConnector
    ```
- 克隆含有子模块的项目
    ```shell
    git clone https://github.com/chaconinc/MainProject #此时包含子模块目录，但是其中没有任何文件
    cd MainProject
    cd DbConnector/
    # 此时有DbConnector目录，但是文件夹是空的
    git submodule init # 用来初始化本地配置文件
    git submodule update # 从该项目中抓取并检出父项目中列出的合适的提交
    ```
    或者
    ```shell
    git clone --recurse-submodules https://github.com/chaconinc/MainProject
    ```
    或者已经克隆了项目，但是忘记```--recurse-submodule```, 则使用
    ```shell
    git submodule update --init --recursive
    ```

### 4. **<font color=red>拉取远程分支到本地</font>**
- 拉取某一个远程的分支，并在创建相应的本地分支名称

    ```shell
    git fetch origin remote-branch-name
    git checkout -b local-branch-name origin/remote-branch-name
    ```

### 5. **<font color=red>git tag</font>**

- 用git tag打标签
    ```shell
    git tag -a v1.0
    git tag -a v0 85fc7e7 #追加标签
    ```
-  git clone 按照tag拉取代码
    ```shell
    # git clone --branch [tags标签] [git地址]
    git clone -b v5.2.0 --depth=1 http://gitlab地址
    ```
### 6. **<font color=red>git stash</font>**
 - ```git stash```:隐藏修改
     ```shell
     git stash # 隐藏修改
     git stash save "stash-name" #给每一个stash取名字
     git stash pop # 恢复隐藏的修改
     git stash list # 列出所有的隐藏
     git stash apply [number] # 指定恢复使用哪一个隐藏修改
     git stash drop # 移除某一项修改
     git stash clear # 删除所有隐藏的修改
     git stash show # 查看隐藏的修改
     git stash show -p
     git stash show --patch # 查看特定的stash的diff
     ```
### 7. **<font color=red>代码回退: git reset/git revert</font>**
 - ref:https://blog.csdn.net/weixin_35082950/article/details/113629326
 - 本地分支版本回退的方法
   ```shell
   git reflog # 找回要回退的版本的commit_id
   git reset --hard <commit_id>
   ```
 - 自己的远程分支版本回退的方法
   ```shell
   # 如果你的错误提交已经推送到自己的远程分支了，那么就需要回滚远程分支了。
   # 1. 首先要回退本地分支：
   git reflog
   git reset --hard <commit_id>
   # 2. 强制推送到远程分支
   git push -f
   ```
 - 公共远程分支版本回退的问题
   > 一个显而易见的问题：如果你回退公共远程分支，把别人的提交给丢掉了怎么办？

   假设你的远程master分支情况是这样的:
   ```shell
   A1–A2–B1 #
   ```
   其中A、B分别代表两个人，A1、A2、B1代表各自的提交。并且所有人的本地分支都已经更新到最新版本，和远程分支一致。

   这个时候你发现A2这次提交有错误，你用reset回滚远程分支master到A1，那么理想状态是你的队友一拉代码git pull，他们的master分支也回滚了，然而现实却是，你的队友会看到下面的提示：

   ```shell
   $ git status
      On branch master
      Your branch is ahead of 'origin/master' by 2 commits.
      (use "git push" to publish your local commits)
      nothing to commit, working directory clean
   ```
   > 也就是说，你的队友的分支并没有主动回退，而是比远程分支超前了两次提交，因为远程分支回退了嘛。

   ```shell
   git revert HEAD #撤销最近一次提交
   git revert HEAD~1 #撤销上上次的提交，注意：数字从0开始
   git revert 0ffaacc  #撤销0ffaacc这次提交
   ```

   > git revert 命令意思是撤销某次提交。它会产生一个新的提交，虽然代码回退了，但是版本依然是向前的，所以，当你用revert回退之后，所有人pull之后，他们的代码也自动的回退了。
   但是，要注意以下几点：
    >>    1、revert 是撤销一次提交，所以后面的commit id是你需要回滚到的版本的前一次提交。
          2、使用revert HEAD是撤销最近的一次提交，如果你最近一次提交是用revert命令产生的，那么你再执行一次，就相当于撤销了上次的撤销操作，换句话说，你连续执行两次revert HEAD命令，就跟没执行是一样的。
          3、使用revert HEAD~1 表示撤销最近2次提交，这个数字是从0开始的，如果你之前撤销过产生了commi id，那么也会计算在内的。
          4、如果使用 revert 撤销的不是最近一次提交，那么一定会有代码冲突，需要你合并代码，合并代码只需要把当前的代码全部去掉，保留之前版本的代码就可以了。
  > git revert 命令的好处就是不会丢掉别人的提交，即使你撤销后覆盖了别人的提交，他更新代码后，可以在本地用 reset 向前回滚，找到自己的代码，然后拉一下分支，再回来合并上去就可以找回被你覆盖的提交了。


**revert 合并代码，解决冲突**
使用revert命令，如果不是撤销的最近一次提交，那么一定会有冲突，如下所示：
```shell
全部清空
第一次提交
```

解决冲突很简单，因为我们只想回到某次提交，因此需要把当前最新的代码去掉即可，也就是HEAD标记的代码:

```shell
<<<<<<< HEAD
全部清空
第一次提交
=======
```
把上面部分代码去掉就可以了，然后再提交一次代码就可以解决冲突了。

### 8. **<font color=red>git branch</font>**

- 将本地分支与远程分支关联:
  ```shell
  git branch --set-upstream=origin/remote_branch your_branch
  ```

### 9. **<font color=red>git commit</font>**

- `git commit --amend`: 提交小修改但是不增加`commit_id`:
  ```shell
  git add .
  git commmit --amend # 此除可以修改commit message
  git push origin master
  ```

### 10. **<font color=red>git pull</font>**
- 示例:
    ```shell
    git pull <远程主机名> <远程分支名>:<本地分支名>
    ```
- **Examples**：
  - 取回origin主机的next分支，与本地的master分支合并
    ```shell
    git pull origin next:master
    ```
  - 远程分支(next)要与当前分支合并，则冒号后面的部分可以省略。
    ```shell
    git pull origin next
    ```
  - 如果当前分支与远程分支存在追踪关系，git pull就可以省略远程分支名
    ```shell
    git pull origin
    ```
  - 如果当前分支只有一个追踪分支，连远程主机名都可以省略
    ```shell
    ```


### 11. **<font color=red>git clone</font>**



https://mp.weixin.qq.com/s?__biz=MzA4NjkwOTQ2OA==&mid=2459742421&idx=1&sn=48b3849c03a8e99aa4a557e9643a68d1&chksm=89af6fd53c65887bd2c4a8ada2d5afabbffabf2b390477bc298e7ed11e7a563f3ba83fff9e13&scene=132&exptype=timeline_recommend_article_extendread_extendread_for_notrec#wechat_redirect
[git flow](https://blog.csdn.net/sunyctf/article/details/130587970)
    git pull

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/gitnotes1/  

